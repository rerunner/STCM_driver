///
/// @brief      Splitter for DVD stream, extracts packets with a specific stream ID
///

#include "DVDPESSplitter.h"
#include "VDR/Interface/Unit/Datapath/VDRMultiplexedStreamTags.h"
#include "VDR/Interface/Unit/Audio/IVDRAudioUnits.h"
#include "VDR/Interface/Unit/Video/Decoder/IVDRVideoDecoderTypes.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "Device/Interface/Unit/Video/IMPEGVideoTypes.h"


/// The change set group which is used for the streaming-synchronized tag updates
#define VDVDCSG_STREAMID	0
#define VDVDCSG_START_PTM	1
#define VDVDCSG_END_PTM		2

#define NO_NAVPACK_PROCESSING 1

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Splitter Unit
///////////////////////////////////////////////////////////////////////////////

STFResult DVDPESSplitterUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}


STFResult DVDPESSplitterUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
 	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}


STFResult DVDPESSplitterUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Splitter Unit
///////////////////////////////////////////////////////////////////////////////


VirtualDVDPESSplitterUnit::VirtualDVDPESSplitterUnit(DVDPESSplitterUnit * physical)
	: VirtualNonthreadedStandardInOutStreamingUnit(physical, 16), streamQueue(16)
	{
	state = DVDPESS_PARSE_PACKHEADER_0;
	pesConfigState = DVDPESCS_NONE;
	startTimeValid	= false;
	endTimeValid	= false;

	startPTM			= 0;
	firstStartPTM	= 0;
	endPTM			= 0;
	totalPTM			= 0;
	previousStartPTM = 0;
	previousEndPTM = 0;

	firstPacket		= true;
	initialNavPackMissing = true;				
#if NO_NAVPACK_PROCESSING
	navPackRequired = false;
#else
	navPackRequired = true;
#endif
	skipNavPackPTM = false;
	syncChangeSet	= 0;
	}


STFResult VirtualDVDPESSplitterUnit::ParsePESPacket(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	uint8			value;
	uint8		*	pos;
	uint32		size = ranges[range].size;
	uint32		start, done;
	static uint8 packHeader[4] = {0x00, 0x00, 0x01, 0xba};

	while (range < num)
		{
		pos = ranges[range].GetStart();
		size = ranges[range].size;
		start = offset;

		if (state == DVDPESS_PARSE_PACKHEADER_0 && size == 2048 && offset == 0 && ((PADDR)pos & 3) == 0 && *((uint32 *)pos) == *((uint32 *)&packHeader))
			{
			streamQueue.AppendRange(ranges[range], this);
			range++;

			state = DVDPESS_ANALYZE_PACKET;

			STFRES_RAISE_OK;
			}
		else
			{
			while (offset < size)
				{
				value = pos[offset];

				switch (state)
					{
					case DVDPESS_PARSE_PACKHEADER_0:
						if (value == 0x00)
							{
							state = DVDPESS_PARSE_PACKHEADER_1;
							start = offset;
							}
						else
							streamQueue.FlushRanges(this);

						offset++;
						break;

					case DVDPESS_PARSE_PACKHEADER_1:
						if (value == 0x00)
							{
							state = DVDPESS_PARSE_PACKHEADER_2;
							}
						else
							{
							state = DVDPESS_PARSE_PACKHEADER_0;
							streamQueue.FlushRanges(this);
							}
						offset++;
						break;

					case DVDPESS_PARSE_PACKHEADER_2:
						if (value == 0x01)
							{
							state = DVDPESS_PARSE_PACKHEADER_3;
							}
						else if (value == 0x00)
							{
							//
							// we remain in this state during any size of zero group...
							// We just eat byte from the start of the overlap section.
							//
							if (streamQueue.Size())
								{
								//
								// Oops, we already consumed some ranges...
								//
								// Remove the start byte from the first overlap range
								//
								streamQueue.DropBytes(1, done, this);
								}
							else
								{
								//
								// Remove first byte of this first range
								//
								start++;
								}
							}
						else
							{
							state = DVDPESS_PARSE_PACKHEADER_0;
							streamQueue.FlushRanges(this);
							}
						offset++;
						break;

					case DVDPESS_PARSE_PACKHEADER_3:
						if (value == 0xba)	// pack header is mandatory for DVD
							{
							state = DVDPESS_PARSE_PAYLOAD;
							offset++;
							packetSize = 2048 - streamQueue.Size();
							}
						else
							{
							state = DVDPESS_PARSE_PACKHEADER_0;
							streamQueue.FlushRanges(this);
							}
						break;

					case DVDPESS_PARSE_PAYLOAD:
						if (start + packetSize > size)
							{
							offset = size;
							packetSize -= size - start;
							}
						else
							{
							streamQueue.AppendSubRange(ranges[range], start, packetSize, this);

							offset += packetSize;
							if (offset == size)
								{
								range++;
								offset = 0;
								}

							state = DVDPESS_ANALYZE_PACKET;

							STFRES_RAISE_OK;
							}
						break;

					default:
						break;
					}
				}

			if (start < size && state != DVDPESS_PARSE_PACKHEADER_0)
				{
				streamQueue.AppendSubRange(ranges[range], start, size - start, this);
				}
			}

		range++;
		offset = 0;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESSplitterUnit::ParsePESPTS(uint32 offset, uint32 & pts)
	{
 	if (streamQueue[offset + 7] & 0x80)	// check for PTS flag
		{
		pts = ((uint32)((streamQueue[offset +  9] >> 1) & 0x07) << 30) |
		      ((uint32)( streamQueue[offset + 10]             ) << 22) |
		      ((uint32)((streamQueue[offset + 11] >> 1) & 0x7f) << 15) |
		      ((uint32)( streamQueue[offset + 12]             ) <<  7) |
		      ((uint32)((streamQueue[offset + 13] >> 1) & 0x7f)      );

		STFRES_RAISE_TRUE;
		}
	else
		STFRES_RAISE_FALSE;
	}

STFResult VirtualDVDPESSplitterUnit::AnalyzePESPacket(void)
	{
	uint8		id;
	uint16	headerLength;
	uint8		subStreamId;
	uint32	startPTS;
	uint32	newStartPTM;
	uint32	pesPacketOffset;

	pesPacketOffset = 14;	// behind pack header

	id = streamQueue[pesPacketOffset + 3];

	if (id == SYSTEM_HEADER_START_CODE)	// may be followed by video or private stream 2 nav info
		{
		headerLength = (streamQueue[pesPacketOffset + 4] << 8) + streamQueue[pesPacketOffset + 5];

		pesPacketOffset += headerLength + 6;	// plus system header

		// now check following PES packet header
		if (streamQueue[pesPacketOffset    ] == 0x00  &&
		    streamQueue[pesPacketOffset + 1] == 0x00  &&
		    streamQueue[pesPacketOffset + 2] == 0x01)
			{
			// start code pattern found
			id = streamQueue[pesPacketOffset + 3];

			if (id == PRIVATE_STREAM_2_ID)
				{
				//
				// Nav Pack....
				//
				if (!skipNavPackPTM)
					{
					subStreamId = streamQueue[pesPacketOffset + 6];
					// PCI data (0x00) or RDI data (0x50) ?
					if (subStreamId == 0x00)	// sub stream id = PCI data
						{
						if (startTimePending)
							{
							totalPTM = 0;
							startTimePending = false;
							}
						else
							totalPTM += endPTM - startPTM;

						startPTM = ((uint32)(streamQueue[57]) << 24) |
									  ((uint32)(streamQueue[58]) << 16) |
									  ((uint32)(streamQueue[59]) <<  8) |
									  ((uint32)(streamQueue[60])      );

						endPTM   = ((uint32)(streamQueue[61]) << 24) |
									  ((uint32)(streamQueue[62]) << 16) |
									  ((uint32)(streamQueue[63]) <<  8) |
									  ((uint32)(streamQueue[64])      );
						}
					else if (subStreamId == 0x50)	// sub stream id = RDI data
						{
						// there is no endPTM in RDI pack, the newStartPTM is the end of the previous
						newStartPTM = ((uint32)(streamQueue[47]) << 24) |
										  ((uint32)(streamQueue[48]) << 16) |
										  ((uint32)(streamQueue[49]) <<  8) |
										  ((uint32)(streamQueue[50])      );

						if (startTimePending)
							{
							totalPTM = 0;
							startTimePending = false;
							}
						else
							totalPTM += newStartPTM - startPTM;

						startPTM = newStartPTM;
						}
					else
						DP("! unexpected sub stream id 0x%02X behind private stream 2 header\n", subStreamId);
					}

				if (initialNavPackMissing)
					firstStartPTM = startPTM;

				initialNavPackMissing = false;
				}
			else
				{
				if (id != 0xe0) // video
					DP("! unexpected start code identifier 0x%02X behind system header\n", id);
				}
			}
		else
			DP("! No proper start code behind system header\n");
		}

	if ( (!initialNavPackMissing || !navPackRequired) && (id == streamIDs[0] || id == streamIDs[1]))
		{
		// in case check for substream ID (first byte after PES header)
		if (streamIDs[2] == 0xff || streamQueue[pesPacketOffset + 9 + streamQueue[pesPacketOffset + 8]] == streamIDs[2])
			{
			//
			// Its our packet, we'll keep it :-)
			//
			if (STFRES_TRUE == ParsePESPTS(pesPacketOffset, startPTS))
				{
//				DPR("PESS %02x %d %9d : SPTM %08d EPTM %08d TPTM %08d PTS %08d\n", id, firstPacket, startTime.Get32BitTime(), startPTM, endPTM, totalPTM, startPTS);

				if (firstPacket && startPTS < firstStartPTM)
					{
					streamQueue.FlushRanges(this);
					outputStartTime = startTime + STFHiPrec64BitDuration(totalPTM, STFTU_90KHZTICKS);
					startTimeValid = true;
					}
				else
					{
					firstPacket = false;
					outputStartTime = startTime + STFHiPrec64BitDuration((int32)(totalPTM + startPTS - startPTM), STFTU_90KHZTICKS);
					startTimeValid = true;
					}
				}
			else if (firstPacket)
				{
				//
				// No time info yet, don't use the packet
				//
				streamQueue.FlushRanges(this);
				}

			//
			// Some time manipulation might be performed...
			//
			}
		else
			{
			streamQueue.FlushRanges(this);
			}
		}
	else
		{
		streamQueue.FlushRanges(this);
		}

	state = DVDPESS_DELIVER_START_TIME;

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::DeliverPESPacket(void)
	{
	switch (state)
		{
		case DVDPESS_DELIVER_START_TIME:
			if (startTimeValid)
				{
				STFRES_REASSERT(outputFormatter.PutStartTime(outputStartTime));
				startTimeValid = false;
				}
			state = DVDPESS_DELIVER_FRAME_START;

		case DVDPESS_DELIVER_FRAME_START:
			STFRES_REASSERT(outputFormatter.PutFrameStart());
			state = DVDPESS_DELIVER_RANGES;
		
		case DVDPESS_DELIVER_RANGES:
			STFRES_REASSERT(streamQueue.SendRanges(&outputFormatter, this));
			state = DVDPESS_DELIVER_END_TIME;

		case DVDPESS_DELIVER_END_TIME:
			if (endTimeValid)
				{
				STFRES_REASSERT(outputFormatter.PutEndTime(outputEndTime));
				endTimeValid = false;
				}
			state = DVDPESS_DELIVER_GROUP_END;

		case DVDPESS_DELIVER_GROUP_END:
			if (groupEndPending)
				{
				STFRES_REASSERT(outputFormatter.CompleteGroup(groupEndNotification));
				groupEndPending = false;
				}

			state = DVDPESS_DELIVER_TAGS;

		case DVDPESS_DELIVER_TAGS:
			while (numSentTags < numPendingTags)
				{
				STFRES_REASSERT(outputFormatter.PutTag(pendingTags[numSentTags]));
				numSentTags++;
				}

			state = DVDPESS_DELIVER_GROUP_START;

		case DVDPESS_DELIVER_GROUP_START:
			if (groupStartPending)
				{
				STFRES_REASSERT(outputFormatter.BeginGroup(groupNumber, groupStartNotification, singleUnitGroup));
				groupStartPending = false;
				}

			state = DVDPESS_PARSE_PACKHEADER_0;

		default:
			break;
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ProcessStartEndPTM(void)
	{
	if (changeSet & MKFLAG(VDVDCSG_END_PTM))
		{
		// we get start and end PTM

		// add time of previous group to totalPTM
		totalPTM += previousEndPTM - previousStartPTM;
		previousEndPTM = endPTM;
		}
	else
		{
		// we get only start PTM

		// add time of previous group to totalPTM
		totalPTM += startPTM - previousStartPTM;
		}

	previousStartPTM = startPTM;
	skipNavPackPTM = true;	// we received start/end PTM already

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	STFRES_REASSERT(this->DeliverPESPacket());

	while (range < num)
		{
		// Only call ParseFrame if we are in the parsing state.

		if (state < DVDPESS_ANALYZE_PACKET)
			STFRES_REASSERT(this->ParsePESPacket(ranges, num, range, offset));

		if (state == DVDPESS_ANALYZE_PACKET)
			STFRES_REASSERT(this->AnalyzePESPacket());

		STFRES_REASSERT(this->DeliverPESPacket());
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ParseStartTime(const STFHiPrec64BitTime & time)
	{
	startTimePending = true;
	startTime = time;

	// we got a new Display Time from Navigation -> we reset totalPTM
	totalPTM = 0;

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ParseEndTime(const STFHiPrec64BitTime & time)
	{
	endTimePending = true;
	endTime = time;

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ResetInputProcessing(void)
	{
	if (state < DVDPESS_ANALYZE_PACKET)
		{
		streamQueue.FlushRanges(this);
		state = DVDPESS_PARSE_PACKHEADER_0;
		}

	pesConfigState = DVDPESCS_NONE;

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ResetParser());
	}


STFResult VirtualDVDPESSplitterUnit::ResetParser(void)
	{
	if (state < DVDPESS_ANALYZE_PACKET)
		{
		streamQueue.FlushRanges(this);
		state = DVDPESS_PARSE_PACKHEADER_0;
		}

	pesConfigState = DVDPESCS_NONE;

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ResetParser());
	}


STFResult VirtualDVDPESSplitterUnit::ProcessFlushing(void)
	{
	streamQueue.FlushRanges(this);
	outputFormatter.Flush();
	state = DVDPESS_PARSE_PACKHEADER_0;
	startTimeValid = false;
	endTimeValid = false;
	endPTM = 0;
	startPTM = 0;
	firstStartPTM = 0;
	endPTM = 0;
	totalPTM = 0;
	previousStartPTM = 0;
	previousEndPTM = 0;
	firstPacket = true;
	initialNavPackMissing = true;
	skipNavPackPTM = false;

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ProcessFlushing());
	}


bool VirtualDVDPESSplitterUnit::InputPending(void)
	{
	return state != DVDPESS_PARSE_PACKHEADER_0 && state < DVDPESS_ANALYZE_PACKET;
	}


uint32 VirtualDVDPESSplitterUnit::GetSynchronousChangeFlags(void)
	{
	// Convert the change set group to a mask
	return (1 << VDVDCSG_STREAMID);
	}


STFResult VirtualDVDPESSplitterUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	//
	// If it is a packet request, we might be able to satisfy it immediately
	//
	switch (message)
		{
		case VDRMID_STRM_PACKET_REQUEST:
		case VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE:
			// Process a pending packet, this may lead to further packet request upstream notifications
			this->ProcessPendingPacket();

#if ENABLE_PACKET_BOMBING
			message = VDRMID_STRM_PACKET_REQUEST;
#else
			STFRES_RAISE_OK;
#endif
			break;

		case VDRMID_STRM_GROUP_END:
//			DP("Splitter: received group %d end notification from connector %d\n", param1, connectorID);
			break;

		case VDRMID_STRM_SEGMENT_END:
			// display ends presentation of the current segment
//			DP("Splitter: received segment end notification\n");
			break;
		}

	//
	// Forward the request to the upstream filter via the input connector
	//
	STFRES_RAISE(inputConnector->SendUpstreamNotification(message, param1, param2));
	}


// Process synchronized tags at the beginning of each group
STFResult VirtualDVDPESSplitterUnit::ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	skipNavPackPTM = false;

	// Check if stream tags must be sent
	STFRES_REASSERT(ProcessConfigState());

	// Must always call our ancestor to properly forward the group start
	STFRES_REASSERT(VirtualNonthreadedStandardInOutStreamingUnit::ParseBeginGroup(groupNumber, requestNotification, singleUnitGroup));

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESSplitterUnit::ParseBeginSegment(uint16 segmentNumber, bool sendNotification)
	{
	// Check if stream tags must be sent
	STFRES_REASSERT(ProcessConfigState());

	// Must always call our ancestor to properly forward the segment start
	STFRES_REASSERT(VirtualNonthreadedStandardInOutStreamingUnit::ParseBeginSegment(segmentNumber, sendNotification));

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ProcessConfigState(void)
	{
	if (!InputPending())
		{
		if (streamingStreamTypeID != currentStreamTypeID)
			{
			currentStreamTypeID = configStreamTypeID = streamingStreamTypeID;
			pesConfigState = DVDPESCS_TIME_DISCONTINUITY;
			}
		else if (configStreamTypeID != currentStreamTypeID)
			{
			streamingStreamTypeID = currentStreamTypeID = configStreamTypeID;
			pesConfigState = DVDPESCS_TIME_DISCONTINUITY;
			}
		}

	while (pesConfigState != DVDPESCS_NONE)
		{
		switch (pesConfigState)
			{
			case DVDPESCS_TIME_DISCONTINUITY:
				STFRES_REASSERT(outputFormatter.PutTimeDiscontinuity());

				pesConfigState = DVDPESCS_DATA_DISCONTINUITY;
			case DVDPESCS_DATA_DISCONTINUITY:
				STFRES_REASSERT(outputFormatter.PutDataDiscontinuity());

				pesConfigState = DVDPESCS_TAGS;
			case DVDPESCS_TAGS:
				if (changeSet & MKFLAG(VDVDCSG_START_PTM))
					STFRES_REASSERT(ProcessStartEndPTM());

				if (changeSet & MKFLAG(VDVDCSG_STREAMID))
					{
					STFRES_REASSERT(InternalUpdateStreamID());
					}

				changeSet = 0;

				pesConfigState = DVDPESCS_TAGS_DONE;
			case DVDPESCS_TAGS_DONE:
				STFRES_REASSERT(outputFormatter.CompleteTags());

				pesConfigState = DVDPESCS_NONE;

			default:
				break;
			}
		}
	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSplitterUnit::ParseBeginConfigure(void)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESSplitterUnit::ParseCompleteConfigure(void)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESSplitterUnit::InternalConfigureTags(TAG * tags)
	{
	uint32	streamTypeID = configStreamTypeID;

	STFRES_REASSERT(InternalParseStreamID(tags, streamTypeID));

	configStreamTypeID = streamTypeID;

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESSplitterUnit::ParseConfigure(TAG *& tags)
	{
	// first parse general stream tags
	PARSE_TAGS_START(tags, changeSet)
		GETSETC(START_PTM,	startPTM,	VDVDCSG_START_PTM);
		GETSETC(END_PTM,		endPTM,		VDVDCSG_END_PTM);
	PARSE_TAGS_END;

	STFRES_RAISE(InternalParseStreamID(tags, streamingStreamTypeID));
	}

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Splitter Gate Unit
///////////////////////////////////////////////////////////////////////////////

VirtualDVDPESSplitterGateUnit::VirtualDVDPESSplitterGateUnit(DVDPESSplitterUnit * physical)
	: VirtualDVDPESSplitterUnit(physical),
	  inSingleUnitGroup(false)
	{
	}

STFResult VirtualDVDPESSplitterGateUnit::ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	if (singleUnitGroup)
		{
		// Entering into single unit group.
		this->inSingleUnitGroup = true;
		if (requestNotification)
			// Reply for requested notification.
			// presentation time is truncated in MILLISEC unit.
			STFRES_REASSERT(inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_START, groupNumber, 0));
		STFRES_RAISE_OK;
		}

	// Not in single unit group.  Call ancestor's implementation.
	this->inSingleUnitGroup = false;
	STFRES_RAISE(VirtualDVDPESSplitterUnit::ParseBeginGroup(groupNumber, requestNotification, singleUnitGroup));
	}

STFResult VirtualDVDPESSplitterGateUnit::ParseEndGroup(uint16 groupNumber, bool requestNotification)
	{
	if (this->inSingleUnitGroup)
		{
		if (requestNotification)
			// Reply for requested notification.
			// presentation time is truncated in MILLISEC unit.
			STFRES_REASSERT(inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_END, groupNumber, 0));
		STFRES_RAISE_OK;
		}
	// Not in single unit group.  Call ancestor's implementation.
	STFRES_RAISE(VirtualDVDPESSplitterUnit::ParseEndGroup(groupNumber, requestNotification));
	}

STFResult VirtualDVDPESSplitterGateUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	if (this->inSingleUnitGroup)
		{
		// We don't forward data range that is unnecessary in single unit group.
		STFRES_RAISE_OK;
		}

	// Not in single unit group.  Call ancestor's implementation.
	STFRES_RAISE(VirtualDVDPESSplitterUnit::ParseRanges(ranges, num, range, offset));
	}

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Audio Splitter Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateDVDPESAudioSplitterUnit, DVDPESAudioSplitterUnit)


STFResult DVDPESAudioSplitterUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDVDPESAudioSplitterUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult DVDPESAudioSplitterUnit::GetTagIDs(VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_AUDIO_GENERAL_MODE_PROPERTY,
		VDRTID_AUDIO_STREAM_PROPERTY,
		VDRTID_DONE
		};

	ids = (VDRTID *)supportedTagTypes;

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Audio Splitter Unit
///////////////////////////////////////////////////////////////////////////////

VirtualDVDPESAudioSplitterUnit::VirtualDVDPESAudioSplitterUnit(DVDPESSplitterUnit * physical)
	: VirtualDVDPESSplitterGateUnit(physical)
	{
	currentStreamTypeID = streamingStreamTypeID = configStreamTypeID = ((uint32)VDR_AUDIOTYPE_UNDEFINED << 16);

	streamIDs[0]	= PRIVATE_STREAM_1_ID;	// Private stream
	streamIDs[1]	= 0xff;	// Do not check for an extension stream
	streamIDs[2]	= 0x80;	// Take AC3 stream 0 by default

	// Comment the line below to set the default stream as AC3
	//streamIDs[2]	= 0xa0;	// Take LPCM stream 0 by default

	// streamID			= 0;
	// streamType		= VDR_AUDIOTYPE_AC3;
	// Comment the line below to set the default stream as AC3
	//streamType		= VDR_AUDIOTYPE_LPCM;
	
	outputFormatter.SetRangesThreshold(1);
	}


STFResult VirtualDVDPESAudioSplitterUnit::InternalParseStreamID(TAG * tags, uint32 & streamTypeID)
	{
	uint32					streamID   = (streamTypeID & 0x07);
	VDRAudioStreamType	streamType = (VDRAudioStreamType)(streamTypeID >> 16);

	PARSE_TAGS_START(tags, changeSet)
		GETSETC(AUDIO_STREAM_TYPE,	streamType,	VDVDCSG_STREAMID);
		GETSETC(AUDIO_STREAM_ID,	streamID,	VDVDCSG_STREAMID);
	PARSE_TAGS_END;

	streamTypeID = (streamID & 7) | ((uint32)streamType << 16);

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESAudioSplitterUnit::InternalUpdateStreamID(void)
	{
	uint32					streamID;
	VDRAudioStreamType	streamType;

	// Select streamIDs[0] according to streamType
	streamID = currentStreamTypeID & 7;
	streamType = (VDRAudioStreamType)(currentStreamTypeID >> 16);

	switch (streamType)
		{
		case VDR_AUDIOTYPE_MPEG:
			streamIDs[0] =	0xc0 | (streamID & 0x7);	// stream ID
			streamIDs[1] = 0xd0 | (streamID & 0x7);	// extension stream ID
			streamIDs[2] = 0xff;								// No private stream to check
			break;

		case VDR_AUDIOTYPE_AC3:
			streamIDs[0] = PRIVATE_STREAM_1_ID;								// Private stream 1
			streamIDs[1] = 0xff;								// No extension stream to check
			streamIDs[2] = 0x80 | (streamID & 0x7);	//
			break;

		case VDR_AUDIOTYPE_LPCM:
		case VDR_AUDIOTYPE_LPCM_DVDA:
		case VDR_AUDIOTYPE_MLP:
			streamIDs[0] = PRIVATE_STREAM_1_ID;								// Private stream 1
			streamIDs[1] = 0xff;								// No extension stream to check
			streamIDs[2] = 0xa0 | (streamID & 0x7);
			break;

		case VDR_AUDIOTYPE_DTS:
			streamIDs[0] = PRIVATE_STREAM_1_ID;								// Private stream 1
			streamIDs[1] = 0xff;								// No extension stream to check
			streamIDs[2] = 0x88 | (streamID & 0x7);
			break;

		case VDR_AUDIOTYPE_SDDS:
			streamIDs[0] = PRIVATE_STREAM_1_ID;								// Private stream 1
			streamIDs[1] = 0xff;								// No extension stream to check
			streamIDs[2] = 0x90 | (streamID & 0x7);
			break;

		default:
			STFRES_RAISE(STFRES_UNSUPPORTED_AUDIO_STREAM_TYPE);
			break;
		}

	STFRES_REASSERT(outputFormatter.PutTag(SET_AUDIO_STREAM_TYPE(streamType)));

	STFRES_RAISE_OK;
	}

//
// Need a special implementation here due to the possibility of MPEG 2  multichannel audio
// having the PTS hidden deep inside the DVD packet
//
STFResult VirtualDVDPESAudioSplitterUnit::ParsePESPTS(uint32 offset, uint32 & pts)
	{
	uint32	size;

	if ((VDRAudioStreamType)(currentStreamTypeID >> 16) == VDR_AUDIOTYPE_MPEG)
		{
		while ((streamQueue[offset + 7] & 0x80) == 0 || (streamQueue[offset + 3] & 0xf8) == 0xd0)
			{
			size = (streamQueue[offset + 4] << 8) + streamQueue[offset + 5];
			offset += size + 6;
			if (offset >= 2048 - 13 || (streamQueue[offset + 3] & 0xe8) != 0xc0)
				STFRES_RAISE_FALSE;
			}
		}

 	if (streamQueue[offset + 7] & 0x80)	// check for PTS flag
		{
		pts = ((uint32)((streamQueue[offset +  9] >> 1) & 0x07) << 30) |
			   ((uint32)( streamQueue[offset + 10]			      ) << 22) |
				((uint32)((streamQueue[offset + 11] >> 1) & 0x7f) << 15) |
				((uint32)( streamQueue[offset + 12]             ) <<  7) |
				((uint32)((streamQueue[offset + 13] >> 1) & 0x7f)      );

		STFRES_RAISE_TRUE;
		}
	else
		STFRES_RAISE_FALSE;
	}


#if _DEBUG
STFString VirtualDVDPESAudioSplitterUnit::GetInformation(void)
	{
	return STFString("DVDPESAudioSplitter ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif



///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Video Splitter Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateDVDPESVideoSplitterUnit, DVDPESVideoSplitterUnit)

STFResult DVDPESVideoSplitterUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDVDPESVideoSplitterUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult DVDPESVideoSplitterUnit::GetTagIDs(VDRTID * & ids)
	{
	ids = NULL;
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Video Splitter Unit
///////////////////////////////////////////////////////////////////////////////


VirtualDVDPESVideoSplitterUnit::VirtualDVDPESVideoSplitterUnit(DVDPESSplitterUnit * physical)
	: VirtualDVDPESSplitterUnit(physical)
	{
	currentStreamTypeID = configStreamTypeID = streamingStreamTypeID = 0;
	streamIDs[0] = 0xe0;
	streamIDs[1] = 0xff;	// Do not check for an extension stream
	streamIDs[2] = 0xff;	// No private stream ID to be checked
	}

STFResult VirtualDVDPESVideoSplitterUnit::ParseStartTime(const STFHiPrec64BitTime & time)
	{
	STFRES_REASSERT(VirtualDVDPESSplitterUnit::ParseStartTime(time));
	// We initialize outputStartTime with the input group start time.
	// If the group contains frames with a start time it will be overridden,
	// else - in case of nav pack group - the group start time will be sent.
	outputStartTime = time;
	startTimeValid = true;

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESVideoSplitterUnit::ParseEndTime(const STFHiPrec64BitTime & time)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.PutEndTime(time));
		}
	else
		{
		STFRES_REASSERT(VirtualDVDPESSplitterUnit::ParseEndTime(time));
		outputEndTime = time;
		endTimeValid = true;
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESVideoSplitterUnit::InternalParseStreamID(TAG * tags, uint32 & streamTypeID)
	{
	PARSE_TAGS_START(tags, changeSet)
		GETSETC(MPEG_VIDEO_STREAM_ID,	streamTypeID, VDVDCSG_STREAMID);
	PARSE_TAGS_END;

	// The update will be handled in StandardStreamingUnit::InternalUpdate().
	// The change set group 0 will be synchronized with the streaming processing.
	// See VirtualDVDPESSplitterUnit::GetSynchronousChangeFlags(void) above.

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESVideoSplitterUnit::InternalUpdateStreamID(void)
	{
	streamIDs[0] = 0xe0 | (currentStreamTypeID & 0x07);	// stream ID

	STFRES_RAISE_OK;
	}


#if _DEBUG
STFString VirtualDVDPESVideoSplitterUnit::GetInformation(void)
	{
	return STFString("DVDPESVideoSplitter ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif


///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Subpicture Splitter Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateDVDPESSubpictureSplitterUnit, DVDPESSubpictureSplitterUnit)


STFResult DVDPESSubpictureSplitterUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDVDPESSubpictureSplitterUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult DVDPESSubpictureSplitterUnit::GetTagIDs(VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_SUBPICTURE_DECODER_PROPERTIES,
		VDRTID_DONE
		};

	ids = (VDRTID *)supportedTagTypes;

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Subpicture Splitter Unit
///////////////////////////////////////////////////////////////////////////////

VirtualDVDPESSubpictureSplitterUnit::VirtualDVDPESSubpictureSplitterUnit(DVDPESSplitterUnit * physical)
	: VirtualDVDPESSplitterGateUnit(physical)
	{
	currentStreamTypeID = configStreamTypeID = streamingStreamTypeID = 0;
	streamIDs[0] = PRIVATE_STREAM_1_ID;	// Check for private stream
	streamIDs[1] = 0xff;	// Do not check for an extension stream
	streamIDs[2] = 0x20;	// Check for subpicture stream 0 by default

	outputFormatter.SetRangesThreshold(1);
	}

STFResult VirtualDVDPESSubpictureSplitterUnit::InternalParseStreamID(TAG * tags, uint32 & streamTypeID)
	{
	PARSE_TAGS_START(tags, changeSet)
		GETSETC(SUBPICTURE_STREAM_ID,	streamTypeID, VDVDCSG_STREAMID);
	PARSE_TAGS_END;

	// The update will be handled in StandardStreamingUnit::InternalUpdate().
	// The change set group 0 will be synchronized with the streaming processing.
	// See VirtualDVDPESSplitterUnit::GetSynchronousChangeFlags(void) above.

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDPESSubpictureSplitterUnit::InternalUpdateStreamID(void)
	{
	streamIDs[0] = PRIVATE_STREAM_1_ID;	// Check for private stream
	streamIDs[1] = 0xff;	// Do not check for an extension stream
	streamIDs[2] = 0x20 | (currentStreamTypeID & 0x1f);	// Private stream ID to check

	// Flush all the subpicture streaming data and remove current frame.
	STFRES_REASSERT(outputFormatter.Flush());
	STFRES_REASSERT(outputFormatter.PutTag(SET_SUBPICTURE_STREAM_ID(streamIDs[2])));
	STFRES_REASSERT(outputFormatter.CompleteTags());
	STFRES_REASSERT(outputFormatter.Commit());
	STFRES_RAISE_OK;
	}

#if _DEBUG
STFString VirtualDVDPESSubpictureSplitterUnit::GetInformation(void)
	{
	return STFString("DVDPESSubpictureSplitter ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif
