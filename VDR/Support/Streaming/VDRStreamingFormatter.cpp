///
/// @file       VDR/Support/Streaming/VDRStreamingFormatter.cpp
///
/// @brief      VDR Streaming Formatter Implementation for client side
///
/// @author     Ulrich Sigmund
///
/// @date       2003-02-10 
///
/// @par OWNER: STCM Streaming Architecture Team
///
/// @par SCOPE: PUBLIC Implementation File
///
/// VDR Streaming Formatter Implementation for client side
///
/// &copy:      2003 ST Microelectronics. All Rights Reserved.
///

#include "VDRStreamingFormatter.h"


VDRStreamingFormatter::VDRStreamingFormatter(void)
	{
	unit = NULL;

	packetPending = false;
	packetValid = false;
	groupNumber = 0;
	segmentNumber = 0;
	pendingFrameStart = false;
	numRangeThreshold = VDR_MAX_TAG_DATA_RANGES_PER_PACKET;
	}


VDRStreamingFormatter::~VDRStreamingFormatter(void)
	{
	Disconnect();
	}


STFResult VDRStreamingFormatter::Connect(IVDRStreamingProxyUnit * unit, int connectorID)
	{
	if (this->unit != NULL)
		STFRES_RAISE(STFRES_OBJECT_IN_USE);
	else
		{
		this->unit = unit;
		this->connectorID = connectorID;
		unit->AddRef();

		STFRES_RAISE_OK;
		}
	}


STFResult VDRStreamingFormatter::Disconnect(void)
	{
	if (unit)
		{
		unit->Release();
		unit = NULL;
		}

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingFormatter::UpdatePacket(bool send)
	{
	uint32	accepted;
	//lint --e{613}
	if (packetValid && send)
		{
		packetPending = true;
		}

	if (packetPending)
		{
		STFRES_REASSERT(unit->DeliverDataPackets(connectorID,	&packet, 1, accepted));

		if (accepted == 1)
			{
			packetPending = false;
			packetValid = false;
			}
		else
			STFRES_RAISE(STFRES_OBJECT_FULL);
		}

	if (!packetValid)
		{
		packet.size = sizeof(VDRStreamingDataPacket);
		packet.numRanges = 0;
		packet.numTags = 0;
		packet.flags = 0;
		packet.frameStartFlags = 0;
		packet.groupNumber = groupNumber;
		packet.segmentNumber = segmentNumber;
		
		packetValid = true;
		}

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingFormatter::Flush(void)
	{
	int	i;

	if (packetValid)
		{
		for(i=0; i<packet.numRanges; i++)
			{
			packet.tagRanges.ranges[i + packet.numTags].Release();
			}
		}

	packetPending = false;
	packetValid = false;
	groupNumber = 0;
	segmentNumber = 0;
	pendingFrameStart = false;

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingFormatter::Commit(void)
	{
	uint32	accepted;
	//lint --e{613}
	if (packetValid)
		{
		packetPending = true;
		}

	if (packetPending)
		{
		STFRES_REASSERT(unit->DeliverDataPackets(connectorID,	&packet, 1, accepted));

		if (accepted == 1)
			{
			packetPending = false;
			packetValid = false;
			}
		else
			STFRES_RAISE(STFRES_OBJECT_FULL);
		}

	STFRES_RAISE_OK;
	}


VDRStreamingParser::VDRStreamingParser(void)
	{
	unit = NULL;
	packetState = SPSPS_IDLE;
	inConfigure = false;
	}


VDRStreamingParser::~VDRStreamingParser(void)
	{
	Disconnect();
	}


STFResult VDRStreamingParser::Connect(IVDRStreamingProxyUnit * unit, int connectorID)
	{
	if (this->unit != NULL)
		STFRES_RAISE(STFRES_OBJECT_IN_USE);
	else
		{
		this->unit = unit;
		this->connectorID = connectorID;
		unit->AddRef();

		STFRES_RAISE_OK;
		}
	}


STFResult VDRStreamingParser::Disconnect(void)
	{
	if (unit)
		{
		unit->Release();
		unit = NULL;
		}

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseReceivePacket(void)
	{
	STFAutoMutex	lock(&mutex);
	//lint --e{613}
	uint32	accept;

	for(;;)
		{
		switch (packetState)
			{
			case SPSPS_IDLE:
				accept = 0;

				STFRES_REASSERT(unit->GetDataPackets(connectorID, &packet, 1, accept));
				if (accept == 0)
					{
					unit->RequestDataPackets(connectorID);
					STFRES_RAISE_OK;
					}

				packetState = SPSPS_DATA_DISCONTINUITY;
			case SPSPS_DATA_DISCONTINUITY:
				if (packet.flags & VDR_MSMF_DATA_DISCONTINUITY)
					STFRES_REASSERT(this->ParseDataDiscontinuity());

				packetState = SPSPS_BEGIN_SEGMENT;
			case SPSPS_BEGIN_SEGMENT:
				if (packet.flags & VDR_MSMF_SEGMENT_START)
					STFRES_REASSERT(this->ParseBeginSegment(packet.segmentNumber, packet.flags & VDR_MSCF_SEGMENT_START_NOTIFICATION));

				packetState = SPSPS_BEGIN_CONFIGURE;
			case SPSPS_BEGIN_CONFIGURE:
				if (packet.flags & VDR_MSMF_TAGS_VALID)
					{
					if (!inConfigure)
						{
						STFRES_REASSERT(this->ParseBeginConfigure());
						inConfigure = true;
						}
					}

				if (packet.flags & VDR_MSMF_TAGS_VALID)
					ptags = (TAG *)(packet.tagRanges.tags);
				else
					ptags = NULL;

				packetState = SPSPS_CONFIGURE;
			case SPSPS_CONFIGURE:
				if (ptags)
					STFRES_REASSERT(this->ParseConfigure(ptags));

				packetState = SPSPS_END_CONFIGURE;
			case SPSPS_END_CONFIGURE:
 				if (inConfigure)
					{
 					if (!(packet.flags & VDR_MSMF_TAGS_VALID) || packet.numRanges || (packet.flags & VDR_MSMF_GROUP_START))
						{
						STFRES_REASSERT(this->ParseCompleteConfigure());
						inConfigure = false;
						}
					}

				packetState = SPSPS_BEGIN_GROUP;

			case SPSPS_BEGIN_GROUP:
				if (packet.flags & VDR_MSMF_GROUP_START)
					STFRES_REASSERT(this->ParseBeginGroup(packet.groupNumber, 
																	 packet.flags & VDR_MSCF_GROUP_START_NOTIFICATION,
																	 packet.flags & VDR_MSMF_SINGLE_UNIT_GROUP));

				packetState = SPSPS_START_TIME;
			case SPSPS_START_TIME:
 				if (packet.flags & VDR_MSMF_START_TIME_VALID)
					STFRES_REASSERT(this->ParseStartTime(packet.startTime));

				packetState = SPSPS_SKIP_UNTIL;
			case SPSPS_SKIP_UNTIL:
				if (packet.flags & VDR_MSCF_SKIP_UNTIL)
					STFRES_REASSERT(this->ParseSkipDuration(packet.skipDuration));
				
				packetState = SPSPS_CUT_AFTER;
			case SPSPS_CUT_AFTER:
				if (packet.flags & VDR_MSCF_CUT_AFTER)
					STFRES_REASSERT(this->ParseCutDuration(packet.cutDuration));

				prange = 0;
				poffset = 0;

				packetState = SPSPS_DATA_RANGE;
			case SPSPS_DATA_RANGE:
				if (prange < packet.numRanges)	
					{
					if (packet.frameStartFlags)
						{
						int	fppos;

						//
						// We have to consider the frame start flags, so we have to split the packet
						// into frames.
						//
						while (prange < packet.numRanges)
							{
							//
							// First check if we are on a frame start
							//
							if (poffset == 0 && ((packet.frameStartFlags >> prange) & 1) != 0)
								{
								this->ParseFrameStart();
								}

							fppos = prange + 1;
							while (fppos < packet.numRanges && ((packet.frameStartFlags >> fppos) & 1) == 0)
								fppos++;

							STFRES_REASSERT(this->ParseRanges(packet.tagRanges.ranges + packet.numTags, fppos, prange, poffset));
							}
						}
					else
						STFRES_REASSERT(this->ParseRanges(packet.tagRanges.ranges + packet.numTags, packet.numRanges, prange, poffset));
					}

				packetState = SPSPS_END_TIME;
			case SPSPS_END_TIME:
 				if (packet.flags & VDR_MSMF_END_TIME_VALID)
					STFRES_REASSERT(this->ParseEndTime(packet.endTime));

				packetState = SPSPS_END_GROUP;
			case SPSPS_END_GROUP:
 				if (packet.flags & VDR_MSMF_GROUP_END)
					STFRES_REASSERT(this->ParseEndGroup(packet.groupNumber, packet.flags & VDR_MSCF_GROUP_END_NOTIFICATION));

				packetState = SPSPS_TIME_DISCONTINUITY;

			case SPSPS_TIME_DISCONTINUITY:
				if (packet.flags & VDR_MSMF_TIME_DISCONTINUITY)
					STFRES_REASSERT(this->ParseTimeDiscontinuity());

				packetState = SPSPS_END_SEGMENT;

			case SPSPS_END_SEGMENT:
 				if (packet.flags & VDR_MSMF_SEGMENT_END)
					STFRES_REASSERT(this->ParseEndSegment(packet.segmentNumber, packet.flags & VDR_MSCF_SEGMENT_END_NOTIFICATION));

				STFRES_REASSERT(ReleasePacket());

				packetState = SPSPS_IDLE;
				break;
			}
		}
	}


STFResult VDRStreamingParser::ParseFlush(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseCommit(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseBeginConfigure(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseConfigure(TAG *& tags)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseCompleteConfigure(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseFrameStart(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseDataDiscontinuity(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseTimeDiscontinuity(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseBeginSegment(uint16 segmentNumber, bool sendNotification)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseEndSegment(uint16 segmentNumber, bool sendNotification)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseBeginGroup(uint16 groupNumber, bool sendNotification, bool singleUnitGroup)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseEndGroup(uint16 groupNumber, bool sendNotification)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseStartTime(const STFHiPrec64BitTime & time)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseEndTime(const STFHiPrec64BitTime & time)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseCutDuration(const STFHiPrec32BitDuration & duration)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseSkipDuration(const STFHiPrec32BitDuration & duration)
	{
	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ReleasePacket(void)
	{
	int	i;
	if (packetState != SPSPS_IDLE)
		{
		for(i=0; i<packet.numRanges; i++)
			packet.tagRanges.ranges[i + packet.numTags].Release();

		packetState = SPSPS_IDLE;
		}

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::Flush(void)
	{
	STFRES_REASSERT(ReleasePacket());

	STFRES_REASSERT(ParseFlush());

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::Commit(void)
	{
	STFRES_REASSERT(ParseReceivePacket());

	STFRES_REASSERT(ParseCommit());

	STFRES_RAISE_OK;
	}


STFResult VDRStreamingParser::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	while (range < num)
		{
		STFRES_REASSERT(this->ParseRange(ranges[range], offset));
		range++;
		offset = 0;
		}

	STFRES_RAISE_OK;
	}

