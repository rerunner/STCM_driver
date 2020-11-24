///
/// @brief      Extracts elementary streams from DVD PES streams
///

#include "DVDPESStreamUnpacker.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "Device/Interface/Unit/Video/IMPEGVideoTypes.h"


UNIT_CREATION_FUNCTION(CreateDVDPESStreamUnpackerUnit, DVDPESStreamUnpackerUnit)


STFResult DVDPESStreamUnpackerUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDVDPESStreamUnpackerUnit(this, rangesThreshold));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult DVDPESStreamUnpackerUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_DWORD || createParams[2] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	rangesThreshold = createParams[1];

	STFRES_RAISE_OK;
	}

STFResult DVDPESStreamUnpackerUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
 	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}

STFResult DVDPESStreamUnpackerUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


VirtualDVDPESStreamUnpackerUnit::VirtualDVDPESStreamUnpackerUnit(DVDPESStreamUnpackerUnit * physical, uint32 rangesThreshold)
	: VirtualNonthreadedStandardInOutStreamingUnit(physical, 32), streamQueue(32)
	{
	state = DVDPESS_PARSE_PACKHEADER_0;

	outputFormatter.SetRangesThreshold(rangesThreshold);
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ParseBeginConfigure(void)
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ParseConfigure(TAG *& tags)
	{
	while (tags->id)
		{
		STFRES_REASSERT(outputFormatter.PutTag(*tags));
		tags++;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ParseCompleteConfigure(void)
	{
	STFRES_REASSERT(outputFormatter.CompleteTags());

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ParsePESPacket(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	uint8			value;
	uint8		*	pos;
	uint32		size = ranges[range].size;
	uint32		start, done;

	while (range < num)
		{
		pos = ranges[range].GetStart();
		size = ranges[range].size;
		start = offset;

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
					if (value == 0xba)
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
						streamQueue.AppendRange(VDRSubDataRange(ranges[range], start, packetSize), this);

						start += packetSize;
						offset = start;

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
			streamQueue.AppendRange(VDRSubDataRange(ranges[range], start, size - start), this);
			}

		range++;
		offset = 0;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::AnalyzePESPacket(void)
	{
	uint32	headerLength;
	uint32	payload, done, extra;
	uint8		secondaryID, primaryID;
	uint32	pesPacketOffset;

	// Attention: it's possible that there is a system header upfront video

	pesPacketOffset = 14;	// behind pack header

	if (streamQueue[pesPacketOffset + 3] == SYSTEM_HEADER_START_CODE)	// system header, PES packet behind
		{
		headerLength = (streamQueue[pesPacketOffset + 4] << 8) + streamQueue[pesPacketOffset + 5];

		pesPacketOffset += headerLength + 6;	// plus system header
		}


	primaryID = streamQueue[pesPacketOffset + 3];
	headerLength = (uint32)streamQueue[pesPacketOffset + 8] + 9;

	if (primaryID == PRIVATE_STREAM_1_ID)	// private stream 1
		{
		secondaryID = streamQueue[pesPacketOffset + headerLength] & 0xf8;

		switch (secondaryID)
			{
			case 0x20:
			case 0x28:
			case 0x30:
			case 0x38:
				// subpicture 0x20 - 0x3F
				headerLength += 1;
				break;
			case 0x80:
				// ac3 audio
			case 0x88:
				// dts audio
			case 0x90:
				// sdds audio
				headerLength += 4;
				break;
			case 0xa0:
				// lpcm audio
				//headerLength += 7;
				// comment the upper line to get the LPCM Private data header
				break;
			}
		}
	
	payload = ((uint32)(streamQueue[pesPacketOffset + 4]) << 8) + (uint32)streamQueue[pesPacketOffset + 5] + 6 - headerLength;
	streamQueue.DropBytes(pesPacketOffset + headerLength, done, this);

#if 0
	if ((primaryID & 0xf0) == 0xd0)
		{
		streamQueue.SkipBytes(0, payload, this);
		payload = 0;
		}
#endif

	while ((primaryID & 0xe8) == 0xc0 && payload + 9 < streamQueue.Size())
		{

		// An MPEG Audio Packet, we have to be extra carefull, to get the extension header with it...
		primaryID = streamQueue[payload + 3];
		if ((primaryID & 0xe8) == 0xc0)
			{
			headerLength = (uint32)streamQueue[payload + 8] + 9;
			extra = ((uint32)(streamQueue[payload + 4]) << 8) + (uint32)streamQueue[payload + 5] + 6 - headerLength;
			streamQueue.SkipBytes(payload, headerLength, this);
			payload += extra;
			}
#if 0
		if ((primaryID & 0xf0) == 0xd0)
			{
			streamQueue.SkipBytes(payload - extra, extra, this);
			payload -= extra;
			}
#endif
		}

	streamQueue.LimitBytes(payload, this);

	state = DVDPESS_DELIVER_PACKET;

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::DeliverPESPacket(void)
	{
	switch (state)
		{
		case DVDPESS_DELIVER_PACKET:
			STFRES_REASSERT(outputFormatter.PutFrameStart());
			state = DVDPESS_DELIVER_RANGES;

		case DVDPESS_DELIVER_RANGES:
			STFRES_REASSERT(streamQueue.SendRanges(&outputFormatter, this));
			state = DVDPESS_DELIVER_END_TIME;

		case DVDPESS_DELIVER_END_TIME:
			if (endTimePending)
				{
				STFRES_REASSERT(outputFormatter.PutEndTime(endTime));
				endTimePending = false;
				}
			state = DVDPESS_DELIVER_GROUP_END;

		case DVDPESS_DELIVER_GROUP_END:
			if (groupEndPending)
				{
				STFRES_REASSERT(outputFormatter.CompleteGroup(groupEndNotification));
				groupEndPending = false;
				}
#if 0
			state = DVDPESS_DELIVER_TAGS;

		case DVDPESS_DELIVER_TAGS:
			while (numSentTags < numPendingTags)
				{
				STFRES_REASSERT(outputFormatter.PutTag(pendingTags[numSentTags]));
				numSentTags++;
				}
#endif

			state = DVDPESS_DELIVER_GROUP_START;

		case DVDPESS_DELIVER_GROUP_START:
			if (groupStartPending)
				{
				STFRES_REASSERT(outputFormatter.BeginGroup(groupNumber, groupStartNotification, singleUnitGroup));
				groupStartPending = false;
				}
			state = DVDPESS_DELIVER_START_TIME;
		
		case DVDPESS_DELIVER_START_TIME:
			if (startTimePending)
				{
				STFRES_REASSERT(outputFormatter.PutStartTime(startTime));
				startTimePending = false;
				}
			state = DVDPESS_PARSE_PACKHEADER_0;

		default:
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
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

STFResult VirtualDVDPESStreamUnpackerUnit::ResetParser(void)
	{
	if (state < DVDPESS_ANALYZE_PACKET)
		{
		streamQueue.FlushRanges(this);
		state = DVDPESS_PARSE_PACKHEADER_0;
		}

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ResetParser());
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ResetInputProcessing(void)
	{
	if (state < DVDPESS_ANALYZE_PACKET)
		{
		streamQueue.FlushRanges(this);
		state = DVDPESS_PARSE_PACKHEADER_0;
		}

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ResetParser());
	}

STFResult VirtualDVDPESStreamUnpackerUnit::ProcessFlushing(void)
	{
	streamQueue.FlushRanges(this);
	outputFormatter.Flush();
	state = DVDPESS_PARSE_PACKHEADER_0;

	STFRES_RAISE(VirtualNonthreadedStandardInOutStreamingUnit::ProcessFlushing());
	}

bool VirtualDVDPESStreamUnpackerUnit::InputPending(void)
	{
	return state != DVDPESS_PARSE_PACKHEADER_0;
	}


#if _DEBUG
STFString VirtualDVDPESStreamUnpackerUnit::GetInformation(void)
	{
	return STFString("DVDPESStreamUnpacker ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif
