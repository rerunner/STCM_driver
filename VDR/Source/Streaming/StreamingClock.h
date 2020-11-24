///
/// @file VDR/Source/Streaming/StreamingClock.h
///
/// @brief header file for actual streaming clock implementation
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

#ifndef STREAMINGCLOCK_H
#define STREAMINGCLOCK_H

#include "IStreamingClocks.h"
#include "VDR/Source/Base/VDRBase.h"


struct StreamingClockClientInfo : StreamingClockClientStartupInfo
	{
	STFHiPrec64BitDuration				startupSilenceDuration;
	uint32									delayFrames;
	STFHiPrec64BitDuration				delayDuration;
	STFHiPrec64BitDuration				systemOffset;
	uint32									priority;
	IStreamingClockClient			*	client;

	StreamingClockClientInfo & operator=(const StreamingClockClientStartupInfo & info)
		{
		streamStartTime       = info.streamStartTime;
		streamStartTimeValid  = info.streamStartTimeValid;
		renderFrameDuration   = info.renderFrameDuration;
		nextRenderFrameTime   = info.nextRenderFrameTime;
		nextRenderFrameNumber = info.nextRenderFrameNumber;		

		return *this;
		}
	};


class StreamingClock : public virtual IStreamingClock, public VDRBase
	{
	protected:
		STFInterlockedInt					pendingClients;
		int32									speed;

		StreamingClockClientInfo	*	clientInfo;
		uint32								numClients, maxClients;
	public:
		StreamingClock(void);
		~StreamingClock(void);

		// Override from VDRBase
  		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
		
		// Override from IStreamingClock
		virtual STFResult RegisterClient(IStreamingClockClient * client, uint32 & id);
		virtual STFResult BeginStartupSequence(int32 speed);
		virtual STFResult SetStartupDelay(uint32 id, const StreamingClockClientStartupInfo & info);
		virtual STFResult SynchronizeClient(uint32 id, uint32 priority, const STFHiPrec64BitDuration & systemOffset, STFHiPrec64BitDuration & offset);
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset);
	};

#endif
