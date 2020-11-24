///
/// @file       VDR/Source/Streaming/InfiniteStreamReplicator.h
///
/// @brief      Replicates a single input stream onto any number of output streams
///
/// @author     Ulrich Sigmund
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Header File
///
/// @date       2003-26-09
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///
										

#ifndef INFINITESTREAMREPLICATOR_H
#define INFINITESTREAMREPLICATOR_H

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "VDR/Source/Streaming/StreamingConnectors.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "STF/Interface/STFSynchronisation.h"


// Note: SRMFM_FORWARD_ALL has to be used if none of the clients supports messaging to avoid a deadlock with the message counters not being cleared
enum StreamReplicatorMessageForwardMode
	{
	SRMFM_FORWARD_DEFAULT,	// default handling: group and segment end messages are collected, all other types the first one is forwarded
	SRMFM_FORWARD_FIRST,		// with all message types the first one is forwarded
	SRMFM_FORWARD_COMBINE,	// with all message types all messages are collected to be forwarded
	SRMFM_FORWARD_MAIN,		// only the messages of the main client (output ID 0) are forwarded
	SRMFM_FORWARD_ALL,		// all messages of all clients are forwarded (attention: this could lead to multiple message forwarding)
	SRMFM_TOTAL
	};

/// Physical Stream Replicator Unit
class StreamReplicatorStreamingUnit	: public SharedPhysicalUnit
	{
	protected:
		uint32											numOutputs;
		uint32											numPacketsPerOutput;
		StreamReplicatorMessageForwardMode		messageMode;

	public:
		StreamReplicatorStreamingUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};


static const uint32 NUM_REPLICATOR_UPSTREAM_COUNTERS	= 0x40;	// to cover up to 64 I-frame groups / Nav Packs being stored downstream, before we bounce packets
static const uint32 MASK_REPLICATOR_UPSTREAM_COUNTERS	= NUM_REPLICATOR_UPSTREAM_COUNTERS - 1;

enum StreamReplicatorMessageCounters
	{
	SRMC_SEGMENT_START,
	SRMC_SEGMENT_START_TIME,
	SRMC_GROUP_START,
	SRMC_GROUP_END,
	SRMC_SEGMENT_END,

	SRMC_TOTAL
	};

/// Virtual Stream Replicator Unit
/// This is the standard implementation of a Streaming Unit that replicates one 
/// incoming stream to a configurable number of output connectors.
/// @TODO: Base this class on StandardStreamingUnit
class VirtualStreamReplicatorStreamingUnit : public VirtualNonthreadedStandardStreamingUnit
	{
	protected:
		struct UpstreamEventCounter
			{
			uint32					key;
			STFInterlockedInt		counter;
			bool					*	id;
			} upstreamCounters[NUM_REPLICATOR_UPSTREAM_COUNTERS][SRMC_TOTAL];

		STFInterlockedInt								startupPossibleCounter, startupRequiredCounter;

		uint32										*	outputStreamTimes;
		uint32										*	outputSegmentNumbers;
		STFInterlockedInt								lastMessageTime;

		STFInterlockedInt								pendingLock;					/// Protection of the pending packet processing

		StreamingOutputConnector				**	outputConnectors;				/// Output connectors
		uint32											numOutputs;
		uint32											numPacketsPerOutput;
		StreamingDataPacket						**	pendingOutputPackets;		
		uint32											replicatedPackets;			/// The number of packets replicated
		uint32											deliveredPackets;				/// The number of delivered packets
		VDRTID										*	streamingTAGIDs;
		StreamReplicatorMessageForwardMode		messageMode;

		enum StreamReplicatorSendingState
			{
			SRSS_ARM_SEGMENT_START,
			SRSS_ARM_SEGMENT_START_TIME,
			SRSS_ARM_SEGMENT_END,
			SRSS_ARM_GROUP_START,
			SRSS_ARM_GROUP_END,
			SRSS_REPLICATE_PACKETS,
			SRSS_SEND_PACKETS
			} sendingState;

		void ReleaseDestruct(void);

		STFResult ArmUpstreamCounter(StreamReplicatorMessageCounters counter, uint32 key, int32 value);
		STFResult TriggerUpstreamCounter(StreamReplicatorMessageCounters counter, uint32 key, uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		STFResult ResetUpStreamCounters(void);
	public:
		VirtualStreamReplicatorStreamingUnit(IPhysicalUnit * physical, uint32 numOutputs, uint32 numPacketsPerOutput, StreamReplicatorMessageForwardMode messageMode);
		~VirtualStreamReplicatorStreamingUnit(void);

		STFResult QueryInterface(VDRIID iid, void *& ifp);
		STFResult Initialize(void);
 		STFResult InternalUpdate(void);


		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);
		virtual STFResult CompleteConnection(void);

		/// Returns if input data is currently being used for processing.
		virtual bool InputPending(void) 
			{
			return false;
			}

		//
		// Range information parsing
		//
		virtual STFResult ParseFrameStart(void) { STFRES_RAISE_OK;}
		virtual STFResult ParseDataDiscontinuity(void) { STFRES_RAISE_OK;}
		virtual STFResult ParseTimeDiscontinuity(void) { STFRES_RAISE_OK;}
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool sendNotification) { STFRES_RAISE_OK;}
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool sendNotification) { STFRES_RAISE_OK;}
		virtual STFResult ParseBeginGroup(uint16 groupNumber,bool sendNotification, bool singleUnitGroup) { STFRES_RAISE_OK;}
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool sendNotification) { STFRES_RAISE_OK;}

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time) { STFRES_RAISE_OK;}
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time) { STFRES_RAISE_OK;}
		virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}
		virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}

		// Flush everything on a flush request....
		virtual STFResult ProcessFlushing(void);

		virtual STFResult ParseReceivePacket(const StreamingDataPacket * packet);

		// Special handling for messages requires overriding the following functions:
		virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};

#endif
