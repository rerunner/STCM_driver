///
/// @brief      Splitter for DVD stream, extracts packets with a specific stream ID
///

#ifndef DVDPESSPLITTER_H
#define DVDPESSPLITTER_H

#include "VDR/Interface/Unit/Audio/IVDRAudioTypes.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingSupport.h"
#include "STF/Interface/Types/STFSharedDataBlock.h"
	

#define NO_NAVPACK_PROCESSING 1

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class DVDPESSplitterUnit : public SharedPhysicalUnit
	{
	public:
		DVDPESSplitterUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualDVDPESSplitterUnit : public VirtualNonthreadedStandardInOutStreamingUnit
	{
	protected:
		/// The following array contains the stream IDs to check for.
		/// 0: main stream ID, 1: extension stream ID, 2: private stream ID
		/// If set to 0xff, an ID will not be searched.
		uint8                streamIDs[3];
		uint32               configStreamTypeID;
		uint32               streamingStreamTypeID;
		uint32               currentStreamTypeID;

		DataRangeStreamQueue streamQueue;
		
		uint32               startPTM;			// stream-internal start time of VOBU / group
		uint32               endPTM;				// stream-internal end time of VOBU / group
		uint32               previousStartPTM;	// if we receive PTM by tag, we need the previous and the new Start PTM for calculation
		uint32               previousEndPTM;	// if we receive PTM by tag, we need the previous and the new End PTM for calculation
		uint32               totalPTM;			// total PTM time since the last group start time was sent
		uint32               firstStartPTM;		// stream-internal start time of first VOBU in this segment
		bool                 startTimeValid;
		bool                 endTimeValid;
		bool                 firstPacket;
		bool                 initialNavPackMissing;
		bool                 navPackRequired;	// if true the parser ignores all data upfront first navpack / streams without navpacks
		bool                 skipNavPackPTM;	// if we received start/end PTM by tags, we need to skip nav pack PTM processing
		STFHiPrec64BitTime   outputStartTime;
		STFHiPrec64BitTime   outputEndTime;

		enum DVDPESState
			{
			DVDPESS_PARSE_PACKHEADER_0,
			DVDPESS_PARSE_PACKHEADER_1,
			DVDPESS_PARSE_PACKHEADER_2,
			DVDPESS_PARSE_PACKHEADER_3,
			DVDPESS_PARSE_PAYLOAD,
			DVDPESS_ANALYZE_PACKET,
			DVDPESS_DELIVER_START_TIME,
			DVDPESS_DELIVER_FRAME_START,
			DVDPESS_DELIVER_RANGES,
			DVDPESS_DELIVER_END_TIME,
			DVDPESS_DELIVER_GROUP_END,
			DVDPESS_DELIVER_TAGS,
			DVDPESS_DELIVER_GROUP_START
			} state;

		enum DVDPESConfigState
			{
			DVDPESCS_NONE,
			DVDPESCS_TIME_DISCONTINUITY,
			DVDPESCS_DATA_DISCONTINUITY,
			DVDPESCS_TAGS,
			DVDPESCS_TAGS_DONE
			} pesConfigState;

		uint32	packetSize;

		// Process configuration state - sends configuration stream tags if required
		STFResult ProcessConfigState(void);

		virtual STFResult ParseBeginConfigure(void);
		virtual STFResult ParseCompleteConfigure(void);

		virtual STFResult ParsePESPacket(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		virtual STFResult ParsePESPTS(uint32 offset, uint32 & pts);
		virtual STFResult AnalyzePESPacket(void);
		virtual STFResult DeliverPESPacket(void);
		virtual STFResult ProcessStartEndPTM(void);

		//
		// Range Processing
		//
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);

		//
		// Time processing
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time);

		/// Overridden for synchronized tag handling at segment and group start
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool sendNotification);
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup);

		/// Reset Streaming Data Packet parser
		virtual STFResult ResetParser(void);

		virtual STFResult ResetInputProcessing(void);

		/// For processing Flush operations in a derived class
		virtual STFResult ProcessFlushing(void);

 		/// Returns if input data is currently being used for processing
		virtual bool InputPending(void);

		/// Override from StandardStreamingUnit
		virtual uint32 GetSynchronousChangeFlags(void);

		/// This must be overridden to process changes of the stream ID (= change of stream selection)
		/// This call is synchronized with the streaming processing (called at a group start only).
		virtual STFResult InternalUpdateStreamID(void) = 0;
		virtual STFResult InternalParseStreamID(TAG * tags, uint32 & streamTypeID) = 0;
 		virtual STFResult InternalConfigureTags(TAG * tags);
		virtual STFResult ParseConfigure(TAG *& tags);

	public:
		VirtualDVDPESSplitterUnit(DVDPESSplitterUnit * physical);

		/// Override from StandardStreamingUnit
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
	};

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Splitter Gate Unit
///////////////////////////////////////////////////////////////////////////////
class DVDPESSplitterGateUnit : public DVDPESSplitterUnit
	{
	public:
		DVDPESSplitterGateUnit(VDRUID unitID) : DVDPESSplitterUnit(unitID) {}
	};

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Splitter Gate Unit
///////////////////////////////////////////////////////////////////////////////
class VirtualDVDPESSplitterGateUnit : public VirtualDVDPESSplitterUnit
	{
	protected:
		// Overridden group start/end to detect single unit group.
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup);
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool sendNotification);

		// Range processing.  Prevent feeding data range while staying in SINGLE UNIT GROUP
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		uint32					inSingleUnitGroup;
	public:
		VirtualDVDPESSplitterGateUnit(DVDPESSplitterUnit * physical);
	};

///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Audio Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class DVDPESAudioSplitterUnit : public DVDPESSplitterGateUnit
	{
	public:
		DVDPESAudioSplitterUnit(VDRUID unitID) : DVDPESSplitterGateUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult GetTagIDs(VDRTID * & ids);
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Audio Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualDVDPESAudioSplitterUnit : public VirtualDVDPESSplitterGateUnit
	{
	protected:
		virtual STFResult InternalUpdateStreamID(void);
		virtual STFResult InternalParseStreamID(TAG * tags, uint32 & streamTypeID);

		virtual STFResult ParsePESPTS(uint32 offset, uint32 & pts);
	public:
		VirtualDVDPESAudioSplitterUnit(DVDPESSplitterUnit * physical);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Video Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class DVDPESVideoSplitterUnit : public DVDPESSplitterUnit
	{
	public:
		DVDPESVideoSplitterUnit(VDRUID unitID) : DVDPESSplitterUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult GetTagIDs(VDRTID * & ids);
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Video Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualDVDPESVideoSplitterUnit : public VirtualDVDPESSplitterUnit
	{
	protected:
		uint32		streamID;

		virtual STFResult InternalUpdateStreamID(void);
		virtual STFResult InternalParseStreamID(TAG * tags, uint32 & streamTypeID);

		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time);
	public:
		VirtualDVDPESVideoSplitterUnit(DVDPESSplitterUnit * physical);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Physical DVD PES Subpicture Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class DVDPESSubpictureSplitterUnit : public DVDPESSplitterGateUnit
	{
	public:
		DVDPESSubpictureSplitterUnit(VDRUID unitID) : DVDPESSplitterGateUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult GetTagIDs(VDRTID * & ids);
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual DVD PES Subpicture Splitter Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualDVDPESSubpictureSplitterUnit : public VirtualDVDPESSplitterGateUnit
	{
	protected:
		uint32		streamID;
		uint32		streamType;

		virtual STFResult InternalUpdateStreamID(void);
		virtual STFResult InternalParseStreamID(TAG * tags, uint32 & streamTypeID);
	public:
		VirtualDVDPESSubpictureSplitterUnit(DVDPESSplitterUnit * physical);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};


#endif
