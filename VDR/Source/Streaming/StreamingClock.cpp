///
/// @file VDR/Source/Streaming/StreamingClock.cpp
///
/// @brief Streaming Clock implementation
///
/// @par OWNER:
/// Ulrich Sigmund
///
/// @par SCOPE:
/// INTERNAL Header File
///
/// 
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#define DPSC	DP_EMPTY
#define DPSCR	DP_EMPTY

#include "StreamingClock.h"

StreamingClock::StreamingClock(void)
	{
	maxClients = 4;
	numClients = 0;
	clientInfo = new StreamingClockClientInfo[maxClients];
	}


StreamingClock::~StreamingClock(void)
	{
	delete[] clientInfo;
	}


STFResult StreamingClock::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_CLOCK, IStreamingClock);
	VDRQI_END(VDRBase);

	STFRES_RAISE_OK;
	}


STFResult StreamingClock::RegisterClient(IStreamingClockClient * client, uint32 & id)
	{
	id = numClients;
	//lint --e{613}
	if (numClients == maxClients)
		{
		StreamingClockClientInfo	*	ninfo;
		uint32								i;

		maxClients *= 2;
		ninfo = new StreamingClockClientInfo[maxClients];
		
		for(i = 0; i < numClients; i++)
			ninfo[i] = clientInfo[i];

		delete[] clientInfo;
		clientInfo = ninfo;
		}

	clientInfo[numClients].client = client;
	numClients++;

	STFRES_RAISE_OK;
	}


STFResult StreamingClock::BeginStartupSequence(int32 speed)
	{
	this->speed = speed;

	pendingClients = numClients;

	STFRES_RAISE_OK;
	}


STFResult StreamingClock::SetStartupDelay(uint32 id, const StreamingClockClientStartupInfo & info)
	{
	uint32						i;
	STFHiPrec64BitTime		firstStreamTime, commonStartFrameTime, startFrameTime, adaptedStartFrameTime;
	STFHiPrec64BitDuration	maxFrameDuration;
	uint32 maxFrameDurationIndex;	

	//
	// Place the value first
	//
	clientInfo[id] = info;
	clientInfo[id].priority = 0;
	//lint --e{613}
	if (info.streamStartTimeValid)
		DPSC("Client %x : stream time is %d, nextRenderFrameNum %x, nextRenderFrameTime %d (ms)\n", 
		     id, 
		     info.streamStartTime.Get32BitTime(), 
		     info.nextRenderFrameNumber, info.nextRenderFrameTime.Get32BitTime());

	//
	// Then decrement the counter of pending units
	//
	if (--pendingClients == 0)
		{
		if (speed > 0)
			{
			StreamingClockClientInfo	*	client;
															
			//
			// Find the first available sample in all of the streams
			//
						
			uint32 firstValidClientIndex = 0;
			while (!clientInfo[firstValidClientIndex].streamStartTimeValid && (firstValidClientIndex < numClients))
				firstValidClientIndex++;

			if (firstValidClientIndex >= numClients)
				{
				firstValidClientIndex = numClients - 1;	
				DP("NO START TIME AT ALL!!\n");
				}			
			
			firstStreamTime = clientInfo[firstValidClientIndex].streamStartTime;
			for(i=firstValidClientIndex; i<numClients; i++)
				{
				if (clientInfo[i].streamStartTimeValid && (clientInfo[i].streamStartTime < firstStreamTime))
					{
					firstStreamTime = clientInfo[i].streamStartTime;
					}
				}

			// for all units, which do not have a valid start stream time, set the 
			// time of another stream
			for (i=0; i<numClients;i++)
				{
				if (!clientInfo[i].streamStartTimeValid)
					clientInfo[i].streamStartTime = firstStreamTime;
				}

			//
			// Calculate distance between the first available sample in every stream and
			// the first available sample in all streams (channel startup silence).
			// The result is in render time, not stream time.
			//
			for(i=0; i<numClients; i++)
				{
				// Note: we have to avoid overflow error due to very high stream start times.
				// So we handle > 100 sec as anyway outside the scope of this startup
				if ((clientInfo[i].streamStartTime - firstStreamTime).Get32BitDuration(STFTU_SECONDS) < 100)
					{
					// @TODO: Make the multiplication with 0x10000 a shift left operation.
					clientInfo[i].startupSilenceDuration = (clientInfo[i].streamStartTime - firstStreamTime) * 0x10000 / speed;
					}
				else
					{
					// we set the startup silence duration to be 10 sec to have it outside the scope of this startup
					clientInfo[i].startupSilenceDuration = STFHiPrec64BitDuration(100, STFTU_SECONDS);
					}

				if (clientInfo[i].startupSilenceDuration.Get32BitDuration() < 0)
					DP("Found error\n");
				}

			//
			// Find the earliest time, that a stream can be started.  This is the maximum
			// of the earliest start times for all sub streams, reduced by the startup
			// silence for this substream (a channel, that starts later may have a higher 
			// startup latency, without delaying the startup process, due to avoiding
			// playback of the startup silence).  
			//
			
			DPSC("Client startFrameTime nextRenderFrameTime startupSilenceDuration\n");
			commonStartFrameTime = clientInfo[0].nextRenderFrameTime - clientInfo[0].startupSilenceDuration;
			DPSC("    %2d       %8d            %8d       %8d\n", 0, (clientInfo[0].nextRenderFrameTime - clientInfo[0].startupSilenceDuration).Get32BitTime(), 
				clientInfo[0].nextRenderFrameTime.Get32BitTime(), clientInfo[0].startupSilenceDuration.Get32BitDuration());
			for(i=1; i<numClients; i++)
				{
				startFrameTime = clientInfo[i].nextRenderFrameTime - clientInfo[i].startupSilenceDuration;
				if (startFrameTime > commonStartFrameTime)
					{
					commonStartFrameTime = startFrameTime;
					}

				DPSC("    %2d       %8d            %8d       %8d\n", i, (clientInfo[i].nextRenderFrameTime - clientInfo[i].startupSilenceDuration).Get32BitTime(), 
					clientInfo[i].nextRenderFrameTime.Get32BitTime(), clientInfo[i].startupSilenceDuration.Get32BitDuration());
				}			


			//
			// Now find the channel with the highest frame duration
			//
			maxFrameDuration = clientInfo[0].renderFrameDuration;
			maxFrameDurationIndex = 0;
			DPSC("Client %d frameDuration: %d\n", 0, clientInfo[0].renderFrameDuration.Get32BitDuration());
			for(i=1; i<numClients; i++)
				{
				if (clientInfo[i].renderFrameDuration > maxFrameDuration)
					{
					maxFrameDurationIndex = i;
					maxFrameDuration = clientInfo[i].renderFrameDuration;
					}
				DPSC("Client %d frameDuration: %d\n", i, clientInfo[i].renderFrameDuration.Get32BitDuration());
				}
			DPSC("Found client %d with maxFrameDuration: %d\n", maxFrameDurationIndex, maxFrameDuration.Get32BitDuration());


			StreamingClockClientInfo	*	maxClient = clientInfo + maxFrameDurationIndex;

			//
			// Now calculate the amount of additional delay for the maximum frame duration
			// channel based on the maximum startup delay.
			//
			maxClient->delayDuration = commonStartFrameTime + maxClient->startupSilenceDuration - maxClient->nextRenderFrameTime;
			maxClient->delayFrames = (maxClient->delayDuration / maxClient->renderFrameDuration).ToUint32();

			//
			// Add an additional frame, if we are not an exact match, and adapt the stream
			// startup offset-
			//
			if (maxClient->renderFrameDuration * maxClient->delayFrames < maxClient->delayDuration)
				maxClient->delayFrames += 1;

			maxClient->delayDuration = maxClient->renderFrameDuration * maxClient->delayFrames;

			adaptedStartFrameTime = maxClient->delayDuration - maxClient->startupSilenceDuration + maxClient->nextRenderFrameTime;

			//
			// Now find the start frame for all other channels
			//
			for(i=0, client = clientInfo; i<numClients; i++, client++)
				{
				if (i != maxFrameDurationIndex)
					{
					client->delayDuration = adaptedStartFrameTime + client->startupSilenceDuration - client->nextRenderFrameTime;
					
					//
					// Calculate the amount of delay frames, and the quantized duration
					//
					client->delayFrames = (client->delayDuration / client->renderFrameDuration).ToUint32();
					client->delayDuration = (client->renderFrameDuration * client->delayFrames);

					//
					// Check if we could do better, if we start on the next frame...
					//
					startFrameTime = client->delayDuration + client->nextRenderFrameTime - client->startupSilenceDuration;

					if ((adaptedStartFrameTime - startFrameTime) * 2 > client->renderFrameDuration)
						{
						//
						// If so, we will wait one more
						//
						client->delayFrames += 1;
						client->delayDuration += client->renderFrameDuration;
						}

					}
				DPSC("Client %d start frame Time: %d delayFrames: %d\n", i, startFrameTime.Get32BitTime(), client->delayFrames);
				}

			//
			// Now trigger the startup of all channels
			//
			for(i=0, client = clientInfo; i<numClients; i++, client++)
				{
				client->client->SetStartupFrame(client->nextRenderFrameNumber + client->delayFrames, client->streamStartTime);
				}
			}
		}

	STFRES_RAISE_OK;
	}


//
// Perform synchronization.  The client with the highest priority will
// dominate the timing of the other clients. Because the speed factor will
// prevent direct playback time - system time deltas, the client must take
// care to perform speed adaption of the delay.
//
STFResult StreamingClock::SynchronizeClient(uint32 id, uint32 priority, const STFHiPrec64BitDuration & systemOffset, STFHiPrec64BitDuration & offset)
	{
	uint32	i, max;
	//lint --e{613}
	clientInfo[id].systemOffset = systemOffset;

	//
	// Avoid zero priority, which denotes a not yet defined offset...
	//
	clientInfo[id].priority = priority + 1;

	//
	// Find the client with the highest priority
	//
	max = 0;
	priority = clientInfo[0].priority;
	for(i=1; i<numClients; i++)
		{
		if (clientInfo[i].priority > priority)
			{
			priority = clientInfo[i].priority;
			max = i;
			}
		}

	offset = clientInfo[max].systemOffset;

	DPSCR("STRCLCK SYNCC %d PRI %2d In %7d Out %7d\n", id, clientInfo[id].priority, systemOffset.Get32BitDuration(), offset.Get32BitDuration());

	STFRES_RAISE_OK;
	}


STFResult StreamingClock::GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset)
	{
	uint32				i;
	STFHiPrec64BitDuration	maxOffset, offset;
	//lint --e{613}
	clientInfo[0].client->GetCurrentStreamTimeOffset(maxOffset);

	for(i=1; i<numClients; i++)
		{
		clientInfo[i].client->GetCurrentStreamTimeOffset(offset);
		if (maxOffset < offset)
			{
			if (speed >= 0)
				maxOffset = offset;
			}
		else
			{
			if (speed < 0)
				maxOffset = offset;
			}
		}

	systemOffset = maxOffset;

	STFRES_RAISE_OK;
	}

