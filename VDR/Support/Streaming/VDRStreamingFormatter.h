#ifndef VDRSTREAMINGFORMATTER_H
#define VDRSTREAMINGFORMATTER_H

///
/// @file       VDR/Support/Streaming/VDRStreamingFormatter.h
///
/// @brief      VDR Streaming Formatter Implementation for client side
///
/// @author     Ulrich Sigmund
///
/// @date       2003-02-10 
///
/// @par OWNER: STCM Streaming Architecture Team
///
/// @par SCOPE: PUBLIC Header File
///
/// VDR Streaming Formatter Implementation for client side
/// The concepts of the classes in this file are documented in the
/// "STCM Media Stream Formatting" document.
///
/// &copy:      2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Interface/Streaming/IVDRStreaming.h"
#include "STF/Interface/STFDataManipulationMacros.h"
#include "STF/Interface/STFSynchronisation.h"


class VDRStreamingFormatter
	{
	protected:
		STFResult UpdatePacket(bool send);

		bool								pendingFrameStart;
		uint16							groupNumber, segmentNumber;

		bool								packetPending, packetValid;
		VDRStreamingDataPacket		packet;
		IVDRStreamingProxyUnit	*	unit;
		int								connectorID;

		uint8								numRangeThreshold;

	public:
		VDRStreamingFormatter(void);
		virtual ~VDRStreamingFormatter(void);

		void SetRangesThreshold(uint8 threshold)
			{
			this->numRangeThreshold = threshold;
			}

		STFResult Connect(IVDRStreamingProxyUnit * unit, int connectorID);
		STFResult Disconnect(void);

		//
		// Streaming control
		//
		STFResult Flush(void);
		STFResult Commit(void);

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
		STFResult CompleteSegment(bool requestNotification);

		//
		// Time information
		//
		STFResult PutStartTime(const STFHiPrec64BitTime & time);
		STFResult PutEndTime(const STFHiPrec64BitTime & time);
		STFResult PutCutDuration(const STFHiPrec32BitDuration & duration);
		STFResult PutSkipDuration(const STFHiPrec32BitDuration & duration);
	};


class VDRStreamingParser
	{
	protected:
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
			SPSPS_TIME_DISCONTINUITY
			} packetState;

		uint32							prange, poffset;
		VDRStreamingDataPacket		packet;
		TAG							*	ptags;
		IVDRStreamingProxyUnit	*	unit;
		int								connectorID;
		bool								inConfigure;
		STFMutex							mutex;

		STFResult ReleasePacket(void);
	public:
		VDRStreamingParser(void);
		virtual ~VDRStreamingParser(void);

		STFResult Connect(IVDRStreamingProxyUnit * unit, int connectorID);
		STFResult Disconnect(void);

		//
		// Driving calls
		//
		STFResult ParseReceivePacket(void);
		STFResult Flush(void);
		STFResult Commit(void);

		//
		// Streaming control
		//
		virtual STFResult ParseFlush(void);
		virtual STFResult ParseCommit(void);

		//
		// Tag list parsing
		//
		virtual STFResult ParseBeginConfigure(void);
		virtual STFResult ParseConfigure(TAG *& tags);
		virtual STFResult ParseCompleteConfigure(void);

		//
		// Data range parsing
		//
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		virtual STFResult ParseRange(const VDRDataRange & range, uint32 & offset) {STFRES_RAISE_OK;}

		//
		// Range information parsing
		//
		virtual STFResult ParseFrameStart(void);
		virtual STFResult ParseDataDiscontinuity(void);
		virtual STFResult ParseTimeDiscontinuity(void);
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool sendNotification);
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool sendNotification);
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool sendNotification, bool singleUnitGroup);
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool sendNotification);

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration);
		virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration);
	};


inline STFResult VDRStreamingFormatter::PutRange(const VDRDataRange & range)
	{
	bool	send = false;

	if (packetValid)
		{
		if (packet.numRanges + packet.numTags == VDR_MAX_TAG_DATA_RANGES_PER_PACKET ||
			 packet.numRanges == numRangeThreshold)
			send = true;
		else if (packet.flags & VDR_MSMF_END_TIME_VALID)
			send = true;
		}

	STFRES_REASSERT(this->UpdatePacket(send));

	if (pendingFrameStart)
		{
		packet.frameStartFlags |= MKFLAG(packet.numRanges);
		pendingFrameStart = false;
		}

	packet.tagRanges.ranges[packet.numRanges + packet.numTags] = range;
	packet.tagRanges.ranges[packet.numRanges + packet.numTags].AddRef();
	packet.numRanges++;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutTag(const TAGITEM & tag)
	{
	//Prevent putting tags while already having ranges or completed tag set
	if (packetValid && (packet.numRanges != 0 || (packet.flags & VDR_MSMF_TAGS_VALID) != 0))
		STFRES_REASSERT(this->UpdatePacket(true));
	else
		STFRES_REASSERT(this->UpdatePacket(false));

	if (packet.numTags == VDR_MAX_TAG_DATA_RANGES_PER_PACKET - 1)
		{
		packet.tagRanges.tags[packet.numTags] = TAGDONE;
		packet.numTags++;
		packet.flags |= VDR_MSMF_TAGS_VALID;

		STFRES_REASSERT(this->UpdatePacket(true));
		}

	packet.tagRanges.tags[packet.numTags] = tag;
	//
	// We build a contiguous group, so we have to make sure that
	// the skip value is set to one.
	//
	packet.tagRanges.tags[packet.numTags].skip = 1;
	packet.numTags++;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::CompleteTags(void)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	if ((packet.numTags) && 
		 (packet.numRanges == 0) &&
		 (packet.flags & VDR_MSMF_TAGS_VALID) == 0)
		{
		packet.tagRanges.tags[packet.numTags] = TAGDONE;
		packet.numTags++;
		packet.flags |= VDR_MSMF_TAGS_VALID;
		}

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutStartTime(const STFHiPrec64BitTime & time)
	{
	STFRES_REASSERT(this->UpdatePacket(packet.numRanges > 0));

	packet.flags |= VDR_MSMF_START_TIME_VALID;
	packet.startTime = time;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutEndTime(const STFHiPrec64BitTime & time)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet.flags |= VDR_MSMF_END_TIME_VALID;
	packet.endTime = time;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutCutDuration(const STFHiPrec32BitDuration & duration)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet.flags |= VDR_MSCF_CUT_AFTER;
	packet.cutDuration = duration;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutSkipDuration(const STFHiPrec32BitDuration & duration)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet.flags |= VDR_MSCF_SKIP_UNTIL;
	packet.skipDuration = duration;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutFrameStart(void)
	{
	pendingFrameStart = true;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutDataDiscontinuity(void)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet.flags |= VDR_MSMF_DATA_DISCONTINUITY;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::PutTimeDiscontinuity(void)
	{
	if (packetValid == false || (packet.flags & VDR_MSMF_TIME_DISCONTINUITY) == 0)
		{
 		STFRES_REASSERT(this->UpdatePacket(false));

		packet.flags |= VDR_MSMF_TIME_DISCONTINUITY;
		}

	// Always send the packet because after the time discontinuity,
	// there may not be new data coming for quite a while.

	STFRES_REASSERT(this->Commit());

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::BeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	STFRES_REASSERT(this->UpdatePacket(packetValid && (packet.flags & VDR_MSMF_GROUP_END) != 0));

	packet.flags |= VDR_MSMF_GROUP_START;
	
	if (requestNotification == true) 
		packet.flags |= VDR_MSCF_GROUP_START_NOTIFICATION;

	if (singleUnitGroup == true)
		packet.flags |= VDR_MSMF_SINGLE_UNIT_GROUP;

	this->groupNumber = groupNumber;
	packet.groupNumber = groupNumber;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::CompleteGroup(bool requestNotification)
	{
	STFRES_REASSERT(this->UpdatePacket(false));

	packet.flags |= VDR_MSMF_GROUP_END;
	
	if (requestNotification == true) 
		packet.flags |= VDR_MSCF_GROUP_END_NOTIFICATION;

	packet.groupNumber = groupNumber;
	this->groupNumber = 0xcdcd;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::BeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->UpdatePacket(packetValid && (packet.flags & VDR_MSMF_SEGMENT_END) != 0));

	packet.flags |= VDR_MSMF_SEGMENT_START;
	
	if (requestNotification == true) 
		packet.flags |= VDR_MSCF_SEGMENT_START_NOTIFICATION;

	this->segmentNumber = segmentNumber;
	packet.segmentNumber = segmentNumber;

	STFRES_RAISE_OK;
	}


inline STFResult VDRStreamingFormatter::CompleteSegment(bool requestNotification)
	{
	if (packetValid == false || (packet.flags & VDR_MSMF_SEGMENT_END) == 0)
		{
 		STFRES_REASSERT(this->UpdatePacket(false));

		if ((packet.flags & VDR_MSMF_SEGMENT_END) == 0)
			{
			packet.flags |= VDR_MSMF_SEGMENT_END;
			
			if(requestNotification == true) 
				packet.flags |= VDR_MSCF_SEGMENT_END_NOTIFICATION;

			packet.segmentNumber = segmentNumber;
			this->segmentNumber = 0xcdcd;
			}
		}

	STFRES_REASSERT(this->Commit());

	STFRES_RAISE_OK;
	}


#endif	// #ifndef VDRSTREAMINGFORMATTER_H
