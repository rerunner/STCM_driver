///
/// @brief Multiplexer for DVD streams using SCR and buffer fullness of a virtual decoder
///

#include "Device/Interface/Unit/Video/IMPEGVideoTypes.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Interface/Unit/Video/Encoder/IVDRAVRecorder.h"
#include "VDR/Source/Unit/Tags.h"
#include "Device/Source/Unit/Video/Encoder/MMEVideoEncoder.h"
#include "DVDSCRMultiplexer.h"

#if DEBUG_BITRATE
#define GOPS_IN_SEGMENT 2
#endif 

UNIT_CREATION_FUNCTION(CreateDVDSCRMultiplexerUnit, DVDSCRMultiplexerUnit)

STFResult DVDSCRMultiplexerUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)new VirtualDVDSCRMultiplexerUnit(this);

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult DVDSCRMultiplexerUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	}

STFResult DVDSCRMultiplexerUnit::InternalConfigureTags(TAG * tags)
	{
	STFRES_RAISE_OK;
	
	}

STFResult DVDSCRMultiplexerUnit::InternalUpdate(void)	
	{
	STFRES_RAISE_OK;
	
	}

STFResult DVDSCRMultiplexerUnit::Create(uint64 * createParams)
	{
	if (GetNumberOfParameters(createParams) != 0)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}

STFResult DVDSCRMultiplexerUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// IVDRTagUnit interface implementation
///////////////////////////////////////////////////////////////////////////////

//
// ITagUnit interface implementation
//
STFResult DVDSCRMultiplexerUnit::GetTagIDs(VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_ENCODER_CONFIG,
		VDRTID_DONE
		};

	ids = (VDRTID *)supportedTagTypes;

	STFRES_RAISE_OK;
	}

VirtualDVDSCRMultiplexerUnit::VirtualDVDSCRMultiplexerUnit(IPhysicalUnit * physical)
	: VirtualStreamingUnit(physical), 
	  MultipleStreamingParser(DVDSCRMSID_TOTAL),
	  outputConnector(16, 3, this)
	{
	uint32	i;

	for (i=0; i<DVDSCRMSID_TOTAL; i++) 
		inputConnectors[i] = NULL;

	mergedStreamTags = NULL;
	}

VirtualDVDSCRMultiplexerUnit::~VirtualDVDSCRMultiplexerUnit(void)
	{
	uint32	i;

	for (i=0; i<DVDSCRMSID_TOTAL; i++) 
		delete inputConnectors[i];

	delete[] mergedStreamTags;
	}

static const int	AC3FrameDuration108MHz	=	3456000;
static const int	NTSCFrameDuration108MHz	=	3603600;
static const int	PALFrameDuration108MHz	=	4320000;

///
///@brief Initializes the input connectors for Audio,Video, and SP types
///
STFResult VirtualDVDSCRMultiplexerUnit::Initialize(void)
	{
	uint32 i;

//	inputConnectors[DVDSCRMSID_AUDIO] = new QueuedInputConnector(32, 1, 0, this);

	inputConnectors[DVDSCRMSID_AUDIO] = new UnqueuedInputConnector(0, this);
	assert(inputConnectors[DVDSCRMSID_AUDIO]);

	inputConnectors[DVDSCRMSID_VIDEO] = new UnqueuedInputConnector(1, this);
	assert(inputConnectors[DVDSCRMSID_VIDEO]);

	inputConnectors[DVDSCRMSID_SPU  ] = new UnqueuedInputConnector(2, this);
	assert(inputConnectors[DVDSCRMSID_SPU  ]);

	STFRES_REASSERT(AddConnector(inputConnectors[DVDSCRMSID_AUDIO]));
	STFRES_REASSERT(AddConnector(inputConnectors[DVDSCRMSID_VIDEO]));
	STFRES_REASSERT(AddConnector(inputConnectors[DVDSCRMSID_SPU  ]));
	STFRES_REASSERT(AddConnector(&outputConnector));

	STFRES_REASSERT(outputFormatter.SetOutputConnector(&outputConnector));

	//
	// Elementary buffer sizes 4, 232 and 58 based on DVD spec Table 5.3-1
	//
	STFRES_REASSERT(streams[DVDSCRMSID_AUDIO].Initialize(16,   4 * 1024, 0xbd, 0x80, STFHiPrec64BitDuration(AC3FrameDuration108MHz, STFTU_108MHZTICKS)));
	STFRES_REASSERT(streams[DVDSCRMSID_VIDEO].Initialize(32, 232 * 1024, 0xe0, 0xff, STFHiPrec64BitDuration(PALFrameDuration108MHz, STFTU_108MHZTICKS)));
	STFRES_REASSERT(streams[DVDSCRMSID_SPU  ].Initialize(32,  58 * 1024, 0xbd, 0x20, STFHiPrec64BitDuration(PALFrameDuration108MHz, STFTU_108MHZTICKS)));

	inputTVSourceSystem = VSTD_PAL;

	for (i=0; i<DVDSCRMSID_TOTAL; i++)
		{
		pendingPackets[i] = NULL;
		packetBounced[i]  = false;	
		}
	STFRES_RAISE(Reset());
	}

STFResult VirtualDVDSCRMultiplexerUnit::InternalConfigureTags(TAG *tags)
	{
	uint32	changeSet;

	PARSE_TAGS_START(tags, changeSet)
		GETSETC(VIDEO_TV_STANDARD,             inputTVSourceSystem, 0);
	PARSE_TAGS_END

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::InternalEndConfigure(void)
	{
	switch (inputTVSourceSystem)
		{
		case VSTD_NTSC:
			// 3603600 is NTSC frame duration in 108MHz units
			streams[DVDSCRMSID_VIDEO].frameDuration = 
			streams[DVDSCRMSID_SPU  ].frameDuration = STFHiPrec64BitDuration(NTSCFrameDuration108MHz, STFTU_108MHZTICKS);
			break;
		case VSTD_SECAM:
		case VSTD_PAL:
			// 4320000 is PAL frame duration in 108MHz units
			streams[DVDSCRMSID_VIDEO].frameDuration = 
			streams[DVDSCRMSID_SPU  ].frameDuration = STFHiPrec64BitDuration(PALFrameDuration108MHz, STFTU_108MHZTICKS);
			break;
		default:
			STFRES_RAISE(STFRES_INVALID_PARAMETERS);
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}

///
/// @brief This method does parsing of the input packet.
/// 
/// This method calls the ParseReceivePacket() on the received packet
///	and returns the return value of the call. If the call is successful
/// it releases the ranges in the packet. If required, this call also 
/// generate a packet request on the input connector.
///
STFResult VirtualDVDSCRMultiplexerUnit::ProcessPendingPackets(void)
	{
	STFResult	result = STFRES_OK;
	uint32		i;

	//
	// Ensure only one path of control is inside the parsing segment,
	// we have to retry on collision, otherwise a packet request may
	// get lost due to a racing condition (thread rescheduled after
	// delivery failed, but before locking area left).
	//
	processRequest = true;
	do 
		{
		if (!pendingLock++)
			{
			processRequest = false;
			// Are we in the process of flushing?
			if (flushRequest)
				{
				result = ProcessFlushing();	// For further flush processing by a derived class

				for (i=0; i<DVDSCRMSID_TOTAL; i++)
					{
					inputConnectors[i]->FlushPackets();

					if (pendingPackets[i])
						{
						pendingPackets[i]->ReleaseRanges();
						pendingPackets[i]->RemPacketOwner(this);
						pendingPackets[i]->ReturnToOrigin();
						pendingPackets[i] = NULL;
						}

					MultipleStreamingParser::Flush(i);
					}

				Reset();

				this->SignalStreamingCommandCompletion(VDR_STRMCMD_FLUSH, result);

				flushRequest = false;
				}
			else 
				{
				//
				// Run through all streams once
				//
				streamProgressFlags = MKFLAG(DVDSCRMSID_TOTAL) - 1;

				while (streamProgressFlags)
					{
					for (i=0; i<DVDSCRMSID_TOTAL; i++)
						{
						if (XTBF(i, streamProgressFlags))
							{
							streamProgressFlags &= ~MKFLAG(i);

							if (pendingPackets[i])
								{
								//
								// Attempt to deliver the packet
								//
								result = this->ParseReceivePacket(i, pendingPackets[i]);					

								if (result != STFRES_OBJECT_FULL)
									{

									//
									// If the packet was consumed, or an error other than object
									// full appeared, remove the packet from the pending state.
									//
									pendingPackets[i]->ReleaseRanges();
									pendingPackets[i]->RemPacketOwner(this);
									pendingPackets[i]->ReturnToOrigin();
									pendingPackets[i] = NULL;
									}
								else
									{
									//
									// In case of input queue full, we keep the packet and signal
									// an ok.
									//
									result = STFRES_OK;
									}
								}

							if (packetBounced[i] && pendingPackets[i] == NULL)
								{
								packetBounced[i] = false;
								inputConnectors[i]->RequestPackets();
								}
							}
						}
					}
				}
			}
		} while (--pendingLock == 0 && processRequest);

	STFRES_RAISE(result);
	}

STFResult VirtualDVDSCRMultiplexerUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	packetBounced[connectorID] = false;

	//
	// Process a possibly pending packet
	//
	STFRES_REASSERT(this->ProcessPendingPackets());

	//
	// Process new packet if there is no current pending packet
	//
	packetBounced[connectorID] = true;	// To avoid racing condition, we set it to true here

	if (pendingPackets[connectorID] == NULL)
		{
		//
		// Make the new packet the pending packet and try to process it
		//
		packetBounced[connectorID] = false;
		packet->AddPacketOwner(this);
		pendingPackets[connectorID] = packet;
		STFRES_RAISE(this->ProcessPendingPackets());
		}
	else
		{
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}


STFResult VirtualDVDSCRMultiplexerUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	packetBounced[connectorID] = true;
	STFRES_RAISE(ProcessPendingPackets());
	}


STFResult VirtualDVDSCRMultiplexerUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	uint32	i;

	//
	// If it is a packet request, we might be able to satisfy it immediately
	//
	if (message == VDRMID_STRM_PACKET_REQUEST || message == VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE)
		{
		// Process a pending packet, this may lead to further packet request upstream notifications
		this->ProcessPendingPackets();

		STFRES_RAISE_OK;
		}

	//
	// Forward the request to the upstream filter via the input connector
	//
	for (i=0; i<DVDSCRMSID_TOTAL; i++)
		STFRES_REASSERT(inputConnectors[i]->SendUpstreamNotification(message, param1, param2));

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDSCRMultiplexerUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	STFResult result = STFRES_OK;

	switch (command)
		{
		case VDR_STRMCMD_BEGIN:		
			this->SignalStreamingCommandCompletion(command, result);
			break;

		case VDR_STRMCMD_FLUSH:
			// Call process pending packet in order to get rid of the pending packet.
			flushRequest = true;	// Signal that a flush is requested
			result = ProcessPendingPackets();			
			break;

		default:
			this->SignalStreamingCommandCompletion(command, result);
		}


	STFRES_RAISE(result);
	}

STFResult VirtualDVDSCRMultiplexerUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	uint32	i;

	STFRES_REASSERT(StreamingUnit::CompleteStreamingCommand(command, targetState));
	if (this->state == VDR_STRMSTATE_READY)
		{
		for (i=0; i<DVDSCRMSID_TOTAL; i++)
			packetBounced[i] = true;
		ProcessPendingPackets();
		} 

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDSCRMultiplexerUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_MPEG_FRAME_INFO,
		VDRTID_DONE
		};

	VDRTID	*	downstreamTags;

	if (!mergedStreamTags)
		{		
		STFRES_REASSERT(outputConnector.GetStreamTagIDs(downstreamTags));
		STFRES_REASSERT(MergeTagTypeIDList(supportedTagTypes, downstreamTags, mergedStreamTags));
		}

	ids = mergedStreamTags;

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDSCRMultiplexerUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	uint32	i;

	for (i=0 ; i<DVDSCRMSID_TOTAL; i++)
		STFRES_REASSERT(inputConnectors[i]->ProvideAllocator(allocator));

	STFRES_RAISE_OK;
	}

VirtualDVDSCRMultiplexerUnit::SCRStream::SCRStream(void)
	{
	aunits = NULL;
	size   = 0;
	}

VirtualDVDSCRMultiplexerUnit::SCRStream::~SCRStream(void)
	{
	delete[] aunits;
	}

STFResult VirtualDVDSCRMultiplexerUnit::SCRStream::Initialize(uint64 queueSize, uint32 bufferSize, uint8 streamID, uint8 subStreamID, const STFHiPrec64BitDuration & frameDuration)
	{	
	if (queueSize > size)
		{
		mask = 1;
		while (mask < queueSize)
			mask <<= 1;
		size = mask;
		mask -= 1;
		delete[] aunits;
		aunits = new SCRAccessUnit[size];
		assert(aunits);
		}

	this->bufferSize      = bufferSize;
	this->mpegStreamID    = streamID;
	this->mpegSubStreamID = subStreamID;
	this->frameDuration   = frameDuration;
	
	STFRES_RAISE(Reset());
	}

STFResult VirtualDVDSCRMultiplexerUnit::SCRStream::Reset(void)
	{
	first = last          = 0;

	bufferLevel           = 0;
	skipSize		          = 0;
	payloadSize           = 0;
	packetPayloadSize     = 0;
	packetStuffingSize    = 0;
	frameOffset           = 0;
	frameCount            = 0;
	frameType             = MPFT_INVALID;
	referenceFrameCount   = 0;
	referenceFrameMask    = 0;
	range.size            = 0;
	temporalReference     = 0;

	presentationTimeValid = false;
	decodingTimeValid     = false;
	groupStartTimeValid   = false;
	openSegment           = true;	  

	firstPackInVOB        = true;
	firstPackInVOBU       = true;
	firstPackOfVOBU       = false;

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::Reset(void)
	{
	uint32	i;

	flushRequest      = false;
	processRequest    = false;
	multiplexTime     = ZERO_HI_PREC_64_BIT_TIME;

	// based on 2048 Bytes packet size and 10.08MBits/s max DVD bitrate
	packetDuration    = STFHiPrec64BitDuration(175544, STFTU_108MHZTICKS);

	// Based on DVD video bitbuffer size and maximum video bitrate.
	// This is the time required to fill the video buffer to max level.
	maxSCRtoDTSOffset	= STFHiPrec64BitDuration(   225, STFTU_MILLISECS);

	for (i=0; i<DVDSCRMSID_TOTAL; i++)
		{		
		streams[i].Reset();
		}

	numPendingTags  = 0;
	deliverStreamID = DVDSCRMSID_TOTAL;
	groupID         = 0;
	numPacksInVOBU  = 0;
	dstate          = SCRMXS_PARSE;

//[BS]
#if DEBUG_BITRATE
	GopSize = 0;
	GOPBitrate = 0;
	SegmentBitrate = 0;
	GOPCount = 0;
	packCount = 0;
	maxPackCount = 0;
	maxSegmentBitrate = 0;
#endif
	videoFrameCountInCurrVOBU = 0;
	audioFrameCountInCurrVOBU = 0;
//[BS]

	STFRES_RAISE_OK;
	}

///
/// @brief Release the pending ranges for all elementary streams
///
STFResult VirtualDVDSCRMultiplexerUnit::ProcessFlushing(void)
	{
	uint32	i;

	//
	// Release the pending ranges for all elementary streams
	//
	for (i=0; i<DVDSCRMSID_TOTAL; i++)
		{
		if (streams[i].range.size > 0)
			{
			streams[i].range.Release(this);
			streams[i].range.size = 0;
			}
		}

	outputFormatter.Flush();

	STFRES_RAISE_OK;
	}

///
/// @brief This function reduces the occupency of the simulated decoding buffer size by the size of
///			the access unit whose decoding time has arrived.
/// @param	multiplex time: Represent the current multiplexer time (clock value)
///
STFResult VirtualDVDSCRMultiplexerUnit::SCRStream::DequeueExtractedPayload(const STFHiPrec64BitTime & multiplexTime)
	{
	while (first != last && aunits[first & mask].extractionTime <= multiplexTime)
		{
		bufferLevel -= aunits[first & mask].payloadSize;
		first++;
		}

	STFRES_RAISE_OK;
	}

///
/// @brief This function determines whether current packet can be multiplexed or not.
/// @param	multiplex time: Represent the current multiplexer time (clock value)
///			maxSCRtoDTSOffset: Represents the maximum allowable difference between the DTS and the current
///			multiplex time.
///
bool VirtualDVDSCRMultiplexerUnit::SCRStream::CanMux(const STFHiPrec64BitTime & multiplexTime, const STFHiPrec64BitDuration & maxSCRtoDTSOffset) 
	{
	return (bufferLevel + 2048 <= bufferSize) && 
		    (!decodingTimeValid || (decodingTime <= multiplexTime + maxSCRtoDTSOffset)) && 
			 openSegment;
	}

///
/// @brief This function determines what is the nearest time to which the current time can be moved to.
///		   This step is reuired when no more data can be multiplexed due to decoding buffer fullness.
/// @param	event time: Represent the current selected time.
///			maxSCRtoDTSOffset: Represents the maximum allowable difference between the DTS and the current
///			multiplex time.
///
STFResult VirtualDVDSCRMultiplexerUnit::SCRStream::FindClosestEventTime(const STFHiPrec64BitDuration & maxSCRtoDTSOffset, STFHiPrec64BitTime & eventTime, bool & eventTimeValid)
	{
	if (first != last)
		{
		if (!eventTimeValid || aunits[first & mask].extractionTime < eventTime)
			{
			eventTime = aunits[first & mask].extractionTime;
			eventTimeValid = true;
			}
		}

	//[BS] 
#if 0
	if (decodingTimeValid)
		{
		if (!eventTimeValid || decodingTime - maxSCRtoDTSOffset < eventTime)
			{
			eventTime = decodingTime - maxSCRtoDTSOffset;
			eventTimeValid = true;
			}
		}
#else
	if (decodingTimeValid)
		{
		if (bufferLevel + 2048 <= bufferSize && (!eventTimeValid || decodingTime - maxSCRtoDTSOffset < eventTime))
			{
			eventTime = decodingTime - maxSCRtoDTSOffset;
			eventTimeValid = true;
			}
		}
#endif
	//[BS]

	STFRES_RAISE_OK;
	}

///
/// @brief this function selects which type (audio/video/SP) of packet to be multiplexed next.
///

STFResult VirtualDVDSCRMultiplexerUnit::MultiplexPackets(uint32 streamID)
	{
	uint32					multiplexID;
	bool						advanced, eventTimeValid;
	STFHiPrec64BitTime	eventTime;

	if (streams[streamID].range.size == 2048)
		{
		do {
			advanced = false;

			//
			// Attempt to send a pending packet before generating a new one
			//
			STFRES_REASSERT(SendPendingPacket());

			//
			// Remove all due payload packet from the virtual elementary decoder input buffer
			//
			for (multiplexID = DVDSCRMSID_AUDIO; multiplexID < DVDSCRMSID_TOTAL; multiplexID++)
				{
				STFRES_REASSERT(streams[multiplexID].DequeueExtractedPayload(multiplexTime));
				}

#if 0
			DPR("MUX A:%d(%d,%d) V:%d(%d,%d) S:%d(%d,%d)\n", 
				streams[DVDSCRMSID_AUDIO].bufferLevel, streams[DVDSCRMSID_AUDIO].first, streams[DVDSCRMSID_AUDIO].last,
				streams[DVDSCRMSID_VIDEO].bufferLevel, streams[DVDSCRMSID_VIDEO].first, streams[DVDSCRMSID_VIDEO].last,
				streams[DVDSCRMSID_SPU  ].bufferLevel, streams[DVDSCRMSID_SPU  ].first, streams[DVDSCRMSID_SPU  ].last);
#endif

			//
			// Search for highest priority stream with an open slot in the input buffer.  The first pack of
			// each VOBU should be a video pack, therefore we may not multiplex any audio before the first
			// video pack in the VOB.
			//
			multiplexID = (groupID || numPacksInVOBU) ? DVDSCRMSID_AUDIO : DVDSCRMSID_VIDEO;
			while ( multiplexID < DVDSCRMSID_TOTAL && !streams[multiplexID].CanMux(multiplexTime, maxSCRtoDTSOffset))				
				multiplexID++;

#if DEBUG_BITRATE
			//[BS]
			if (multiplexID == DVDSCRMSID_AUDIO && streams[DVDSCRMSID_VIDEO].CanMux(multiplexTime, maxSCRtoDTSOffset))
			{
				// check if video needs to be sent first
				if ((streams[DVDSCRMSID_AUDIO].decodingTime > streams[DVDSCRMSID_VIDEO].decodingTime)
					&& (multiplexTime + packetDuration > streams[DVDSCRMSID_VIDEO].decodingTime))
				{
					multiplexID++;
				}
			}
			

			if (multiplexID == DVDSCRMSID_VIDEO && streams[DVDSCRMSID_VIDEO].frameType == MPFT_IFRAME && 
				multiplexTime > streams[DVDSCRMSID_VIDEO].decodingTime)
			{
				DPR("###########OVERFLOW############\n");
			}
			//[BS]
#endif

			// 
			// Special check for subpicture, SPU must be part of VOBU, so we can not advance with the video
			// to the next VOBU unless we have sent all pending SPU packets
			//
			if (multiplexID == DVDSCRMSID_VIDEO && 
				 streams[DVDSCRMSID_VIDEO].firstPackOfVOBU && 
				 streams[DVDSCRMSID_VIDEO].decodingTimeValid &&
				 streams[DVDSCRMSID_SPU].range.size &&
				 (!streams[DVDSCRMSID_SPU].presentationTimeValid || streams[DVDSCRMSID_SPU].presentationTime < streams[DVDSCRMSID_VIDEO].decodingTime))
				{
				if (streams[DVDSCRMSID_SPU].bufferLevel + 2048 <= streams[DVDSCRMSID_SPU].bufferSize)
					multiplexID = DVDSCRMSID_SPU;
				else
					multiplexID = DVDSCRMSID_TOTAL;
				}

			//
			// If there is an open slot, attempt to fill it with a pending packet of the matching stream
			//
			if (multiplexID < DVDSCRMSID_TOTAL)
				{
				if (streams[multiplexID].range.size == 2048)
					{
					STFRES_REASSERT(MultiplexPacket(multiplexID));
					advanced = true;
					streamProgressFlags |= MKFLAG(multiplexID);
					}
				}

			//
			// Check if we need to wait for data.  Multipexer will wait for video and audio data,
			// but not for subpicture data.
			//
			if ( (multiplexID == DVDSCRMSID_TOTAL) || 
				  (!advanced && (multiplexID == DVDSCRMSID_SPU)) )
				{
				//
				// No buffer available, advance to next event time.
				//
				eventTimeValid = false;
				for (multiplexID = DVDSCRMSID_AUDIO; multiplexID < DVDSCRMSID_TOTAL; multiplexID++)
					{
					STFRES_REASSERT(streams[multiplexID].FindClosestEventTime(maxSCRtoDTSOffset, eventTime, eventTimeValid));
					}

				if (eventTimeValid && eventTime > multiplexTime)
					{
					multiplexTime = eventTime;
					advanced = true;
					}
				}
			} while (advanced);
		}

	if (streams[streamID].range.size == 2048)
		STFRES_RAISE(STFRES_OBJECT_FULL);
	else
		STFRES_RAISE_OK;
	}

///
/// @brief This function sends the multiplexed packet to the next unit (Navigation Layer).
///			Appropriate TAGs are generated and flags are set.
///
STFResult VirtualDVDSCRMultiplexerUnit::SendPendingPacket(void)
	{
	SCRStream	*	q = streams + deliverStreamID;
	uint32			i;

	switch (dstate)
		{
		case SCRMXS_DELIVER_END_TIME:
			//
			// Deliver an end time for the complete VOBU
			//
			if (!q->firstPackInVOB && !numPacksInVOBU)
				{
				if (q->groupStartTimeValid)
					STFRES_REASSERT(outputFormatter.PutEndTime(q->groupStartTime));
				}

			dstate = SCRMXS_DELIVER_GROUP_END;
		case SCRMXS_DELIVER_GROUP_END:
			//
			// Check for first pack of a VOBU
			//
			if (!numPacksInVOBU)
				{
				//
				// Start a segment, if this is the first packet of the VOB, otherwise
				// complete the previous group
				//
				if (q->firstPackInVOB)
					STFRES_REASSERT(outputFormatter.BeginSegment(0, false));
				else
					{
					STFRES_REASSERT(outputFormatter.CompleteGroup(false));
					groupID++;
					}
				}
			q->firstPackInVOB = false;

			dstate = SCRMXS_DELIVER_START_TIME;
		case SCRMXS_DELIVER_START_TIME:
			//
			// Deliver a start time for the complete VOBU
			//
			if (!numPacksInVOBU)
				{
				if (q->groupStartTimeValid)
					STFRES_REASSERT(outputFormatter.PutStartTime(q->groupStartTime));
				}

			q->presentationTimeValid = false;
			q->decodingTimeValid     = false;
			q->groupStartTimeValid   = false;

			dstate = SCRMXS_DELIVER_GROUP_START;
		case SCRMXS_DELIVER_GROUP_START:
			//
			// Check for first pack of a VOBU
			//
			if (!numPacksInVOBU)
				{
				//
				// If so, begin the next group
				//
				STFRES_REASSERT(outputFormatter.BeginGroup(groupID, false, false));

				//
				// Notify all other streams, that a new VOBU has started
				//
				for (i=0; i<DVDSCRMSID_TOTAL; i++)
					streams[i].firstPackInVOBU = true;
				}
			q->firstPackInVOBU = false;

			numSentTags = 0;
			dstate = SCRMXS_DELIVER_TAGS;
		case SCRMXS_DELIVER_TAGS:
			//
			// Deliver all pending TAGS
			//
			while (numSentTags < numPendingTags)
				{
				STFRES_REASSERT(outputFormatter.PutTag(pendingTags[numSentTags]));
				numSentTags++;
				}

			dstate = SCRMXS_DELIVER_COMPLETE_TAGS;
		case SCRMXS_DELIVER_COMPLETE_TAGS:
			//
			// Complete TAG delivery
			//
			if (numPendingTags)
				{
				STFRES_REASSERT(outputFormatter.CompleteTags());
				numPendingTags = 0;
				}

			dstate = SCRMXS_DELIVER_RANGE;
		case SCRMXS_DELIVER_RANGE:
			STFRES_REASSERT(outputFormatter.PutRange(q->range));
			q->range.Release(this);
			q->range.size = 0;

			numPacksInVOBU++;

			dstate = SCRMXS_PARSE;
		default:
			STFRES_RAISE_OK;
		}
	}

///
/// @brief Returns the multiplex time in the form of SCRBase and SCRExt
///
void VirtualDVDSCRMultiplexerUnit::GetSCR(uint32 & scrBase, uint32 & scrExt)
	{
	STFInt64			scr;

	//
	// Extract SCR base and SCR extension from the multiplex time
	//
	scr = multiplexTime.Get64BitTime(STFTU_108MHZTICKS);
	scr >>= 2;

	scrBase = (scr / 300).Lower();
	scrExt  = (scr - STFInt64(scrBase) * 300).Lower();
	}

///
/// @brief Calculates the SCR value and generates some TAG information if this is begining of
///			new the VOBU.
///
STFResult VirtualDVDSCRMultiplexerUnit::MultiplexPacket(uint32 streamID)
	{
	SCRStream	*	q = streams + streamID;
	uint8			*	pp = q->range.GetStart();
	uint32			scrBase, scrExt;
	//bool				firstPackOfVOBU = q->firstPackOfVOBU; // [NHV] Removed due to compilerwarning

	//
	// Start a new VOBU if this is the initial packet of one
	//
	if (q->firstPackOfVOBU)
		{
		q->firstPackOfVOBU = false;

		numPacksInVOBU = 0;

#if 0
		//
		// Ensure that the SCR does not advance to far away
		//
		if (q->decodingTimeValid && q->decodingTime - maxSCRtoDTSOffset > multiplexTime)
			multiplexTime = q->decodingTime - maxSCRtoDTSOffset;
#endif

//[BS]
#if DEBUG_BITRATE
		packCount++; //extraNavPack
#endif
//[BS]

		//
		// Keep space for nav pack
		//
		GetSCR(scrBase, scrExt);
		pendingTags[numPendingTags++] = SET_ENCODED_VOBU_SCR_BASE(scrBase);
		pendingTags[numPendingTags++] = SET_ENCODED_VOBU_SCR_EXTENSION(scrExt);
		multiplexTime += packetDuration;
		}

//[BS]
#if DEBUG_BITRATE
		packCount++; //extraNavPack
#endif
//[BS]

	//
	// Send packet positions to NV_PCK formatter
	//
	//[BS]
	if (XTBF(0, q->referenceFrameMask) && (XTBF(1, q->referenceFrameMask) || (q->frameType == MPFT_BFRAME)))
	{
		//DPR("Number Of Packs 1st REF =%d\n", numPacksInVOBU);
	pendingTags[numPendingTags++] = SET_ENCODED_VOBU_1ST_REF_EA(numPacksInVOBU);	
	q->referenceFrameMask &= ~MKFLAG(0);
	}

	if (XTBF(1, q->referenceFrameMask) && (XTBF(2, q->referenceFrameMask) || (q->frameType == MPFT_BFRAME)))
	{
	pendingTags[numPendingTags++] = SET_ENCODED_VOBU_2ND_REF_EA(numPacksInVOBU);	
	q->referenceFrameMask &= ~MKFLAG(1);
	}

	if (XTBF(2, q->referenceFrameMask) && (XTBF(3, q->referenceFrameMask) || (q->frameType == MPFT_BFRAME)))
	{
	pendingTags[numPendingTags++] = SET_ENCODED_VOBU_3RD_REF_EA(numPacksInVOBU);	
	q->referenceFrameMask &= ~MKFLAG(2);
	}
	//[BS]
	if (streamID == DVDSCRMSID_AUDIO && q->firstPackInVOBU)
		pendingTags[numPendingTags++] = SET_ENCODED_VOBU_A_PCKA(numPacksInVOBU);
	
#if 0
	DPR("MPX %d FVP %d SCR %5d EXT %5d DTS %5d PTS %5d GTS %5d\n", streamID, firstPackOfVOBU, multiplexTime.Get32BitTime(), 
		q->extractionTime.Get32BitTime(),
		q->decodingTimeValid     ? q->decodingTime.Get32BitTime() : -1, 
		q->presentationTimeValid ? q->presentationTime.Get32BitTime() : -1,
		q->groupStartTimeValid   ? q->groupStartTime.Get32BitTime() : -1);
#endif

	//
	// Put SCR in PACK header, according to ISO 13818.1 Table 2-33
	//
	GetSCR(scrBase, scrExt);
	pp[4] = 0x44 | (scrBase >> 27) & 0x38 | (scrBase >> 28) & 0x03;
	pp[5] =        (scrBase >> 20) & 0xff;
	pp[6] = 0x04 | (scrBase >> 12) & 0xf8 | (scrBase >> 13) & 0x03;
	pp[7] =        (scrBase >>  5) & 0xff;
	pp[8] = 0x04 | (scrBase <<  3) & 0xf8 | (scrExt  >>  7) & 0x03;
	pp[9] = 0x01 | (scrExt  <<  1) & 0xfe;

	//
	// Advance SCR timer by a packet deliver duration
	//
	multiplexTime += packetDuration;

	//
	// Fill virtual decoder buffer
	//
	q->bufferLevel += q->packetPayloadSize;
	q->packetPayloadSize = 0;
	q->packetStuffingSize = 0;

	//
	// Start delivery of this packet
	//
	deliverStreamID = streamID;
	dstate = SCRMXS_DELIVER_END_TIME;

	STFRES_RAISE_OK;
	}
///
/// @brief This function formats the packet, inserts pack/packet header and stuffing
///			bytes if required.
///
STFResult VirtualDVDSCRMultiplexerUnit::FormatPacket(uint32 streamID)
	{
	SCRStream	*	q = streams + streamID;
	uint8			*	pp = q->range.GetStart();
	uint32			packetSize, tstamp, offset;
	uint32			hp;
	uint32			stuffingSize = q->packetStuffingSize + 2048 - q->range.size;
	
	q->range.size = 2048;

	//
	// Pack header start code, according to ISO 13818.1 Table 2-33
	//
	pp[ 0] = 0x00;
	pp[ 1] = 0x00;
	pp[ 2] = 0x01;
	pp[ 3] = 0xba;

	//
	// SCR delayed until muxing, will be written in ::MultiplexPacket
	// 

	//
	// Mux rate, fixed for DVD to 10.08MBps, according to DVD Spec Part 3, Table 5.2.1-2
	//
	pp[10] = 0x01;
	pp[11] = 0x89;
	pp[12] = 0xc3;
	pp[13] = 0xf8;

	//
	// Packet header start code, according to ISO 13818.1 Table 2-18
	//
	pp[14] = 0x00;
	pp[15] = 0x00;
	pp[16] = 0x01;
	pp[17] = q->mpegStreamID;

	//
	// Calculate effective packet size, based on DVD sector size and amount of stuffing
	//
	if (stuffingSize >= 8)
		packetSize = 2048 - 20 - stuffingSize;
	else
		packetSize = 2048 - 20;

	//
	// Packet size
	//
	pp[18] = packetSize >> 8;
	pp[19] = packetSize & 0xff;

	//
	//	Set packet flags
	// 

	pp[20] = 0x81;
	pp[21] = MKBFB(7, q->presentationTimeValid) |
		      MKBFB(6, q->decodingTimeValid) |
				MKBFB(0, q->firstPackInVOB);


	//
	// Extended header size delayed until all other fields are written
	//
	hp = 23;

	//
	// Write presentation time
	//
	if (q->presentationTimeValid)
		{
//[BS]
#if 1
		tstamp = q->presentationTime.Get32BitTime(STFTU_90KHZTICKS);
#else
		uint32 tstampMS = q->presentationTime.Get32BitTime(STFTU_MILLISECS);
		tstamp = tstampMS * 90;
#endif
//[BS]


		pp[hp++] = 0x21 | (tstamp >> 29) & 0x06 | MKBF(4, q->decodingTimeValid); // there is no bit 32 in DVD PTS
		pp[hp++] =        (tstamp >> 22) & 0xff;
		pp[hp++] = 0x01 | (tstamp >> 14) & 0xfe;
		pp[hp++] =        (tstamp >>  7) & 0xff;
		pp[hp++] = 0x01 | (tstamp <<  1) & 0xfe;
		}
	else if (streamID == DVDSCRMSID_AUDIO) // Gabargel
		{
		hp += 5;
		DP("Apply audio packet start fix\n");
		}

	//
	// Write decoding time
	//
	if (q->decodingTimeValid)
		{
//[BS]
#if 1
		tstamp = q->decodingTime.Get32BitTime(STFTU_90KHZTICKS);
#else
		uint32 tstampMS = q->decodingTime.Get32BitTime(STFTU_MILLISECS);
		tstamp = tstampMS * 90;
#endif
//[BS]


		pp[hp++] = 0x11 | (tstamp >> 29) & 0x06; // there is no bit 32 in DVD PTS
		pp[hp++] =        (tstamp >> 22) & 0xff;
		pp[hp++] = 0x01 | (tstamp >> 14) & 0xfe;
		pp[hp++] =        (tstamp >>  7) & 0xff;
		pp[hp++] = 0x01 | (tstamp <<  1) & 0xfe;
		}

	//
	// Write extended header data for first pack in VOB
	//

	//[BS]
#if 0
	if (q->firstPackInVOB || streamID == DVDSCRMSID_AUDIO) // Check for streamID should not be here...
#else
	if (q->firstPackInVOB)
#endif
	//[BS]
		{
		pp[hp++] = 0x1e;
		pp[hp++] = 0x60;

		switch (streamID)
			{
			case DVDSCRMSID_VIDEO:
				pp[hp++] = 232;
				break;
			case DVDSCRMSID_AUDIO:
				pp[hp++] = 58;	// might be 4, but seems to be a prob in standard
				break;
			case DVDSCRMSID_SPU:
				pp[hp++] = 58;
				break;
			}
		}
	//[BS]
#if 0
	if (streamID == DVDSCRMSID_AUDIO)
		assert(stuffingSize == 0 && hp == 31);
#else
	if (streamID == DVDSCRMSID_AUDIO)
		assert(stuffingSize == 0);
#endif
	//[BS]

	//
	// Apply small stuffing as extended header data
	//
	if (stuffingSize > 0 && stuffingSize < 8)
		{
		memmove(pp + hp + stuffingSize, pp + hp, 2048 - hp - stuffingSize);

		// convert the stuffing bytes to 0xff
		uint8 *buffer = pp + hp;

		for (uint32 byteCount = 0; byteCount < stuffingSize; byteCount++)
			buffer[byteCount] = 0xff;

		hp += stuffingSize;
		}

	//
	// Set extended header size
	//
	pp[22] = hp - 23;

	//
	// Put extension streamID
	//
	if (q->mpegStreamID == 0xbd)
		{
		pp[hp++] = q->mpegSubStreamID;

		switch (q->mpegSubStreamID & 0xf8)
			{
			case 0x80:
				//
				// AC3, put frame count and frame offset
				//
				pp[hp++] = q->frameCount;
				if (q->frameCount)
					offset = q->frameOffset - hp - 1;
				else
					offset = 0;
				pp[hp++] = offset >> 8;
				pp[hp++] = offset & 0xff;

				q->frameCount = 0;
				q->frameOffset = 0;
				break;
			case 0x20:
			case 0x28:
			case 0x30:
			case 0x38:
				break;
			}
		}

	//
	// Apply large stuffing using an independend stuffing packet
	//
	if (stuffingSize >= 8)
		{
		pp += 2048 - stuffingSize;

		pp[0] = 0x00;
		pp[1] = 0x00;
		pp[2] = 0x01;
		pp[3] = 0xbe;

		pp[4] = (uint8)((stuffingSize-6)>>8);
		pp[5] = (uint8)((stuffingSize-6) & 0xff);

		memset(pp + 6, 0xff, stuffingSize - 6);
		}

	STFRES_RAISE_OK;
	}
///
/// @brief This function is called to signal that there is no more data comming for the previous 
///			access unit. The decoding buffer occupancy is increased by the payload size of the 
///			Access unit.
///
STFResult VirtualDVDSCRMultiplexerUnit::CompleteAccessUnit(uint32 streamID)
	{
	SCRStream				*	q = streams + streamID;

	//
	// This is probably the end of an access unit, so we put the payload size into
	// the virtual elementary decoder buffer queue.
	//
	if (q->payloadSize)
		{
		if (q->last - q->first < q->size)
			{
			q->aunits[q->last & q->mask].extractionTime = q->extractionTime;
			q->aunits[q->last & q->mask].payloadSize    = q->payloadSize;
			q->last++;
			q->payloadSize = 0;
			}
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseBeginGroup(uint32 streamID, uint16 groupNumber, bool sendNotification, bool singleUnitGroup)
	{
	SCRStream				*	q = streams + streamID;

	if (streamID == DVDSCRMSID_VIDEO)
		{
		//
		// Reset VOBU specific fields
		//
		q->referenceFrameCount = 0;
		q->referenceFrameMask  = 0;
		q->firstPackOfVOBU = true;
		q->firstPackInVOBU = true;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseEndGroup(uint32 streamID, uint16 groupNumber, bool sendNotification)
	{
	SCRStream				*	q = streams + streamID;

	//
	// This marks the end of a frame, so we should close any still pending
	// access units
	//
	STFRES_REASSERT(CompleteAccessUnit(streamID));

	if (streamID == DVDSCRMSID_VIDEO)
		{
		if (q->range.size > 0)
			{
			//
			// If the packet has not yet been completely formatted, we apply the missing amount
			// of stuffing and sent it off to the multiplexer.
			// 
			if (q->range.size < 2048)
				{
				DP("End group with incomplete packets\n");
				STFRES_REASSERT(FormatPacket(streamID));
				}

			//
			// End of a VOBU, will have an incomplete data range at the end, we have to send it, before
			// we can accept new data
			//
			STFRES_REASSERT(MultiplexPackets(streamID));
			}
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseStartTime(uint32 streamID, const STFHiPrec64BitTime & time)
	{
	SCRStream				*	q = streams + streamID;
	STFHiPrec64BitTime		presentationTime = time + maxSCRtoDTSOffset;
	STFHiPrec64BitTime		extractionTime = presentationTime;

	//
	// This marks the start of a new frame, so we should close any still pending
	// access units
	//
	STFRES_REASSERT(CompleteAccessUnit(streamID));

	//
	// Deliver all still pending full packets
	//
	STFRES_REASSERT(MultiplexPackets(streamID));

	if (streamID == DVDSCRMSID_VIDEO)
		{
		if (q->frameType == MPFT_IFRAME || q->frameType == MPFT_PFRAME)
			{
			//
			// Calculate the DTS based on the assumption of 2 B-Frames, this should be a value
			// set by a streaming tag.
			//
			extractionTime -= q->frameDuration * (q->temporalReference == 0 ? 1 : 3);

			//
			// Set DTS and PTS only on I-Frames
			//
			if (q->frameType == MPFT_IFRAME)
				{
				q->presentationTimeValid = true;
				q->presentationTime      = presentationTime;
				q->decodingTimeValid     = true;
				q->decodingTime          = extractionTime;
				q->groupStartTimeValid   = true;
				q->groupStartTime        = presentationTime - q->frameDuration * q->temporalReference;

				//[BS]
				// New VOBU is starting hence make the frame Count to be 1
				videoFrameCountInCurrVOBU = 0;
#if DEBUG_BITRATE

				SegmentBitrate += GopSize;

				if (GOPCount%GOPS_IN_SEGMENT == 0)
					{
					SegmentBitrate = SegmentBitrate * 80/(GOPS_IN_SEGMENT * 6); // n gops, each 600 ms

					if (maxSegmentBitrate < SegmentBitrate)
						maxSegmentBitrate = SegmentBitrate;

					if (maxPackCount < packCount)
						maxPackCount = packCount;

					if (GOPCount%(GOPS_IN_SEGMENT*10) == 0)
						{
						DPR("Multiplexer: Max Segment BR = %d Max Pack count = %d\n", maxSegmentBitrate, maxPackCount);
						maxSegmentBitrate = 0;
						maxPackCount = 0;
						}

					SegmentBitrate = 0;
					packCount = 0;
					}

				GopSize = 0;
				GOPCount++;

				if(GOPCount%100 == 0)
					{
					WriteDebugRecording("d:\\BitrateDPR.txt");
					}
#endif
//[BS]
				}
			}
		}
	else if (!q->presentationTimeValid)
		{
		//
		// Set presentation time for all other packets
		//
		q->presentationTimeValid = true;
		q->presentationTime = presentationTime;
		}

	q->extractionTime = extractionTime;

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseBeginConfigure(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseConfigure(uint32 streamID, TAG *& tags)
	{
	uint32			changeSet;
	SCRStream	*	q = streams + streamID;

	STFRES_REASSERT(MultiplexPackets(streamID));

	PARSE_TAGS_START(tags, changeSet)
		GETSETC(MPEG_FRAME_TYPE,					q->frameType, 0);
		GETSETC(MPEG_FRAME_TEMPORAL_REFERENCE, q->temporalReference, 0);
//		GETSETC(IS_FIRST_PACK_OF_SPU, isFirstPackOfSpu, 0);
	PARSE_TAGS_END;

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseCompleteConfigure(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseFrameStart(uint32 streamID)
	{
	SCRStream	*	q = streams + streamID;

	//
	// This marks the start of a new frame, so we should close any still pending
	// access units
	//
	STFRES_REASSERT(CompleteAccessUnit(streamID));

	//
	// Send all full packets
	//
	STFRES_REASSERT(MultiplexPackets(streamID));

	if (streamID == DVDSCRMSID_VIDEO)
		{
		if (q->frameType == MPFT_IFRAME || q->frameType == MPFT_PFRAME)
			{
			//
			// mark this as a starting packet for a reference frame
			//
			q->referenceFrameMask |= MKFLAG(q->referenceFrameCount);
			q->referenceFrameCount++;
			}
		videoFrameCountInCurrVOBU++;
		}
	else if (streamID == DVDSCRMSID_AUDIO)
		{
		//
		// Mark the frame start and the number of frame starts
		// inside this packet
		//
		if (!q->frameCount)
			{
			if (q->range.size)
				q->frameOffset = q->range.size;
			else
				q->frameOffset = q->skipSize;
			}

		q->frameCount++;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseRange(uint32 streamID, const VDRDataRange & range, uint32 & offset)
	{
	SCRStream	*	q = streams + streamID;

#if 0
	if (streamID == DVDSCRMSID_AUDIO)
		{
		uint8	*	p = range.GetStart();
		DPR("qrs %d rs %d ro %d skp %d (%02x %02x)\n", q->range.size, range.size, q->range.offset, q->skipSize, p[0], p[1]);
		}
#endif

	//
	// Check if we are at the end of a packet due to a discontinuity.  If this is the case,
	// we can close the current range by applying the correct amount of stuffing.
	//
	if (q->range.size > 0 && q->range.size < 2048 && (q->range.block != range.block || q->range.size + q->range.offset != range.offset))
		{
		DP("Apply range mismatch stuffing\n");
		FormatPacket(streamID);
		}

	//
	// If we have a complete packet, we can go on an multiplex it
	//
	STFRES_REASSERT(MultiplexPackets(streamID));

	//
	// Adapt packet payload size
	//
	switch (range.type)
		{
		case VDRRID_PACKET_PAYLOAD:
			q->payloadSize       += range.size;
			q->packetPayloadSize += range.size;
			break;
		case VDRRID_PACKET_PADDING:
			q->packetStuffingSize += range.size;
			break;
		case VDRRID_PACKET_HEADER:
			break;
		default:
			if (range.size > q->skipSize)
				{
				q->payloadSize       += range.size - q->skipSize;
				q->packetPayloadSize += range.size - q->skipSize;
				}
		}

#if DEBUG_BITRATE
				GopSize += range.size;
#endif


	//
	// Append or copy range
	//
	if (q->range.size)
		{
		q->range.size += range.size;
		}
	else
		{
		q->range = range;
		q->range.AddRef(this);
		}

	//
	// Check if the range is full
	//
	if (q->range.size == 2048)
		{
		STFRES_REASSERT(FormatPacket(streamID));
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseFlush(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseCommit(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseInterrupted(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}


//
// Currently we ignore most of the incomming nosie from the encoders until
// they send a correctly formatted stream
//
STFResult VirtualDVDSCRMultiplexerUnit::ParseDataDiscontinuity(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseTimeDiscontinuity(uint32 streamID)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseBeginSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification)
	{
	SCRStream	*	q = streams + streamID;

	if (q->openSegment == false)
		STFRES_RAISE(STFRES_OBJECT_FULL);

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseEndSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification)
	{
	SCRStream	*	q = streams + streamID;

	STFRES_REASSERT(MultiplexPackets(streamID));

	q->openSegment = false;

	bool segmentClosedOnAllStreams = true;
	for (uint32 multiplexID = DVDSCRMSID_AUDIO; multiplexID < DVDSCRMSID_TOTAL; multiplexID++)
		segmentClosedOnAllStreams &= !streams[multiplexID].openSegment;
	
	if (segmentClosedOnAllStreams)
		{
		//[BS]
		STFHiPrec64BitTime endTime = streams[DVDSCRMSID_VIDEO].groupStartTime + streams[DVDSCRMSID_VIDEO].frameDuration * videoFrameCountInCurrVOBU;
		STFRES_REASSERT(outputFormatter.PutEndTime(endTime));
		//[BS]
		STFRES_REASSERT(outputFormatter.CompleteGroup(false));
		STFRES_REASSERT(outputFormatter.CompleteSegment(false));
		Reset();
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDSCRMultiplexerUnit::ParseEndTime(uint32 streamID, const STFHiPrec64BitTime & time)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseCutDuration(uint32 streamID, const STFHiPrec32BitDuration & duration)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDSCRMultiplexerUnit::ParseSkipDuration(uint32 streamID, const STFHiPrec32BitDuration & duration)
	{
	STFRES_RAISE_OK;
	}

#if _DEBUG
STFResult VirtualDVDSCRMultiplexerUnit::PrintDebugInfo (uint32 id)
	{
	DEBUGLOG(id, "VirtualDVDSCRMultiplexerUnit\n");	// Some more info can probably be output

	STFRES_RAISE_OK;
	}

STFString VirtualDVDSCRMultiplexerUnit::GetInformation(void)
	{
	return STFString("VirtualDVDSCRMultiplexerUnit");
	}
#endif
