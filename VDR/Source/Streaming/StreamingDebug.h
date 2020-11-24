#ifndef STREAMINGDEBUG_H
#define STREAMINGDEBUG_H

///
/// @file VDR/Source/Streaming/StreamingDebug.h
///
/// @brief Debugging streaming units
///
/// @author U. Sigmund
///
/// @date 2003-08-25 
///
/// @par OWNER: 
/// VDR Architecture Team
///
/// @par SCOPE:
/// INTERNAL Header File
///
/// Logging and other debugging streaming units
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Unit/PhysicalUnit.h"

#include <stdio.h>
#include <stdarg.h>

class LoggingInputConnector : public UnqueuedInputConnector
	{				
	public:
		LoggingInputConnector(uint32 id, IStreamingUnit * unit) : UnqueuedInputConnector(id, unit) 
			{			
			};

		virtual STFResult ReceivePacket(StreamingDataPacket * packet)
			{
			STFRES_RAISE(unit->ReceivePacket(id, packet));
			}
	};

class PhysicalLoggingStreamingUnit : public SharedPhysicalUnit
	{
	friend class VirtualLoggingStreamingUnit;
	protected:
		char		name[256];
	public:
		PhysicalLoggingStreamingUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};

class VirtualLoggingStreamingUnit : public VirtualStreamingUnit
	{
	protected:
		PhysicalLoggingStreamingUnit	*	physicalUnit;
		LoggingInputConnector				inputConnector;
		StreamingOutputConnector			outputConnector;

		bool										insideSegment;
		bool										insideGroup;
	public:
		VirtualLoggingStreamingUnit(PhysicalLoggingStreamingUnit * physicalUnit);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE_OK;
			}

		virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);

		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};

class PhysicalDumpingStreamingUnit : public SharedPhysicalUnit
	{
	friend class VirtualDumpingStreamingUnit;
	protected:
		char		name[256];
		uint32 numInputs;
		uint32 numOutputs;
	public:
		PhysicalDumpingStreamingUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) { numInputs = 1; numOutputs = 1;}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};

class VirtualDumpingStreamingUnit : public VirtualStreamingUnit
	{
	protected:
		PhysicalDumpingStreamingUnit	*	physicalUnit;
		LoggingInputConnector **			inputConnectors;
		StreamingOutputConnector **		outputConnectors;
		uint32									segmentNumber, segmentSize;

		FILE * file;
		FILE * mfile;
	public:
		VirtualDumpingStreamingUnit(PhysicalDumpingStreamingUnit * physicalUnit);
		virtual ~VirtualDumpingStreamingUnit();

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE_OK;
			}

		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};

class PhysicalMultichannelDumpingStramingUnit : public PhysicalDumpingStreamingUnit
	{
	friend class VirtualMultichannelDumpingStreamingUnit;			
	public:
		PhysicalMultichannelDumpingStramingUnit(VDRUID unitID) : PhysicalDumpingStreamingUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);		
	};

class VirtualMultichannelDumpingStreamingUnit : public VirtualDumpingStreamingUnit
	{
	protected:
		STFMutex classLock;

	public:		

		VirtualMultichannelDumpingStreamingUnit(PhysicalMultichannelDumpingStramingUnit * p) : VirtualDumpingStreamingUnit(p) {};

		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);		
	};


 
class PhysicalDumpFedStreamingUnit : public SharedPhysicalUnit
	{
	friend class VirtualDumpFedStreamingUnit;

	protected:
		uint32 numInputs;
		uint32 numOutputs;

		uint32 blockSize;
		uint32 blockNum;

		char		metafileName[32];
		char		datafileName[32];
		
		IPhysicalUnit * physicalAllocator;


	public:
		PhysicalDumpFedStreamingUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) { physicalAllocator = NULL; }
		
		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

	};

static const uint32 MAX_BLOCK_SIZE = 32000;

class VirtualDumpFedStreamingUnit : public VirtualStreamingUnitCollection, public STFThread, public DirectSTFMessageSink
	{
	protected:
		PhysicalDumpFedStreamingUnit * physicalFeedUnit;
		VDRMemoryBlock * outputBlock;
		uint32 actualBlockSize;
		uint32 nextBlockStart;
		uint32 currentPosition;
		uint32 metaFileLength;		

		uint8   metaFileBlock[MAX_BLOCK_SIZE];
		FILE *  dataFile;
		FILE *  metaFile;

		uint32 segmentNumber;
		uint32 groupNumber;
		uint32 startTime;
		uint32 endTime;
		uint32 rangeSize;
		uint32 tagId;
		uint32 tagFirstParam;
		uint32 tagNextParam;

		StreamingPoolAllocator allocator;

		VDRDataRange currentRange;

		uint32 connectorId;

		typedef enum 
			{
			ACTION_BEGIN_SEGMENT,
			ACTION_DATA_DISCONTINUITY,
			ACTTION_BEGIN_GROUP,
			ACTION_PUT_START_TIME,
			ACTION_PUT_TAG,
			ACTION_PUT_FRAME_START,
			ACTION_PUT_RANGE,
			ACTION_PUT_END_TIME,
			ACTION_TIME_DISCONTINUITY,
			ACTION_COMPLETE_GROUP,
			ACTION_COMPLETE_SEGMENT
			} FeedAction;

		FeedAction actionToTake;
		
		StreamingOutputConnector ** outputConnectors;
		OutputConnectorStreamingFormatter ** outputFormatters;
	
		virtual void ThreadEntry();
		STFResult NotifyThreadTermination() { STFRES_RAISE(SetThreadSignal());}

		void ParseMetaFileToken();

		
		STFResult AllocateChildUnits(void);
		STFResult OpenFiles();
		STFResult CloseFiles();
		STFResult ReadMetaFileBlock();
		inline uint32 GetDecimal();
		inline uint32 GetHexadecimal();
		STFResult ParseMetaFileToken(FeedAction & actionToTake);
		STFResult PutRangeFromDateFile(uint32 connectorId, uint32 rangesSize);
		STFResult ReceiveMessage(STFMessage & message);
		STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		STFResult PerformOperation(FeedAction action);

	public:
		VirtualDumpFedStreamingUnit(PhysicalDumpFedStreamingUnit * physical);
		
		STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void) { return "Dump-Fed streaming unit";}
#endif
	};


#endif
