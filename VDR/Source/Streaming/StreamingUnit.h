///
/// @file       VDR/Source/Streaming/StreamingUnit.h
///
/// @brief      VDR Streaming Unit Standard Implementation
///
/// @author     Stefan Herr
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Header File
///
/// @date       2003-01-08
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///


#ifndef STREAMINGUNIT_H
#define STREAMINGUNIT_H

#include "VDR/Source/Unit/UnitCollection.h"
#include "VDR/Source/Unit/IMessageDispatcherUnit.h"
#include "VDR/Source/Base/VDRBase.h"
#include "VDR/Source/Base/VDRMessage.h"
#include "VDR/Source/Streaming/StreamingConnectors.h"
#include "VDR/Source/Streaming/StreamingClock.h"

 
///////////////////////////////////////////////////////////////////////////////
// Streaming Pool Allocator
///////////////////////////////////////////////////////////////////////////////

class StreamingPoolAllocator : public DirectSTFMessageSink
	{
	protected:
		IVDRMemoryPoolAllocator			*	allocator;
		IVDRMessageSinkRegistration	*	allocatorSink;
		IStreamingUnit						*	unit;
		uint32									connectorID;
	public:
		StreamingPoolAllocator(IStreamingUnit * unit, uint32 connectorID);
		virtual ~StreamingPoolAllocator(void);

		STFResult SetAllocator(IVDRMemoryPoolAllocator * allocator);

		bool HasAllocator(void) {return allocator != NULL;}

		STFResult GetMemoryBlocks (VDRMemoryBlock ** blocks, uint32 number, uint32 &done);

		STFResult ReceiveMessage(STFMessage & message);

		IStreamingUnit * GetAllocatorStreamingUnit(void);

		IVDRMemoryPoolAllocator * GetAllocator(void) {return allocator;}
	};



///////////////////////////////////////////////////////////////////////////////
// Base Streaming Unit Implementation
///////////////////////////////////////////////////////////////////////////////


//! Base Streaming Unit
class StreamingUnit : virtual public IStreamingUnit
	{
	protected:
		IStreamingChainUnit	*	parentStreamingUnit;	// Streaming Chain this unit is part of

		IStreamingConnectorPtr	*	connectors;		// Connectors to the outside of the unit
		uint32							numConnectors;
		uint32							totalConnectors;

		VDRStreamingState		state;				// The current Streaming State the unit is in

		//! Called by the unit itself upon completion of a streaming command
		virtual STFResult SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result);

		//! Check state for command execution
		/*!
			Checks if the streaming unit is in the right state for the command to be
			executed. This function may only be needed when debugging the system.
		*/
		STFResult CheckCommandState(VDRStreamingCommand com);

		//! Go to a new state
		STFResult SetStreamingState(VDRStreamingState newState);

		//! Adds a connector to the list of connectors
		STFResult AddConnector(IStreamingConnector * connector);

		void ReleaseDestruct(void);
	public:
		StreamingUnit();

		virtual ~StreamingUnit();


		//
		// IBaseStreamingUnit functions
		//

		virtual STFResult GetState(VDRStreamingState & state);

		//
		// IStreamingUnit functions
		//
		virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);
		virtual STFResult FindConnector(uint32 connectorID, IStreamingConnector *& connector);
		virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock);
		virtual STFResult SetParentStreamingUnit(IStreamingChainUnit * unit);
		virtual STFResult CompleteConnection(void);
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);
		// The following 4 functions are default implementations, returning STFRES_UNIMPLEMENTED:
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult IsPushingChain(uint32 connectorID);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void) = 0;
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
#endif
	};




///////////////////////////////////////////////////////////////////////////////
// Base Virtual Streaming Unit Implementation
///////////////////////////////////////////////////////////////////////////////


//! Base Virtual Streaming Unit Implementation
class VirtualStreamingUnit : public StreamingUnit,
									  public VirtualUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		VirtualStreamingUnit(IPhysicalUnit * physical) : VirtualUnit(physical) {}

		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual Streaming Unit Collection Implementation
///////////////////////////////////////////////////////////////////////////////


//! Base Virtual Streaming Unit Collection Implementation
/*!
	This is a simple Virtual Streaming Unit that has other non-streaming subunits.
	For this reason, it inherits from VirtualUnitCollection instead of VirtualUnit.
*/
class VirtualStreamingUnitCollection : public StreamingUnit,
													public VirtualUnitCollection
	{
	protected:
		void ReleaseDestruct(void);
	public:
		VirtualStreamingUnitCollection(IPhysicalUnit * physical, int maxLeafUnits) : VirtualUnitCollection(physical, maxLeafUnits) {}

		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
	};



///////////////////////////////////////////////////////////////////////////////
// Base Streaming Chain Unit Implementation
///////////////////////////////////////////////////////////////////////////////


typedef IStreamingUnit * IStreamingUnitPtr;

//! Base Streaming Chain Unit Implementation
class StreamingChainUnit : public virtual IStreamingChainUnit
	{
	protected:
		IStreamingUnitPtr	*	streamingSubUnits;
		int						totalStreamingSubUnits;
		int						numStreamingSubUnits;

		STFInterlockedInt		commandsIssued;	//! Counter for commands issued to sub units (includes this unit)

		virtual STFResult SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result) = 0;

		//! Called when a sub unit or this unit has completed command execution
		virtual STFResult StreamingCommandCompletedNotification(VDRStreamingCommand command, STFResult result);

 		STFResult CompleteSubUnitConnection(void);

		STFResult PropagateSubUnitStreamingClock(IStreamingClock * streamingClock);

		//! Plug a pair of connectors of two subunits of the chain
		/*!
			\param sourceUnitID: the index in the array of streaming subunits of the source unit
			\param sourceConnectorID: the connector ID of the source connector, belonging to the source unit
			\param destUnitID: the index in the array of streaming subunits of the destination unit
			\param destConnector: the connector ID of the destination connector, belonging to the destination unit

			As a result the source connector is plugged into the destination connector.
		*/
		STFResult PlugSubUnitConnectors(uint32 sourceUnitID, uint32 sourceConnectorID, uint32 destUnitID, uint32 destConnectorID);
		STFResult UnplugSubUnitConnectors(uint32 sourceUnitID, uint32 sourceConnectorID, uint32 destUnitID, uint32 destConnectorID);

		void ReleaseDestruct(void);
	public:
		StreamingChainUnit(int maxSubUnits)
			{
			streamingSubUnits			= new IStreamingUnitPtr[maxSubUnits];
			totalStreamingSubUnits	= maxSubUnits;
			numStreamingSubUnits		= 0;
			}

		virtual ~StreamingChainUnit(void);

		//! Add a Streaming Sub Unit to the list of subunits
		STFResult AddStreamingSubUnit(IStreamingUnit * unit)
			{
			if (numStreamingSubUnits == totalStreamingSubUnits)
				STFRES_RAISE(STFRES_OBJECT_FULL);

			streamingSubUnits[numStreamingSubUnits++] = unit;
			STFRES_RAISE_OK;
			}

		//! Forward the prepare phase to all child units
		STFResult ForwardPrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);

		//! Forward the begin phase to all child units
		STFResult ForwardBeginStreamingCommand(VDRStreamingCommand command, int32 param);

		//! Forward the complete phase to all child units
		STFResult ForwardCompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);

		//
		// IStreamingChainUnit functions
		//
		// The following are empty default implementations, returning STFRES_UNIMPLEMENTED

		virtual STFResult NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet);
		virtual STFResult NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets);
 		virtual STFResult NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult NestedGetStreamTagIDs(uint32 connectorID, VDRTID * & ids);
		virtual STFResult NestedIsPushingChain(uint32 nestedConnectorID);

		// IBaseStreamingUnit functions
		// Temporarily, we override this only to overcome a problem when destructing Streaming Chains in the
		// course of the VirtualStreamingProxyUnit's destructor. Then it can happen that the destructor of
		// the proxied chain is called after the virtual tables of the proxy unit have been destructed.
		// A real solution is the introduction of a controlled virtual unit cleanup phase, based on the
		// Virtual Unit set tree.
		virtual STFResult GetState(VDRStreamingState & state);
	};



///////////////////////////////////////////////////////////////////////////////
// Base Virtual Streaming Chain Unit Implementation
///////////////////////////////////////////////////////////////////////////////


//! Base Virtual Streaming Chain Unit realizing a chain with its subunits
class VirtualStreamingChainUnit : public StreamingUnit,
											 public StreamingChainUnit,
											 public VirtualUnitCollection		// is also a VirtualUnit
	{
	protected:
//		virtual STFResult SignalCommandArrival(void);

		virtual STFResult SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result);

		void ReleaseDestruct(void);
	public:
		VirtualStreamingChainUnit(IPhysicalUnit * physicalUnit, int maxLeafUnits) 
			: StreamingChainUnit(maxLeafUnits), VirtualUnitCollection(physicalUnit, maxLeafUnits)
			{
			}

		//
		// IVirtualUnit Interface functions
		//

		//! Virtual Streaming Chain Unit Initialisation
		/*!
			Register as parent streaming unit at all managed subunits.
		*/
		virtual STFResult Initialize(void);

		//
		// IVDRBase functions
		//
		// VirtualUnitCollection::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// IStreamingUnit functions
		//
		virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);
		virtual STFResult CompleteConnection(void);
		virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock);

		//
		// IStreamingChainUnit functions
		//
		//! Process messages from child units
		virtual STFResult UpchainNotification(VDRMID message, uint32 param1, uint32 param2);

		virtual STFResult GetState(VDRStreamingState & state)
			{
			// Resolve inheritance ambiguity
			STFRES_RAISE(StreamingUnit::GetState(state));
			}
	};




///////////////////////////////////////////////////////////////////////////////
// Base Virtual Streaming Proxy Unit Implementation
///////////////////////////////////////////////////////////////////////////////

//! Base Virtual Streaming Proxy Unit Implementation
class VirtualStreamingProxyUnit : public virtual IVDRStreamingProxyUnit,
											 public StreamingChainUnit,
											 public VirtualUnitCollection,  // To carry the SubUnits
											 public MessageSinkRegistration
	{
	protected:
		StreamingClock			*	masterClock;
		VDRStreamingState			state;
		int32							direction;	// Current direction of streaming
		int32							speed;		// Current speed of streaming

		STFMutex						proxyMutex;	// Global mutex protecting critical functions of the proxy

		/// Override from StreamingChainUnit
		virtual STFResult SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result);

		void ReleaseDestruct(void);
	public:
		VirtualStreamingProxyUnit(IPhysicalUnit * physicalUnit,
										  int maxLeafUnits);

		virtual ~VirtualStreamingProxyUnit();

		//
		// IVDRStreamingProxyUnit functions
		//
		virtual STFResult SendCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult GetState(VDRStreamingState & state);
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset);

		//
		// IStreamingChainUnit functions
		//
		virtual STFResult NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet);
		virtual STFResult NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets);
		virtual STFResult UpchainNotification(VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult NestedIsPushingChain(uint32 nestedConnectorID);

		//
		// IVDRBase functions
		//
		// VirtualUnitCollection::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
	};



///////////////////////////////////////////////////////////////////////////////
// Physical & Virtual Sinlge Streaming Proxy Units Implementation
///////////////////////////////////////////////////////////////////////////////


//! Standard Physical Streaming Chain Unit
class PhysicalSingleStreamingProxyUnit : public ExclusivePhysicalUnit
	{
	friend class VirtualSingleStreamingProxyUnit;

	protected:
		IPhysicalUnit	*	messageDispatcherUnit;
		IPhysicalUnit	*	subStreamingUnit;	//! The single Physical Streaming Sub-Unit

	public:
		PhysicalSingleStreamingProxyUnit(VDRUID unitID) : ExclusivePhysicalUnit(unitID) {}

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


//! Virtual Streaming Proxy Unit representing one single Virtual Streaming Chain Unit
class VirtualSingleStreamingProxyUnit : public VirtualStreamingProxyUnit
	{
	friend class PhysicalSingleStreamingProxyUnit;

	protected:
		PhysicalSingleStreamingProxyUnit *	physicalSingleProxy;	//! Physical counterpart to this class
		IMessageDispatcherUnit				*	messageDispatcherUnit;

		QueuedNestedInputConnectorPtr		*	inputConnectors;
		NestedOutputConnectorPtr			*	outputConnectors;
		IVDRMemoryPoolAllocatorPtr			*	allocators;

		uint32	numInputConnectors;
		uint32	numOutputConnectors;

	public:
		VirtualSingleStreamingProxyUnit (PhysicalSingleStreamingProxyUnit * physicalUnit);
		~VirtualSingleStreamingProxyUnit (void);

		//
		// IVDRVirtualUnit
		//
		virtual STFResult AllocateChildUnits(void);
 		virtual STFResult	InternalUpdate(void);


		//
		// IVDRStreamingUnit functions
		//
		virtual STFResult NestedReceiveAllocator(uint32 nestedConnectodID, IVDRMemoryPoolAllocator * allocator);

		//
		// IVDRStreamingProxyUnit functions
		//
		virtual STFResult GetDataPackets(uint32 connectorID, VDRStreamingDataPacket * packets, 
													uint32 numPackets, uint32 & filledPackets);
		virtual STFResult RequestDataPackets(uint32 connectorID);
		virtual STFResult DeliverDataPackets(uint32 connectorID,	VDRStreamingDataPacket * packets,
														 uint32 numPackets, uint32 & acceptedPackets);
		virtual STFResult ProvideAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult RequestAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * & allocator);

		//
		// IVirtualUnit Interface functions
		//
		//! Initialize Streaming Proxy Unit
		/*!
			Register as parent streaming unit at the managed subunit.
			Creates the necessary nested input and output connectors according to
			what the managed subunit offers as connections, and connects them.
		*/
		virtual STFResult Initialize(void);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
#endif
	};


#endif // #ifndef STREAMINGUNIT_H
