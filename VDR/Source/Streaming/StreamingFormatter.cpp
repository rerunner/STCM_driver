#include "StreamingFormatter.h"
#include <string.h>

STFResult OutputConnectorStreamingFormatter::GetEmptyPacket(StreamingDataPacket * & packet)
	{
	if (output)
		STFRES_RAISE(output->GetEmptyDataPacket(packet));
	else
		STFRES_RAISE(STFRES_OBJECT_INVALID);
	}


STFResult OutputConnectorStreamingFormatter::SendPacket(StreamingDataPacket * packet)
	{
	if (output)
		STFRES_RAISE(output->SendPacket(packet));
	else
		STFRES_RAISE(STFRES_OBJECT_INVALID);
	}


STFResult OutputConnectorStreamingFormatter::FlushPacket(StreamingDataPacket * packet)
	{
	packet->ReleaseRanges();
	packet->ReturnToOrigin();

	STFRES_RAISE_OK;
	}


OutputConnectorStreamingFormatter::OutputConnectorStreamingFormatter(void)
	{
	output = NULL;
	}


OutputConnectorStreamingFormatter::~OutputConnectorStreamingFormatter(void)
	{
	}


STFResult OutputConnectorStreamingFormatter::SetOutputConnector(IStreamingOutputConnector * output)
	{
	this->output = output;

	STFRES_RAISE_OK;
	}

STFResult OutputConnectorStreamingFormatter::CompleteConnection(void)
	{
	//lint --e{613}
	assert(output != NULL);
	STFRES_RAISE(output->GetStreamTagIDs(tagUnitIDs));
	}


StreamingParser::StreamingParser(void)
	{
	ppacket = NULL;
	packetState = SPSPS_IDLE;
	inConfigure = false;
	deferredTags = NULL;
	numDeferredTags = maxDeferredTags = 0;
	}


StreamingParser::~StreamingParser(void)
	{
	}


STFResult StreamingParser::Flush(void)
	{
	ppacket = NULL;
	packetState = SPSPS_IDLE;
	inConfigure = false;

	STFRES_RAISE(this->ParseFlush());
	}


STFResult StreamingParser::Commit(void)
	{
	STFRES_RAISE(this->ParseCommit());
	}

STFResult StreamingParser::ParseDeferredConfigure(void)
	{
	if (packetState < SPSPS_BEGIN_CONFIGURE || packetState >= SPSPS_BEGIN_GROUP && packetState <= SPSPS_TIME_DISCONTINUITY)
		{
		if (numDeferredTags)
			{
			pushedPacketState = packetState;
			packetState = SPSPS_DEFERRED_BEGIN_CONFIGURE;
			numDeferredTags = 0;
			}

		switch (packetState)
			{
			case SPSPS_DEFERRED_BEGIN_CONFIGURE:
				STFRES_REASSERT(this->ParseBeginConfigure());
				ptags = deferredTags;
				
				packetState = SPSPS_DEFERRED_CONFIGURE;
			case SPSPS_DEFERRED_CONFIGURE:
				STFRES_REASSERT(this->ParseConfigure(ptags));
				ptags = NULL;

				packetState = SPSPS_DEFERRED_END_CONFIGURE;
			case SPSPS_DEFERRED_END_CONFIGURE:
				STFRES_REASSERT(this->ParseCompleteConfigure());
				
				packetState = pushedPacketState;

			default:
				// Nothing to do in the other states.
				break;
			}

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_INVALID_CONFIGURE_STATE);
	}

STFResult StreamingParser::EnqueueDeferredTags(TAG * tags)
	{
	int		i, n;
	TAG	*	ttags;

	n = 0;
	ttags = tags;
	//lint --e{613}
	while (ttags->id != VDRTID_DONE)
		{
		n++; ttags += ttags->skip;
		}

	if (n)
		{
		if (numDeferredTags + n + 1 > maxDeferredTags)
			{
			maxDeferredTags = maxDeferredTags * 2 + 16;

			ttags = new TAG[maxDeferredTags];
			if (!ttags)
				STFRES_RAISE(STFRES_OBJECT_FULL);

			for(i=0; i<numDeferredTags; i++)
				ttags[i] = deferredTags[i];
			delete[] deferredTags;
			deferredTags = ttags;
			}

		ttags = tags;
		while (ttags->id != VDRTID_DONE)
			{
			deferredTags[numDeferredTags] = *ttags;
			deferredTags[numDeferredTags].skip = 1;
			numDeferredTags++;
			ttags += ttags->size;
			}

		deferredTags[numDeferredTags] = TAGDONE;
		}

	STFRES_RAISE_OK;
	}

STFResult StreamingParser::ParseReceivePacket(const StreamingDataPacket * packet)
	{
	STFResult	result;

	if (ppacket && packet != ppacket)
		{
		//
		// Should not be here...
		//
		packetState = SPSPS_IDLE;
		inConfigure = false;

		STFRES_REASSERT(ParseInterrupted());
		}

	ppacket = packet;

	while (ppacket)
		{
		switch (packetState)
			{
			case SPSPS_IDLE:
				// Shortcut for pure data packets
				if (packet->vdrPacket.flags == 0 && !inConfigure)
					{
					prange = 0;
					poffset = 0;

					packetState = SPSPS_DATA_RANGE;
					break;
					}

				packetState = SPSPS_DATA_DISCONTINUITY;
			case SPSPS_DATA_DISCONTINUITY:
				if (packet->vdrPacket.flags & VDR_MSMF_DATA_DISCONTINUITY)
					STFRES_REASSERT(this->ParseDataDiscontinuity());

				packetState = SPSPS_BEGIN_SEGMENT;
			case SPSPS_BEGIN_SEGMENT:
				if (packet->vdrPacket.flags & VDR_MSMF_SEGMENT_START)
					STFRES_REASSERT(this->ParseBeginSegment(packet->vdrPacket.segmentNumber, packet->vdrPacket.flags & VDR_MSCF_SEGMENT_START_NOTIFICATION));

				packetState = SPSPS_BEGIN_CONFIGURE;
			case SPSPS_BEGIN_CONFIGURE:
				if (packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID)
					{
					if (numDeferredTags)
						{
						STFRES_REASSERT(this->EnqueueDeferredTags((TAG *)(packet->vdrPacket.tagRanges.tags)));
						packetState = SPSPS_BEGIN_GROUP;
						break;
						}
					if (!inConfigure)
						{
						result = this->ParseBeginConfigure();
						if (result == STFRES_DEFER_STREAM_PARSE_CONFIGURE)
							{
							STFRES_REASSERT(this->EnqueueDeferredTags((TAG *)(packet->vdrPacket.tagRanges.tags)));
							packetState = SPSPS_BEGIN_GROUP;
							break;
							}
						else if (STFRES_FAILED(result))
							STFRES_RAISE(result);

						inConfigure = true;
						}

					ptags = (TAG *)(packet->vdrPacket.tagRanges.tags);
					}
				else
					ptags = NULL;

				packetState = SPSPS_CONFIGURE;
			case SPSPS_CONFIGURE:
				if (ptags)
					{
					STFRES_REASSERT(this->ParseConfigure(ptags));
					ptags = NULL;
					}

				packetState = SPSPS_END_CONFIGURE;
			case SPSPS_END_CONFIGURE:
 				if (inConfigure)
					{
 					if (!(packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID) || packet->vdrPacket.numRanges || (packet->vdrPacket.flags & VDR_MSMF_GROUP_START))
						{
						STFRES_REASSERT(this->ParseCompleteConfigure());
						inConfigure = false;
						}
					}

				packetState = SPSPS_BEGIN_GROUP;

			case SPSPS_BEGIN_GROUP:
				if (packet->vdrPacket.flags & VDR_MSMF_GROUP_START)
					STFRES_REASSERT(this->ParseBeginGroup(packet->vdrPacket.groupNumber, 
																	  packet->vdrPacket.flags & VDR_MSCF_GROUP_START_NOTIFICATION,
																	  packet->vdrPacket.flags & VDR_MSMF_SINGLE_UNIT_GROUP));

				packetState = SPSPS_START_TIME;
			case SPSPS_START_TIME:
 				if (packet->vdrPacket.flags & VDR_MSMF_START_TIME_VALID)
					STFRES_REASSERT(this->ParseStartTime(packet->vdrPacket.startTime));

				packetState = SPSPS_SKIP_UNTIL;
			case SPSPS_SKIP_UNTIL:
				if (packet->vdrPacket.flags & VDR_MSCF_SKIP_UNTIL)
					STFRES_REASSERT(this->ParseSkipDuration(packet->vdrPacket.skipDuration));
				
				packetState = SPSPS_CUT_AFTER;
			case SPSPS_CUT_AFTER:
				if (packet->vdrPacket.flags & VDR_MSCF_CUT_AFTER)
					STFRES_REASSERT(this->ParseCutDuration(packet->vdrPacket.cutDuration));
				prange = 0;
				poffset = 0;

				packetState = SPSPS_DATA_RANGE;
			case SPSPS_DATA_RANGE:
				if (prange < packet->vdrPacket.numRanges)	
					{
					if (packet->vdrPacket.frameStartFlags)
						{
						int	fppos;

						//
						// We have to consider the frame start flags, so we have to split the packet
						// into frames.
						//
						while (prange < packet->vdrPacket.numRanges)
							{
							//
							// First check if we are on a frame start
							//
							if (poffset == 0 && ((packet->vdrPacket.frameStartFlags >> prange) & 1) != 0)
								{
								STFRES_REASSERT(this->ParseFrameStart());
								}

							fppos = prange + 1;
							while (fppos < packet->vdrPacket.numRanges && ((packet->vdrPacket.frameStartFlags >> fppos) & 1) == 0)
								fppos++;

							STFRES_REASSERT(this->ParseRanges(packet->vdrPacket.tagRanges.ranges + packet->vdrPacket.numTags, fppos, prange, poffset));
							}
						}
					else
						STFRES_REASSERT(this->ParseRanges(packet->vdrPacket.tagRanges.ranges + packet->vdrPacket.numTags, packet->vdrPacket.numRanges, prange, poffset));
					}

				// Shortcut for pure data packets
				if (packet->vdrPacket.flags == 0)
					{
					ppacket = NULL;
					packetState = SPSPS_IDLE;
					break;
					}

				packetState = SPSPS_END_TIME;
			case SPSPS_END_TIME:
 				if (packet->vdrPacket.flags & VDR_MSMF_END_TIME_VALID)
					STFRES_REASSERT(this->ParseEndTime(packet->vdrPacket.endTime));

				packetState = SPSPS_END_GROUP;
			case SPSPS_END_GROUP:
 				if (packet->vdrPacket.flags & VDR_MSMF_GROUP_END)
					STFRES_REASSERT(this->ParseEndGroup(packet->vdrPacket.groupNumber, packet->vdrPacket.flags & VDR_MSCF_GROUP_END_NOTIFICATION));

				packetState = SPSPS_TIME_DISCONTINUITY;

			case SPSPS_TIME_DISCONTINUITY:
				if (packet->vdrPacket.flags & VDR_MSMF_TIME_DISCONTINUITY)
					STFRES_REASSERT(this->ParseTimeDiscontinuity());

				packetState = SPSPS_END_SEGMENT;

			case SPSPS_END_SEGMENT:
 				if (packet->vdrPacket.flags & VDR_MSMF_SEGMENT_END)
					STFRES_REASSERT(this->ParseEndSegment(packet->vdrPacket.segmentNumber, packet->vdrPacket.flags & VDR_MSCF_SEGMENT_END_NOTIFICATION));

				ppacket = NULL;
				packetState = SPSPS_IDLE;
				break;

			case SPSPS_DEFERRED_BEGIN_CONFIGURE:
				STFRES_REASSERT(this->ParseBeginConfigure());
				ptags = deferredTags;
				
				packetState = SPSPS_DEFERRED_CONFIGURE;
			case SPSPS_DEFERRED_CONFIGURE:
				STFRES_REASSERT(this->ParseConfigure(ptags));
				ptags = NULL;

				packetState = SPSPS_DEFERRED_END_CONFIGURE;
			case SPSPS_DEFERRED_END_CONFIGURE:
				STFRES_REASSERT(this->ParseCompleteConfigure());
				
				packetState = pushedPacketState;
			}
		}

	STFRES_RAISE_OK;
	}


STFResult StreamingParser::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	while (range < num)
		{
		STFRES_REASSERT(this->ParseRange(ranges[range], offset));
		range++;
		offset = 0;
		}

	STFRES_RAISE_OK;
	}

#if _DEBUG
STFResult StreamingParser::PrintParserState(void)
	{
	DPR("Parser state %d\n", this->packetState);
   STFRES_RAISE_OK;
	}
#endif
//
//Multiple Streaming parser
//

MultipleStreamingParser::MultipleStreamingParser(uint32 numInputs)
	{
	this->numInputs = numInputs;

	pParserState = new ParserState[numInputs];

	if (pParserState)
		memset(pParserState, 0, numInputs * sizeof(ParserState));
	}


MultipleStreamingParser::~MultipleStreamingParser(void)
	{
	if (pParserState) 
		delete[] pParserState;
	}


STFResult MultipleStreamingParser::Flush(uint32 streamID)
	{
	if (pParserState)
		memset(pParserState, 0, numInputs * sizeof(ParserState));

	STFRES_RAISE(this->ParseFlush(streamID));
	}


STFResult MultipleStreamingParser::Commit(uint32 streamID)
	{
	STFRES_RAISE(this->ParseCommit(streamID));
	}


STFResult MultipleStreamingParser::ParseReceivePacket(uint32 streamID, const StreamingDataPacket * packet)
	{
	assert (streamID < numInputs);

	STFRES_RAISE(ParseInternalReceivePacket(streamID, packet));
	}


STFResult MultipleStreamingParser::ParseInternalReceivePacket(uint32 streamID, const StreamingDataPacket * packet)
	{
	//lint --e{613}
	if (pParserState[streamID].ppacket && packet != pParserState[streamID].ppacket)
		{
		//
		// Should not be here...
		//
		pParserState[streamID].packetState = SPSPS_IDLE;
		pParserState[streamID].inConfigure = false;

		STFRES_REASSERT(ParseInterrupted(streamID));
		}

	pParserState[streamID].ppacket = packet;

	switch (pParserState[streamID].packetState)
		{
		case SPSPS_IDLE:

			pParserState[streamID].packetState = SPSPS_DATA_DISCONTINUITY;
		case SPSPS_DATA_DISCONTINUITY:
			if (packet->vdrPacket.flags & VDR_MSMF_DATA_DISCONTINUITY)
				STFRES_REASSERT(this->ParseDataDiscontinuity(streamID));

			pParserState[streamID].packetState = SPSPS_BEGIN_SEGMENT;
		case SPSPS_BEGIN_SEGMENT:
			if (packet->vdrPacket.flags & VDR_MSMF_SEGMENT_START)
				STFRES_REASSERT(this->ParseBeginSegment(streamID, packet->vdrPacket.segmentNumber, packet->vdrPacket.flags & VDR_MSCF_SEGMENT_START_NOTIFICATION));

			pParserState[streamID].packetState = SPSPS_BEGIN_CONFIGURE;
		case SPSPS_BEGIN_CONFIGURE:
			if (packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID)
				{
				if (!pParserState[streamID].inConfigure)
					{
					STFRES_REASSERT(this->ParseBeginConfigure(streamID));
					pParserState[streamID].inConfigure = true;
					}
				}

			if (packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID)
				pParserState[streamID].ptags = (TAG *)(packet->vdrPacket.tagRanges.tags);
			else
				pParserState[streamID].ptags = NULL;

			pParserState[streamID].packetState = SPSPS_CONFIGURE;
		case SPSPS_CONFIGURE:
			if (pParserState[streamID].ptags)
				{
				STFRES_REASSERT(this->ParseConfigure(streamID, pParserState[streamID].ptags));
				pParserState[streamID].ptags = NULL;
				}

			pParserState[streamID].packetState = SPSPS_END_CONFIGURE;
		case SPSPS_END_CONFIGURE:
 			if (pParserState[streamID].inConfigure)
				{
 				if (!(packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID) || packet->vdrPacket.numRanges || (packet->vdrPacket.flags & VDR_MSMF_GROUP_START))
					{
					STFRES_REASSERT(this->ParseCompleteConfigure(streamID));
					pParserState[streamID].inConfigure = false;
					}
				}

			pParserState[streamID].packetState = SPSPS_BEGIN_GROUP;
		case SPSPS_BEGIN_GROUP:
			if (packet->vdrPacket.flags & VDR_MSMF_GROUP_START)
				STFRES_REASSERT(this->ParseBeginGroup(streamID, 
																  packet->vdrPacket.groupNumber, 
																  packet->vdrPacket.flags & VDR_MSCF_GROUP_START_NOTIFICATION,
																  packet->vdrPacket.flags & VDR_MSMF_SINGLE_UNIT_GROUP));

			pParserState[streamID].packetState = SPSPS_START_TIME;
		case SPSPS_START_TIME:
 			if (packet->vdrPacket.flags & VDR_MSMF_START_TIME_VALID)
				STFRES_REASSERT(this->ParseStartTime(streamID, packet->vdrPacket.startTime));

			pParserState[streamID].packetState = SPSPS_SKIP_UNTIL;
		case SPSPS_SKIP_UNTIL:
			if (packet->vdrPacket.flags & VDR_MSCF_SKIP_UNTIL)
				STFRES_REASSERT(this->ParseSkipDuration(streamID, packet->vdrPacket.skipDuration));
			
			pParserState[streamID].packetState = SPSPS_CUT_AFTER;
		case SPSPS_CUT_AFTER:
			if (packet->vdrPacket.flags & VDR_MSCF_CUT_AFTER)
				STFRES_REASSERT(this->ParseCutDuration(streamID, packet->vdrPacket.cutDuration));

			pParserState[streamID].prange = 0;
			pParserState[streamID].poffset = 0;

			pParserState[streamID].packetState = SPSPS_DATA_RANGE;
		case SPSPS_DATA_RANGE:
			if (pParserState[streamID].prange < packet->vdrPacket.numRanges)	
				{
				if (packet->vdrPacket.frameStartFlags)
					{
					int	fppos;

					//
					// We have to consider the frame start flags, so we have to split the packet
					// into frames.
					//
					while (pParserState[streamID].prange < packet->vdrPacket.numRanges)
						{
						//
						// First check if we are on a frame start
						//
						if (pParserState[streamID].poffset == 0 && ((packet->vdrPacket.frameStartFlags >> pParserState[streamID].prange) & 1) != 0)
							{
							STFRES_REASSERT(this->ParseFrameStart(streamID));
							}

						fppos = pParserState[streamID].prange + 1;
						while (fppos < packet->vdrPacket.numRanges && ((packet->vdrPacket.frameStartFlags >> fppos) & 1) == 0)
							fppos++;

						STFRES_REASSERT(this->ParseRanges(streamID, packet->vdrPacket.tagRanges.ranges + packet->vdrPacket.numTags, 
																	 fppos, pParserState[streamID].prange, pParserState[streamID].poffset));
						}
					}
				else
					STFRES_REASSERT(this->ParseRanges(streamID, packet->vdrPacket.tagRanges.ranges + packet->vdrPacket.numTags, 
																 packet->vdrPacket.numRanges, pParserState[streamID].prange, pParserState[streamID].poffset));
				}

			pParserState[streamID].packetState = SPSPS_END_TIME;
		case SPSPS_END_TIME:
 			if (packet->vdrPacket.flags & VDR_MSMF_END_TIME_VALID)
				STFRES_REASSERT(this->ParseEndTime(streamID, packet->vdrPacket.endTime));

			pParserState[streamID].packetState = SPSPS_END_GROUP;
		case SPSPS_END_GROUP:
 			if (packet->vdrPacket.flags & VDR_MSMF_GROUP_END)
				STFRES_REASSERT(this->ParseEndGroup(streamID, packet->vdrPacket.groupNumber, packet->vdrPacket.flags & VDR_MSCF_GROUP_END_NOTIFICATION));

			pParserState[streamID].packetState = SPSPS_END_SEGMENT;
		case SPSPS_END_SEGMENT:
 			if (packet->vdrPacket.flags & VDR_MSMF_SEGMENT_END)
				STFRES_REASSERT(this->ParseEndSegment(streamID, packet->vdrPacket.segmentNumber, packet->vdrPacket.flags & VDR_MSCF_SEGMENT_END_NOTIFICATION));

			pParserState[streamID].ppacket = NULL;
			pParserState[streamID].packetState = SPSPS_TIME_DISCONTINUITY;

		case SPSPS_TIME_DISCONTINUITY:
			if (packet->vdrPacket.flags & VDR_MSMF_TIME_DISCONTINUITY)
				STFRES_REASSERT(this->ParseTimeDiscontinuity(streamID));

			pParserState[streamID].packetState = SPSPS_IDLE;

		case SPSPS_DEFERRED_BEGIN_CONFIGURE:
		case SPSPS_DEFERRED_CONFIGURE:
		case SPSPS_DEFERRED_END_CONFIGURE:
			//??? No deferred tag configuration support yet for the MultipleStreamingParser!
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult MultipleStreamingParser::ParseRanges(uint32 streamID, const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	while (range < num)
		{
		STFRES_REASSERT(this->ParseRange(streamID, ranges[range], offset));
		range++;
		offset = 0;
		}

	STFRES_RAISE_OK;
	}

#if _DEBUG
STFResult MultipleStreamingParser::PrintParserState(void)
	{
	DPR("Multiple Parser state %d\n", this->pParserState->packetState);
   STFRES_RAISE_OK;
	}
#endif

