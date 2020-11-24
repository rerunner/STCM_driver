///
/// @brief Multiplexer for DVD streams using SCR and buffer fullness of a virtual decoder
///
///
#ifndef DVDSCRMULTIPLEXER_H
#define DVDSCRMULTIPLEXER_H

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/StreamingSupport.h"
#include "VDR/Interface/Unit/Video/Display/IVDRVideoTypes.h"	
#include "Device/Interface/Unit/Video/IMPEGVideoTypes.h"	

//[BS]
#define DEBUG_BITRATE 0	
//[BS]


static const VDRRID VDRRID_PACKET_HEADER  = 0x0000a000;
static const VDRRID VDRRID_PACKET_PAYLOAD = 0x0000b000;
static const VDRRID VDRRID_PACKET_PADDING = 0x0000c000;

class DVDSCRMultiplexerUnit : public SharedPhysicalUnit
	{
	public:
		DVDSCRMultiplexerUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult GetTagIDs(VDRTID * & ids);

		virtual STFResult InternalConfigureTags(TAG * tags);
		virtual STFResult	InternalUpdate(void);
	};

static const uint32	DVDSCRMSID_AUDIO		=	0;
static const uint32	DVDSCRMSID_VIDEO		=	1;
static const uint32	DVDSCRMSID_SPU			=	2;
static const uint32	DVDSCRMSID_TOTAL		=	3;

class VirtualDVDSCRMultiplexerUnit : public VirtualStreamingUnit, protected MultipleStreamingParser
	{
	protected:
		VDRTID									*	mergedStreamTags;	/// The merges stream tag types of this unit and its downstream units

		STFInterlockedInt							pendingLock;		/// Protection of the pending packet processing

		BaseStreamingInputConnector		*	inputConnectors[DVDSCRMSID_TOTAL];	/// Input connector
		OutputConnectorStreamingFormatter	outputFormatter;	/// Streaming Formatter operating on output connector
		StreamingOutputConnector				outputConnector;	/// Output connector
	  
		StreamingDataPacket					*	pendingPackets[DVDSCRMSID_TOTAL];	/// Pending input packets

		volatile bool								packetBounced[DVDSCRMSID_TOTAL];		/// Flag, whether an object full has been sent
		volatile bool								flushRequest;								/// The unit should be flushed
		volatile bool								processRequest;							/// A processing request is still pending

		STFHiPrec64BitDuration					packetDuration, maxSCRtoDTSOffset;	
		STFHiPrec64BitTime						multiplexTime;

//[BS]
#if DEBUG_BITRATE
		uint32								GopSize;
		uint32								GOPBitrate;
		uint32								SegmentBitrate;
		uint32								GOPCount;
		STFHiPrec64BitTime					LastmultiplexTime;
		uint32								packCount;	
		uint32								maxPackCount;
		uint32								maxSegmentBitrate;
#endif
		uint32								videoFrameCountInCurrVOBU;
		uint32								audioFrameCountInCurrVOBU;
//[BS]
		
		struct SCRAccessUnit
			{
			STFHiPrec64BitTime	extractionTime;
			uint32					payloadSize;
			};

		struct SCRStream
			{
			SCRAccessUnit			*	aunits;									// Queue of access units and time of extraction from bit buffer
			uint32						size, mask, first, last;			// Queue control variables

			uint32						bufferLevel;							// Current fullness of virtual decoder buffer
			uint32						bufferSize;								// Total size of decoder buffer
			uint32						skipSize;								// Amount of data at the start of each range, which is not part of the payload
			uint32						payloadSize;							// Size of currently filling access unit
			uint32						packetPayloadSize;					// Size of actual payload in the packet
			uint32						packetStuffingSize;					// Size of stuffing at end of packet
			uint32						frameOffset, frameCount;			// start of first frame and number of frame starts
			MPEGFrameType				frameType;
			uint32						referenceFrameCount;					// number of reference frames in VOBU;
			uint32						referenceFrameMask;					// flags for reference frame starts
			uint32						temporalReference;					//	temporal refernce for next frame

			uint8							mpegStreamID, mpegSubStreamID;	// packet stream ID
			
			VDRDataRange				range;									// Currently building DVD packet

			STFHiPrec64BitTime		presentationTime;						// Presentation time of first access unit in this packet
			bool							presentationTimeValid;
			STFHiPrec64BitTime		decodingTime;							// Decoding time of first access unit in this packet
			bool							decodingTimeValid;
			STFHiPrec64BitTime		groupStartTime;
			bool							groupStartTimeValid;

			STFHiPrec64BitDuration	frameDuration;							// The duration of one access unit
			STFHiPrec64BitTime		extractionTime;						// Extraction time of currently filling access unit

			bool							firstPackOfVOBU;
			bool							firstPackInVOB;						// true for the first pack of each stream in the full VOB
			bool							firstPackInVOBU;						// true for the first pack of each stream of a VOBU

			bool                    openSegment;                     // true while a segment is open on this stream

			STFResult Reset(void);
			STFResult Initialize(uint64 queueSize, uint32 bufferSize, uint8 streamID, uint8 subStreamID, const STFHiPrec64BitDuration & frameDuration);

			SCRStream(void);
			~SCRStream(void);

			bool IsQueueEmpty(void) 
				{return first == last;}

		   SCRAccessUnit & FirstQueueElement(void)
				{return aunits[first & mask];}

			STFResult DequeueExtractedPayload(const STFHiPrec64BitTime & multiplexTime);

			bool CanMux(const STFHiPrec64BitTime & multiplexTime, const STFHiPrec64BitDuration & maxSCRtoDTSOffset);

			STFResult FindClosestEventTime(const STFHiPrec64BitDuration & maxSCRtoDTSOffset, STFHiPrec64BitTime & eventTime, bool & eventTimeValid);

			} streams[DVDSCRMSID_TOTAL];

		uint32						streamProgressFlags;

		TAG							pendingTags[16];
		uint32						numPendingTags;

		uint32						deliverStreamID;
		uint32						groupID, numPacksInVOBU;
		uint32						numSentTags;

		VideoStandard				inputTVSourceSystem;

		void GetSCR(uint32 & scrBase, uint32 & scrExt);

		enum SCRMuxDeliverState
			{
			SCRMXS_PARSE,
			SCRMXS_DELIVER_END_TIME,
			SCRMXS_DELIVER_GROUP_END,
			SCRMXS_DELIVER_START_TIME,
			SCRMXS_DELIVER_GROUP_START,
			SCRMXS_DELIVER_TAGS,
			SCRMXS_DELIVER_COMPLETE_TAGS,
			SCRMXS_DELIVER_RANGE
			} dstate;

		virtual STFResult SendPendingPacket(void);

		STFResult MultiplexPackets(uint32 streamID);
		STFResult FormatPacket(uint32 streamID);
		STFResult MultiplexPacket(uint32 streamID);
		STFResult CompleteAccessUnit(uint32 streamID);

		virtual STFResult Reset(void);
		virtual STFResult ProcessFlushing(void);

		virtual STFResult ParseFlush(uint32 streamID);
		virtual STFResult ParseCommit(uint32 streamID);
		virtual STFResult ParseInterrupted(uint32 streamID);

		virtual STFResult ParseBeginConfigure(uint32 streamID);
		virtual STFResult ParseConfigure(uint32 streamID, TAG *& tags);
		virtual STFResult ParseCompleteConfigure(uint32 streamID);

		virtual STFResult ParseRange(uint32 streamID, const VDRDataRange & range, uint32 & offset);

		virtual STFResult ParseFrameStart(uint32 streamID);
		virtual STFResult ParseDataDiscontinuity(uint32 streamID);
		virtual STFResult ParseTimeDiscontinuity(uint32 streamID);
		virtual STFResult ParseBeginSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification);
		virtual STFResult ParseEndSegment(uint32 streamID, uint16 segmentNumber, bool sendNotification);
		virtual STFResult ParseBeginGroup(uint32 streamID, uint16 groupNumber, bool sendNotification, bool singleUnitGroup);
		virtual STFResult ParseEndGroup(uint32 streamID, uint16 groupNumber, bool sendNotification);

		virtual STFResult ParseStartTime(uint32 streamID, const STFHiPrec64BitTime & time);
		virtual STFResult ParseEndTime(uint32 streamID, const STFHiPrec64BitTime & time);
		virtual STFResult ParseCutDuration(uint32 streamID, const STFHiPrec32BitDuration & duration);
		virtual STFResult ParseSkipDuration(uint32 streamID, const STFHiPrec32BitDuration & duration);

		virtual STFResult ProcessPendingPackets(void);
	public:
		VirtualDVDSCRMultiplexerUnit(IPhysicalUnit * physical);
		~VirtualDVDSCRMultiplexerUnit(void);

		virtual STFResult Initialize(void);

		// Tag processing
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);
		virtual STFResult InternalConfigureTags(TAG *tags);
		virtual STFResult InternalEndConfigure(void);
		virtual STFResult InternalUpdate(void);

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);

		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);

#if _DEBUG
		STFResult PrintDebugInfo (uint32 id);
		STFString GetInformation(void);
#endif
	};

#endif
