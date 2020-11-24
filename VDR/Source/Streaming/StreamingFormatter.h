///
/// @brief      VDR Streaming Formatter Implementation
///

#ifndef STREAMING_FORMATTER_H
#define STREAMING_FORMATTER_H

#include "IStreaming.h"
#include "STF/Interface/STFDataManipulationMacros.h"
#include <assert.h>

//! Returned by StreamingParser::BeginConfigure, if the configuration should be deferred until a later point in the parsing.
static const STFResult STFRES_DEFER_STREAM_PARSE_CONFIGURE = 0x8884403e;


class StreamingFormatter
	{
	protected:
		virtual STFResult GetEmptyPacket(StreamingDataPacket * & packet) = 0;
		virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;
		virtual STFResult FlushPacket(StreamingDataPacket * packet) = 0;

		STFResult UpdatePacket(bool send);

		bool								pendingFrameStart;
		uint16							groupNumber, segmentNumber;

		uint8								numRangeThreshold;
		VDRTID						*	tagUnitIDs;

		StreamingDataPacket		*	packet;
	public:
		StreamingFormatter(void);
		virtual ~StreamingFormatter(void);

		void SetRangesThreshold(uint8 threshold)
			{
			this->numRangeThreshold = threshold;
			}

		//
		// Streaming control
		//
		STFResult Flush(void);
		STFResult Commit(void);
		STFResult LowLatencyCommit(void);

		//
		// Tag list formatting
		//
		STFResult PutTag(const TAGITEM & tag);
		STFResult CompleteTags(void);

		//
		// Data range formatting
		//
		STFResult PutRange(const VDRDataRange & range);

		//
		// Range information formatting
		//
		STFResult PutFrameStart(void);
		STFResult PutDataDiscontinuity(void);
		STFResult PutTimeDiscontinuity(void);
		STFResult BeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup);
		STFResult CompleteGroup(bool requestNotification);
		STFResult BeginSegment(uint16 segmentNumber, bool requestNotification);
		STFResult CompleteSegment(bool requestNotificatione);

		//
		// Time information
		//
		STFResult PutStartTime(const STFHiPrec64BitTime & time);
		STFResult PutEndTime(const STFHiPrec64BitTime & time);
		STFResult PutCutDuration(const STFHiPrec32BitDuration & duration);
		STFResult PutSkipDuration(const STFHiPrec32BitDuration & duration);
	};

class OutputConnectorStreamingFormatter : public StreamingFormatter
	{
	protected:
		IStreamingOutputConnector	*	output;

		STFResult GetEmptyPacket(StreamingDataPacket * & packet);
		STFResult SendPacket(StreamingDataPacket * packet);
		STFResult FlushPacket(StreamingDataPacket * packet);
	public:
		OutputConnectorStreamingFormatter(void);
		virtual ~OutputConnectorStreamingFormatter(void);

		//
		// Will _not_ AddRef output connector, assumes liveness of connection
		// during livetime of filter
		//
		STFResult SetOutputConnector(IStreamingOutputConnector * output);
		STFResult CompleteConnection(void);
	};


enum StreamingPacketState
	{
	SPSPS_IDLE,
	SPSPS_DATA_DISCONTINUITY,
	SPSPS_BEGIN_SEGMENT,
	SPSPS_BEGIN_CONFIGURE,
	SPSPS_CONFIGURE,
	SPSPS_END_CONFIGURE,
	SPSPS_BEGIN_GROUP,
	SPSPS_START_TIME,
	SPSPS_SKIP_UNTIL,
	SPSPS_CUT_AFTER,
	SPSPS_DATA_RANGE,
	SPSPS_END_TIME,
	SPSPS_END_GROUP,
	SPSPS_END_SEGMENT,
	SPSPS_TIME_DISCONTINUITY,
	SPSPS_DEFERRED_BEGIN_CONFIGURE,
	SPSPS_DEFERRED_CONFIGURE,
	SPSPS_DEFERRED_END_CONFIGURE
	};

class StreamingParser
	{
	protected:
		
		StreamingPacketState					packetState, pushedPacketState;
		const StreamingDataPacket		*	ppacket;
		uint32									poffset, prange;
		TAG									*	ptags;
		TAG									*	deferredTags;
		int										numDeferredTags, maxDeferredTags;
		bool										inConfigure;
	
		STFResult  EnqueueDeferredTags(TAG * tags);
	public:
	
		StreamingParser(void);
		virtual ~StreamingParser(void);

		//
		// Driving calls
		//
		virtual STFResult ParseReceivePacket(const StreamingDataPacket * packet);
		STFResult ParseDeferredConfigure(void);
		STFResult Flush(void);
		STFResult Commit(void);

		//
		// Streaming control
		//
		virtual STFResult ParseFlush(void) = 0;
		virtual STFResult ParseCommit(void) = 0;
		virtual STFResult ParseInterrupted(void) = 0;

		//
		// Tag list parsing
		//
		virtual STFResult ParseBeginConfigure(void) = 0;
		virtual STFResult ParseConfigure(TAG *& tags) = 0;
		virtual STFResult ParseCompleteConfigure(void) = 0;

		//
		// Data range parsing
		//
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		virtual STFResult ParseRange(const VDRDataRange & range, uint32 & offset) {STFRES_RAISE_OK;}

		//
		// Range information parsing
		//
		virtual STFResult ParseFrameStart(void) = 0;
		virtual STFResult ParseDataDiscontinuity(void) = 0;
		virtual STFResult ParseTimeDiscontinuity(void) = 0;
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool sendNotification) = 0;
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool sendNotification) = 0;
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool sendNotification, bool singleUnitGroup) = 0;
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool sendNotification) = 0;

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time) = 0;
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time) = 0;
		virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration) = 0;
		virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration) = 0;
#if _DEBUG
		STFResult PrintParserState(void);
#endif
	};

class MultipleStreamingParser
	{
	protected:
		
		struct ParserState
			{
			StreamingPacketState				packetState;
			const StreamingDataPacket	*	ppacket;
			uint32								poffset, prange;
			TAG								*	ptags;
			bool									inConfigure;
			} *pParserState;

		uint32									numInputs;
		
		STFResult								ParseInternalReceivePacket(uint32 streamID, const StreamingDataPacket * packet);
		
	public:
	
		MultipleStreamingParser(uint32 numInputs);
		virtual ~MultipleStreamingParser(void);

		//
		// Driving calls
		//
		STFResult ParseReceivePacket(uint32 streamID, const StreamingDataPacket * packet);
		STFResult Flush(uint32 streamID);
		STFResult Commit(uint32 streamID);

		//
		// Streaming control
		//
		virtual STFResult ParseFlush(uint32 streamID) = 0;
		virtual STFResult ParseCommit(uint32 streamID) = 0;
		virtual STFResult ParseInterrupted(uint32 streamID) = 0;

		//
		// Tag list parsing
		//
		virtual STFResult ParseBeginConfigure(uint32 streamID) = 0;
		virtual STFResult ParseConfigure(uint32 streamID, TAG *& tags) = 0;
		virtual STFResult ParseCompleteConfigure(uint32 streamID) = 0;

		//
		// Data range parsing
		//
		virtual STFResult ParseRanges(uint32 streamID, const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		virtual STFResult ParseRange(uint32 streamID, const VDRDataRange & range, uint32 & offset) {STFRES_RAISE_OK;}

		//
		// Range information parsing
		//
		virtual STFResult ParseFrameStart(uint32 streamID) = 0;
		virtual STFResult ParseDataDiscontinuity(uint32 streamID) = 0;
		virtual STFResult ParseTimeDiscontinuity(uint32 streamID) = 0;
		virtual STFResult ParseBeginSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification) = 0;
		virtual STFResult ParseEndSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification) = 0;
		virtual STFResult ParseBeginGroup(uint32 streamID, uint16 groupNumber, bool sendNotification, bool singleUnitGroup) = 0;
		virtual STFResult ParseEndGroup(uint32 streamID, uint16 groupNumber, bool sendNotification) = 0;

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(uint32 streamID, const STFHiPrec64BitTime & time) = 0;
		virtual STFResult ParseEndTime(uint32 streamID, const STFHiPrec64BitTime & time) = 0;
		virtual STFResult ParseCutDuration(uint32 streamID, const STFHiPrec32BitDuration & duration) = 0;
		virtual STFResult ParseSkipDuration(uint32 streamID, const STFHiPrec32BitDuration & duration) = 0;
#if _DEBUG
		STFResult PrintParserState(void);
#endif
	};



inline StreamingFormatter::StreamingFormatter(void)
	{
	packet = NULL;
	groupNumber = 0;
	segmentNumber = 0;
	pendingFrameStart = false;
	numRangeThreshold = VDR_MAX_TAG_DATA_RANGES_PER_PACKET;
	tagUnitIDs = NULL;
	}


inline StreamingFormatter::~StreamingFormatter(void)
	{
	}


inline STFResult StreamingFormatter::UpdatePacket(bool send)
	{
	STFResult	result = STFRES_OK;
	//lint --e{613}
	if (packet && send)
		{
		STFRES_REASSERT(this->SendPacket(packet));

		packet = NULL;
		}
	if (!packet)
		{
		result = this->GetEmptyPacket(packet);
		if (result == STFRES_OBJECT_EMPTY)
			result = STFRES_OBJECT_FULL;
		else if (result == STFRES_OK)
			{
			// This assert makes sure the packet really contains a non-null (and supposedly valid) pointer
			// I had the case that in this branch the pointer was null.
			assert(packet);
			packet->vdrPacket.numRanges = 0;
			packet->vdrPacket.numTags = 0;
			packet->vdrPacket.flags = 0;
			packet->vdrPacket.frameStartFlags = 0;
			packet->vdrPacket.groupNumber = groupNumber;
			packet->vdrPacket.segmentNumber = segmentNumber;
			}
		}

	STFRES_RAISE(result);
	}


inline STFResult StreamingFormatter::Flush(void)
	{
	if (packet)
		{
		STFRES_REASSERT(this->FlushPacket(packet));
		packet = NULL;
		}

	groupNumber = 0xcdcd;
	segmentNumber = 0xcdcd;
	pendingFrameStart = false;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::Commit(void)
	{
	if (packet)
		{
		STFRES_REASSERT(this->SendPacket(packet));

		packet = NULL;
		}

	STFRES_RAISE_OK;
	}

inline STFResult StreamingFormatter::LowLatencyCommit(void)
	{
	if (packet)
		{
		if (packet->vdrPacket.numRanges > 4 || 
			 (packet->vdrPacket.flags & VDR_MSMF_GROUP_END) ||
			 (packet->vdrPacket.flags & VDR_MSMF_END_TIME_VALID))
			{
			STFRES_REASSERT(this->SendPacket(packet));

			packet = NULL;
			}
		}

	STFRES_RAISE_OK;
	}

inline STFResult StreamingFormatter::PutRange(const VDRDataRange & range)
	{
   STFResult	result = STFRES_OK;
	bool			send = false;

	if (packet)
		{
		if (packet->vdrPacket.numRanges + packet->vdrPacket.numTags == VDR_MAX_TAG_DATA_RANGES_PER_PACKET)
			send = true;
		else if (packet->vdrPacket.flags & VDR_MSMF_END_TIME_VALID)
			send = true;
		}

	STFRES_REASSERT(this->UpdatePacket(send));

	if (pendingFrameStart)
		{
		packet->vdrPacket.frameStartFlags |= MKFLAG(packet->vdrPacket.numRanges);
		pendingFrameStart = false;
		}

	packet->vdrPacket.tagRanges.ranges[packet->vdrPacket.numRanges + packet->vdrPacket.numTags] = range;
	packet->vdrPacket.tagRanges.ranges[packet->vdrPacket.numRanges + packet->vdrPacket.numTags].AddRef(packet); // Set the StreamingDataPacket
																																					// to be the holder
	packet->vdrPacket.numRanges++;

   // Re-examine if number of ranges hits the ceiling.
   if (packet->vdrPacket.numRanges >= numRangeThreshold)
      {
      result = this->UpdatePacket(true);
      if (result == STFRES_OBJECT_FULL)
         result = STFRES_OK;
      }

	STFRES_RAISE(result);
	}


inline STFResult StreamingFormatter::PutTag(const TAGITEM & tag)
	{
	int	i;

	if (tagUnitIDs)
		{
		// Check if this tag is supported down stream
		i = 0;
		while (tagUnitIDs[i] && (tag.id & ANYTYPE) != tagUnitIDs[i])
			i++;

		// If not, just drop it
		if (!tagUnitIDs[i] && i != 0)
			STFRES_RAISE_OK;
		}
	else
		{
		// No target tag types selected, so what shall we do with it ?
		// For compatiblity reason, we just let it pass...
		}

	//Prevent putting tags while already having ranges or completed tag set
	if (packet && (packet->vdrPacket.numRanges != 0 || (packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID) != 0))
		STFRES_REASSERT(this->UpdatePacket(true));
	else
		STFRES_REASSERT(this->UpdatePacket(false));

	if (packet->vdrPacket.numTags == VDR_MAX_TAG_DATA_RANGES_PER_PACKET - 1)
		{
		packet->vdrPacket.tagRanges.tags[packet->vdrPacket.numTags] = TAGDONE;
		packet->vdrPacket.numTags++;
		packet->vdrPacket.flags |= VDR_MSMF_TAGS_VALID;

		STFRES_REASSERT(this->UpdatePacket(true));
		}

	packet->vdrPacket.tagRanges.tags[packet->vdrPacket.numTags] = tag;
	//
	// We build a contiguous group, so we have to make sure that
	// the skip value is set to one.
	//
	packet->vdrPacket.tagRanges.tags[packet->vdrPacket.numTags].skip = 1;
	packet->vdrPacket.numTags++;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::CompleteTags(void)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	if ((packet->vdrPacket.numTags) && 
		 (packet->vdrPacket.numRanges == 0) &&
		 (packet->vdrPacket.flags & VDR_MSMF_TAGS_VALID) == 0)
		{
		packet->vdrPacket.tagRanges.tags[packet->vdrPacket.numTags] = TAGDONE;
		packet->vdrPacket.numTags++;
		packet->vdrPacket.flags |= VDR_MSMF_TAGS_VALID;
		}

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutStartTime(const STFHiPrec64BitTime & time)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(packet && packet->vdrPacket.numRanges > 0));

	packet->vdrPacket.flags |= VDR_MSMF_START_TIME_VALID;
	packet->vdrPacket.startTime = time;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutEndTime(const STFHiPrec64BitTime & time)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSMF_END_TIME_VALID;
	packet->vdrPacket.endTime = time;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutCutDuration(const STFHiPrec32BitDuration & duration)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSCF_CUT_AFTER;
	packet->vdrPacket.cutDuration = duration;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutSkipDuration(const STFHiPrec32BitDuration & duration)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSCF_SKIP_UNTIL;
	packet->vdrPacket.skipDuration = duration;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutFrameStart(void)
	{
	pendingFrameStart = true;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutDataDiscontinuity(void)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSMF_DATA_DISCONTINUITY;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::PutTimeDiscontinuity(void)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSMF_TIME_DISCONTINUITY;

	// Always send the packet because after the time discontinuity,
	// there may not be new data coming for quite a while.
	
	STFRES_REASSERT(this->SendPacket(packet));

	packet = NULL;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::BeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(packet && (packet->vdrPacket.flags & VDR_MSMF_GROUP_END) != 0));

	packet->vdrPacket.flags |= VDR_MSMF_GROUP_START;

	if (requestNotification == true)
		packet->vdrPacket.flags |= VDR_MSCF_GROUP_START_NOTIFICATION;

	if (singleUnitGroup == true)
		packet->vdrPacket.flags |= VDR_MSMF_SINGLE_UNIT_GROUP;

	this->groupNumber = groupNumber;
	packet->vdrPacket.groupNumber = groupNumber;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::CompleteGroup(bool requestNotification)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSMF_GROUP_END;

	if (requestNotification == true)
		packet->vdrPacket.flags |= VDR_MSCF_GROUP_END_NOTIFICATION;

	groupNumber = 0;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::BeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	//lint --e{613}
	STFRES_REASSERT(this->UpdatePacket(packet && (packet->vdrPacket.flags & VDR_MSMF_SEGMENT_END) != 0));

	packet->vdrPacket.flags |= VDR_MSMF_SEGMENT_START;
	
	if (requestNotification == true) 
		packet->vdrPacket.flags |= VDR_MSCF_SEGMENT_START_NOTIFICATION;

	this->segmentNumber = segmentNumber;
	packet->vdrPacket.segmentNumber = segmentNumber;

	STFRES_RAISE_OK;
	}


inline STFResult StreamingFormatter::CompleteSegment(bool requestNotification)
	{
 	//lint --e{613}
 	STFRES_REASSERT(this->UpdatePacket(false));

	packet->vdrPacket.flags |= VDR_MSMF_SEGMENT_END;
	
	if (requestNotification == true) 
		packet->vdrPacket.flags |= VDR_MSCF_SEGMENT_END_NOTIFICATION;

	STFRES_REASSERT(this->SendPacket(packet));

	packet = NULL;

	STFRES_RAISE_OK;
	}

#endif // #ifndef STREAMING_FORMATTER_H

