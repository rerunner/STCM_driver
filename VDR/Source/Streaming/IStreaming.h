
//
// PURPOSE:		VDR Internal Streaming Interfaces & Definitions
//

/*! \file
  \brief VDR Internal Streaming Interfaces & Definitions
*/

#ifndef ISTREAMING_H
#define ISTREAMING_H

#include "VDR/Interface/Base/IVDRDebug.h"
#include "VDR/Interface/Streaming/IVDRStreaming.h"
#include "VDR/Source/Base/VDRBase.h"
#include "VDR/Source/Streaming/IStreamingClocks.h"
// Include this last - otherwise, windows.h will be included afterwards and generate a compile error
// (fix this in OSSTFDataManipulationMacros.h")
#include "STF/Interface/STFDataManipulationMacros.h"


// Forward declarations
class StreamingDataPacket;
class IBaseStreamingUnit;


///////////////////////////////////////////////////////////////////////////////
// Streaming Data Packet Manager
///////////////////////////////////////////////////////////////////////////////

//! Streaming Data Packet Manager Interface ID
static const VDRIID VDRIID_STREAMING_DATA_PACKET_MANAGER = 0x80000025;

//! Streaming Data Packet Manager Interface
/*!
  Functions to retrieve empty data packets from a pool of packets and
  to return them to the pool again.
*/
class IStreamingDataPacketManager// : public virtual IVDRGenericDebugInfo
   {
   public:
   virtual ~IStreamingDataPacketManager(){}; // NHV: Added for g++ 4.1.1

   //! Gets a "new" data packet from the data packet pool
   /*!
     The output connector keeps a pool of data packets. The connector's
     Streaming Unit can get a packet to be filled using this function.
   */
   virtual STFResult GetEmptyDataPacket(StreamingDataPacket *& packet) = 0;

   //! Return data packet to pool
   /*!
     This function is called from somewhere downstream to return the data
     packet to the pool, so that it can be reused
   */
   virtual STFResult ReturnDataPacket(StreamingDataPacket * packet) = 0;

#if _DEBUG
   virtual STFResult PrintDebugInfo (uint32 id = LOGID_ERROR_LOGGING) = 0;
#endif
   };


///////////////////////////////////////////////////////////////////////////////
// Streaming Connector Interface
///////////////////////////////////////////////////////////////////////////////

//@{
//! Flags definitions for connector type bitfield returned by IStreamingConnector->GetType()
/*!
  STRCTF stands for "Streaming Connector Type Flag".
*/

//! Input Connector
#define VDR_STRCTF_INPUT			MKFLAG(0)

//! Output Connector
#define VDR_STRCTF_OUTPUT			MKFLAG(1)

//! Connector blocks on data reception
#define VDR_STRCTF_SYNCHRONOUS	MKFLAG(2)

//! Connector is a Parent Unit connector
#define VDR_STRCTF_PARENT			MKFLAG(3)

//! Connector queues data packets
#define VDR_STRCTF_QUEUE			MKFLAG(4)
//@}


// Forward declaration
class IStreamingUnit;
class IStreamingInputConnector;
class IStreamingOutputConnector;

static const VDRIID VDRIID_STREAMING_CONNECTOR = 0x80000010;

//! Generic Streaming Connector Interface, not part of the interface
class IStreamingConnector
   {
   public:
   virtual ~IStreamingConnector(void) {}

   //! Break connection with other connector
   /*!
     This function must only be called if the Streaming Chain
     formed by the Streaming Units in a Unit Set is in the Initial
     state.
   */
   virtual STFResult Unplug(void) = 0;

   //! Get the connector this connector is linked with
   /*!
     If there is no connection or Connect() was called with NULL
     before, a warning message (STFRES_NOT_CONNECTED) is returned.
   */
   virtual STFResult GetLink(IStreamingConnector *& linkedWith) = 0;

   //! Get Streaming interface of the Streaming Unit this connector belongs to
   virtual STFResult GetStreamingUnit(IBaseStreamingUnit *& unit) = 0;

   //! Get ID local to unit that offers this connector
   /*!
     Connectors are indexed from 0 to numberOfConnectors - 1
     This function retrieves this index as an ID local to the
     Streaming Unit exposing the connector.
   */
   virtual uint32	GetID(void) = 0;
		
   //! Get type of connector (e.g. input or output). See description of STRCTF flags.
   virtual uint32 GetType(void) = 0;

   //! Convert the connector interface to an input connector interface, if it is one
   virtual IStreamingInputConnector * QueryInputConnector(void) = 0;

   //! Convert the connector interface to an output connector interface, if it is one
   virtual IStreamingOutputConnector * QueryOutputConnector(void) = 0;

   //! Get the supported stream TAG IDs of this connector
   virtual STFResult GetStreamTagIDs(VDRTID * & ids) = 0;

   //! Check, whether this chain is a push chain (e.g. has a capture input as a source)
   virtual STFResult IsPushingChain(void) = 0;

#if _DEBUG
   virtual STFString GetInformation(void) = 0;
#endif
   };


typedef IStreamingConnector * IStreamingConnectorPtr;


///////////////////////////////////////////////////////////////////////////////
// Input and Output Connector Interfaces
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_STREAMING_INPUT_CONNECTOR = 0x80000011;

//! Input Connector Interface
class IStreamingInputConnector : public virtual IStreamingConnector
   {
   public:
   //! Build connection with other connector
   /*!
     This function must only be called if the Streaming Chain
     formed by the Streaming Units in a Unit Set is in the Initial
     state.
   */
   virtual STFResult Plug(IStreamingOutputConnector * link) = 0;

   //! Get the connector this connector is linked with
   /*!
     If there is no connection or Connect() was called with NULL
     before, a warning message (STFRES_NOT_CONNECTED) is returned.
   */
   virtual STFResult GetLink(IStreamingOutputConnector *& linkedWith) = 0;

   //! Receive single data packet on the input connector
   /*!
     If the input connector has a queue, the function returns immediately
     after the packet has been put into the queue. If there is no queue,
     this call may not return before the data packet has been processed
     by the downstream units.
     This call will add a reference count to all Data Ranges in the data
     packet, except if there is an error, e.g. because of a full queue.
   */
   virtual STFResult ReceivePacket(StreamingDataPacket * packet) = 0;

   //! Called by the Input Connector's unit to ask for more data packets
   /*!
     If the Input Connector is buffered, one of the Streaming Unit's
     Data Service Threads can call this function to request more data packets.
     If there are any in the connector's queue, they are then provided
     to the unit in the context of the corresponding Data Service Thread or
     interrupt service routine.
     If there are no packets in the queue, or the input connector does not
     have a queue, this call is forwarded to the attached output connector.
     The call must not block, and should have a short execution time.
   */
   virtual STFResult RequestPackets(void) = 0;

   /// Flush any queued packets and the current packet
   virtual STFResult FlushPackets(void) = 0;

   //! Send Upstream Notification
   /*!
     Called from the Streaming Unit of this input connector to send
     an upstream notification message to the attached output connector.
   */
   virtual STFResult SendUpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;

   //! Provide an allocator to the attached output connector
   virtual STFResult ProvideAllocator(IVDRMemoryPoolAllocator * allocator) = 0;

#if _DEBUG
   virtual STFString GetInformation(void) = 0;
#endif
   };


static const VDRIID VDRIID_STREAMING_OUTPUT_CONNECTOR = 0x80000012;

//! Output Connector Interface
class IStreamingOutputConnector : public virtual IStreamingConnector,
                                  public virtual IStreamingDataPacketManager
   {
   public:
   //! Build connection with other connector
   /*!
     This function must only be called if the Streaming Chain
     formed by the Streaming Units in a Unit Set is in the Initial
     state.
   */
   virtual STFResult Plug(IStreamingInputConnector * link) = 0;

   //! Get the connector this connector is linked with
   /*!
     If there is no connection or Connect() was called with NULL
     before, a warning message (STFRES_NOT_CONNECTED) is returned.
   */
   virtual STFResult GetLink(IStreamingInputConnector *& linkedWith) = 0;

   //! Send single packet to attached input connector
   /*!
     The Output Connector's Streaing Unit calls this function to send
     a data packet downstream. Depending on the implementation of the
     attached Input Connector, this call may immediately return after
     the delivery (Queued Input Connector) or after the downstream
     unit(s) have processed the packet.
   */
   virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;

   //! Receive upstream notification
   /*!
     Called by the attached input connector. This notification is processed
     and then forwarded to the Streaming Unit of this output connector.
   */
   virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;

   //! Receive an allocator from the attached input connector
   virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator) = 0;

#if _DEBUG
   virtual STFString GetInformation(void) = 0;
#endif
   };

static inline STFResult PlugConnectors(IStreamingConnector * connector1, IStreamingConnector * connector2)
   {
   IStreamingInputConnector	*	tempInputConnector;
   IStreamingOutputConnector	*	tempOutputConnector;
   //lint --e{613}
   tempInputConnector = connector1->QueryInputConnector();
   if (!tempInputConnector)
      {
      tempOutputConnector = connector1->QueryOutputConnector();
      tempInputConnector  = connector2->QueryInputConnector();
      }
   else
      tempOutputConnector = connector2->QueryOutputConnector();

   assert(tempInputConnector && tempOutputConnector);

   STFRES_RAISE(tempInputConnector->Plug(tempOutputConnector));
   }


///////////////////////////////////////////////////////////////////////////////
//! Internal Streaming Data Packet
///////////////////////////////////////////////////////////////////////////////

class StreamingDataPacket : public STFInterlockedNode,
                            public virtual IVDRDataHolder,
                            public VDRBase
   {
#if _DEBUG
   protected:
   IVDRDataHolder	**loggingArray;
   uint32			maxLoggingEntries;
   uint32			numLoggingEntries;

   void AddToLogging (IVDRDataHolder * holder);
   void RemoveFromLogging (IVDRDataHolder * holder);
#endif
   protected:
   //! Pointer to object that has created (owns) the data packet. 
   /*!
     An application must not make use of this pointer.
   */
   IStreamingDataPacketManager * originator;

   public:
   VDRStreamingDataPacket vdrPacket;
		
   StreamingDataPacket(IStreamingDataPacketManager * originator);
   ~StreamingDataPacket(void);

   /// Copy data content of a VDRStreamingDataPacket to this StreamingDataPacket
   void CopyFromVDRPacket(const VDRStreamingDataPacket * packet);

   /// Copy data content of this StreamingDataPacket to a VDRStreamingDataPacket
   void CopyToVDRPacket(VDRStreamingDataPacket * packet);

   /// Release the packet to return it to its originator
   STFResult ReturnToOrigin(void);

   /// Add a reference to the contained Data Ranges
   /// The packet itself will be registered as the owner of the ranges, so a call
   /// to this function must always be paired with a corresponding ReleaseRanges call!
   void AddRefToRanges(void);

   /// Release reference to the contained Data Ranges
   void ReleaseRanges(void);

   /// Transfer ownership of all ranges to a new IVDRDataHolder. 
   /// This function adds a reference on each contained range for a specified data holder.
   /// Then it calls ReleaseRanges() to release the reference of this packet on the ranges.
   void TransferRangesOwnership(IVDRDataHolder * holder);

   void AddPacketOwner(IVDRDataHolder * holder);
   void RemPacketOwner(IVDRDataHolder * holder);

   uint32 GetPacketDataSize(void);

#if _DEBUG
   //
   // IVDRGenericDebugInfo functions
   //
   virtual STFResult PrintDebugInfo (uint32 id = LOGID_ERROR_LOGGING);
#endif
   };

typedef StreamingDataPacket	*	StreamingDataPacketPtr;

#if _DEBUG
///////////////////////////////////////////////////////////////////////////////
// "Streaming" Unit Debugging Interface
///////////////////////////////////////////////////////////////////////////////

class IStreamingUnitDebugging : public virtual IVDRBase
   {
   public:
   //! Get Info string of a Streaming Unit - good for debug printing
   virtual STFString GetInformation(void) = 0;
   };
#endif	// #if _DEBUG



///////////////////////////////////////////////////////////////////////////////
// Extension interface of IVDRStreamingUnit for enabling debugging facilities
///////////////////////////////////////////////////////////////////////////////

class IBaseStreamingUnit : public virtual IVDRBase,
                           public virtual IVDRDataHolder

#if _DEBUG
                         ,public virtual IStreamingUnitDebugging
#endif
   {
   public:
   /// Function to retrieve the current Streaming State of the streaming unit
   virtual STFResult GetState(VDRStreamingState & state) = 0;
   };


///////////////////////////////////////////////////////////////////////////////
// Streaming Chain Unit Interface
///////////////////////////////////////////////////////////////////////////////

//! Streaming Chain Unit Interface ID
static const VDRIID VDRIID_STREAMING_CHAIN_UNIT = 0x8000001d;

//! Streaming Chain Unit Interface
/*!
  A Streaming Chain Unit implements a Streaming Chain built of child Streaming Units.
  The Streaming Chain Unit connects special "internal" output connectors to the
  input connectors of the subunits, and special internal input connectors to the
  output connectors of the subunits.
  These connectors call functions of this interface to notify the Streaming Chain Unit
  about upstream notifications and the arrival of data packets.
*/
class IStreamingChainUnit : public virtual IBaseStreamingUnit
   {
   public:
   //! Receive upstream notification from a child Nested Output Connector
   virtual STFResult NestedUpstreamNotification(uint32 nestedConnectorID,
                                                VDRMID message, uint32 param1, uint32 param2) = 0;

   //! Receive packet for processing from an Nested Input Connector
   virtual STFResult NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet) = 0;

   //! Signals that new data packets are available on a specific Nested Input Connector
   virtual STFResult NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets) = 0;

   //! Receive message from subunit(s)
   /*!
     The subunits in the chain provide their notifications (e.g. command completion
     messages) through this function.
   */
   virtual STFResult UpchainNotification(VDRMID message, uint32 param1, uint32 param2) = 0;

   //! Receive the allocator from a nested output pin
   virtual STFResult NestedReceiveAllocator(uint32 nestedConnectodID, IVDRMemoryPoolAllocator * allocator) = 0;

   //! Get the supported stream tag types for the given connector
   virtual STFResult NestedGetStreamTagIDs(uint32 nestedConnectorID, VDRTID * & ids) = 0;

   //! Check, whether this input is connected to a pushing chain
   virtual STFResult NestedIsPushingChain(uint32 nestedConnectorID) = 0;

   };


///////////////////////////////////////////////////////////////////////////////
// "Streaming" Unit Interface
///////////////////////////////////////////////////////////////////////////////


//! Internal Streaming Unit Interface ID
static const VDRIID VDRIID_STREAMING_UNIT = 0x80000014;


//! Internal Streaming Unit Interface
/*! A Streaming Unit also exposes the messaging interface (IVDRMessage).
  This is necessary for the exchange of streaming messages and streaming events
  between driver and application.
*/
class IStreamingUnit : public virtual IBaseStreamingUnit
   {
   public:
#if 0
   //! Undo a command that was previously issued to this unit
   /*!
     If a streaming chain unit has several subunits, and execution of the
     command fails for at least one of them, the command execution must
     be undone at all other units for which it did not fail.
   */
   virtual STFResult UndoCommand(VDRStreamingCommand com) = 0;
#endif

   //! Prepare a command for execution
   /*!
     This phase of the command execution is supposed to set up the transitional state
     for all units.  Flags to stop processing should be set if appropriate.  This call
     should not wait for any long duration operation to finish, but return immediately.
   */
   virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState) = 0;

   //! Start command execution (this part takes most time)
   /*!
     This phase should start the command execution.  On completion of this phase, a unit
     shall signal VDRMID_STRM_COMMAND_COMPLETED to its parent uint.  All units of the
     chain remain in the transitional state, until all have signalled completion.
   */
   virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param) = 0;

   //! Complete and clean up command execution
   /*!
     This phase finaly sets the new steady state for the chain.  It will be called when
     the last unit of the chain has signalled command completion.
   */
   virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState) = 0;

   //! Get access to a connector with a specific connector ID.
   /*!
     This function can be used to enumerate the connectors of a Streaming Unit,
     as the connector IDs are indexed from 0 to numberOfConnectors - 1.
     An error (STFRES_...) is returned if no connector with the specified ID
     can be found.
   */
   virtual STFResult FindConnector(uint32 connectorID, IStreamingConnector *& connector) = 0;

   //! Receive a new packet for processing
   /*!
     An input connector provides a new data packet for processing by calling
     this function. If the Input Connector is unqueued, the function is
     called within the context of an output connector's or upstream unit's
     thread. If it is queued, it is called in the context of this streaming
     unit's thread or interrupt servicing routine.
     \param id denominates the input connector that is providing the packet.
   */
   virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet) = 0;

   //! Signals that new data packets are available on a specific input connector
   /*!
     If the unit has queued Input Connectors, they call this function to
     notify a Data Servicing Thread about the arrival of new data. \param id
     is the ID of the input connector that has new data available.
   */
   virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets) = 0;

   //! Receive upstream notifications from an output connector.
   /*!
     param is the uniquePacketID for the Begin/End Processing messages.
   */
   virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2) = 0;

   //! Propagate the streaming clock down through the tree
   /*!
     The owner of the streaming clock (in most cases the proxy) will propagate
     the clock through the streaming graph.  A streaming unit that is a
     streaming clock client, will use the clock.
   */
   virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock) = 0;

   //! Set parent streaming unit (which is always a Streaming Chain Unit)
   virtual STFResult SetParentStreamingUnit(IStreamingChainUnit * unit) = 0;

   //! Receive the allocator from an output pin
   virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator) = 0;

   //! Complete the connection of the unit
   /*!
     This call is made top down through the streaming graph, after all
     connections are established.  It should be used to provide allocators
     to sub/upstream units.
   */
   virtual STFResult CompleteConnection(void) = 0;

   //! Get the supported stream tag types for the given connector
   virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids) = 0;

   //! Check, whether the given connector is connected to a pushing chain
   virtual STFResult IsPushingChain(uint32 connectorID) = 0;
   };


#endif // #ifndef ISTREAMING_H
