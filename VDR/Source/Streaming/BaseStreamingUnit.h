#ifndef BASESTREAMINGUNIT_H
#define BASESTREAMINGUNIT_H

///
/// @file VDR/Source/Streaming/BaseStreamingUnit.h
///
/// @brief Base Implementations for Threaded and Non-Threaded Streaming Units
///
/// @author S. Herr
///
/// @date 2003-08-25 
///
/// @par OWNER: 
/// VDR Architecture Team
///
/// @par SCOPE:
/// INTERNAL Header File
///
/// Base Implementations for Threaded and Non-Threaded Streaming Units.
/// Generic Data Handling.
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"


///////////////////////////////////////////////////////////////////////////////
// StandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////

/// Standard Streaming Unit base class just taking care of the input side
/// (data reception, parsing, synchronized tag handling).
class StandardStreamingUnit : public StreamingUnit,
										public virtual IVDRDataHolder,
										protected StreamingParser
	{
	protected:
		STFInterlockedInt					pendingLock;		/// Protection of the pending packet processing
		IStreamingInputConnector	*	inputConnector;	/// Input connector (received by constructor). Queued or unqueued.

		StreamingDataPacket			*	pendingPacket;		/// The packet currently being processed

		//
		// We can not dynamically increase the amount of tags
		// processed per segment, so we resort to a fixed number
		//
		TAGITEM				pendingTags[64];
		int					numPendingTags, numSentTags;
		bool					tagsPending;

		uint32				changeSet;					/// Change set for unit tags. 
		STFInterlockedInt	pendingChangeSet;	/// Indicates which group of unit tag properties has to be set during processing
															/// of the pending packet. This is used to synchronize tag with stream processing.

		uint32				syncChangeSet;

		uint32				pendingFlags;

		/// Used to send an upstream notification if packet bounced from this streaming unit.
		volatile bool		packetBounced; 
		volatile bool		flushRequest;
		volatile bool		processRequest;

		//
		// Streaming control
		//
		virtual STFResult ParseFlush(void);
		virtual STFResult ParseCommit(void);
		virtual STFResult ParseInterrupted(void);

		//
		// Stream Tag list parsing
		//
		virtual STFResult ParseBeginConfigure(void);
		virtual STFResult ParseConfigure(TAG *& tags);
		virtual STFResult ParseCompleteConfigure(void);
		virtual STFResult GetExclusiveStreamTagIDs(VDRTID * & ids)
			{
			ids = NULL;
			STFRES_RAISE_OK;
			}

		//
		// Range information parsing
		//
		virtual STFResult ParseFrameStart(void);		/// Default implementation - just returns OK


		//
		// Overridable virtual functions
		//

		/// Reset Streaming Data Packet parser
		virtual STFResult ResetParser(void);

		/// Clean up the pending input if u are waiting for some more data
		///  as there is a discontinuity in the stream
		virtual STFResult ResetInputProcessing(void);

		/// Try to process the pending data packet
		virtual STFResult ProcessPendingPacket(bool lowLatencyCommit = false);

		/// Called after all input has been consumed
		virtual STFResult LowLatencyCommit(void);

		/// For processing Flush operations in a derived class
		virtual STFResult ProcessFlushing(void);
		//	{
		//	STFRES_RAISE_OK;	// Default implementation
		//	}

		/// Returns if input data is currently being used for processing.
		virtual bool InputPending(void) = 0;

		/// Get the flag mask of those change set groups the tags of which must be processed synchronously.
		virtual uint32 GetSynchronousChangeFlags(void)
			{
			// Default: none
			return 0;
			}

		/// Update the value of synchronised tags
		virtual STFResult ProcessSynchronizedTags(void);

		//
		// Helper fuctions
		//

		/// Set in and output connectors from the derived classes
		void SetConnectors(IStreamingInputConnector * inputConnector);

		/// Update tags synchronized
		/// In derived classes call this function from InternalUpdate() to make sure that
		/// tags that need to be synchronized with stream processing are properly handled.
		/// This concerns e.g. tags that are needed to be handled at specific points in stream.
		/// Like the stream change tag should only be handled at group boundaries.
		STFResult InternalSynchronizedTagUpdate();

	public:
		/// Specific constructor
		/// @param numOutPackets: number of output packets managed by the output connector
		StandardStreamingUnit();
		~StandardStreamingUnit();

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);

		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
	};



///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////

/// Non-threaded Virtual Streaming Unit with unqueued input connector
class VirtualNonthreadedStandardStreamingUnit : public VirtualUnit,
                                                public StandardStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualNonthreadedStandardStreamingUnit (IPhysicalUnit * physical);
		
		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(StandardStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


/// Non-threaded Virtual Streaming Unit Collection with unqueued input connector
class VirtualNonthreadedStandardStreamingUnitCollection : public VirtualUnitCollection,
                                                          public StandardStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numChildren: number of child units to store in unit collection
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualNonthreadedStandardStreamingUnitCollection(IPhysicalUnit * physical, 
																		  uint32 numChildren);

		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(StandardStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};

class ThreadedStandardStreamingUnit : public StandardStreamingUnit,
												  public STFThread
	{
	protected:
		//
		// STFThread overrides
		//
		void ThreadEntry(void);
		STFResult NotifyThreadTermination(void);

		virtual STFResult CompleteOutputProcessing()
			{
			STFRES_RAISE_OK;
			}

		//
		// Range information parsing - special versions for threaded units
		//
		virtual STFResult ParseDataDiscontinuity(void);
		virtual STFResult ParseTimeDiscontinuity(void);
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification);
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification);

	public:
		/// Specific constructor.
		/// @param inQueueSize: size of input connector's queue
		/// @param inThreshold: (currently) number of packets after which SignalPacketArrival is called.
		/// @param stackSize: stack size of thread
		/// @param priority: priority of thread
		ThreadedStandardStreamingUnit(uint32 inQueueSize,
												int32 inThreshold,
												STFString threadName);

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////

/// Non-threaded Virtual Streaming Unit with unqueued input connector
class VirtualThreadedStandardStreamingUnit : public VirtualUnit,
                                             public ThreadedStandardStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualThreadedStandardStreamingUnit (IPhysicalUnit * physical, 
														  uint32 inQueueSize,
														  int32 inThreshold,
														  STFString threadName);
		
		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(ThreadedStandardStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


/// Non-threaded Virtual Streaming Unit Collection with unqueued input connector
class VirtualThreadedStandardStreamingUnitCollection : public VirtualUnitCollection,
                                                       public ThreadedStandardStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numChildren: number of child units to store in unit collection
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualThreadedStandardStreamingUnitCollection(IPhysicalUnit * physical, 
																	  uint32 numChildren,
																	  uint32 inQueueSize,
																	  int32 inThreshold,
																	  STFString threadName);

		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(ThreadedStandardStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

class StreamingTerminatorUnit : public SharedPhysicalUnit
	{
	friend class VirtualStreamingTerminatorUnit;

	public:
		StreamingTerminatorUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams) {STFRES_RAISE_OK;}
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source) {STFRES_RAISE_OK;}
		virtual STFResult Initialize(uint64 * depUnitsParams) {STFRES_RAISE_OK;}
	};


/// Unit to terminate a Streaming Chain (usually for test purposes or because of incomplete chain implementation)
class VirtualStreamingTerminatorUnit : public VirtualNonthreadedStandardStreamingUnit
	{
	protected:
		//
		// Data range parsing
		//
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);

		//
		// Range information parsing
		//
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup);
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool requestNotification);
		virtual STFResult ParseDataDiscontinuity(void) {STFRES_RAISE_OK;}
		virtual STFResult ParseTimeDiscontinuity(void) {STFRES_RAISE_OK;}
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification);
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification);

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time) {STFRES_RAISE_OK;}
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time) {STFRES_RAISE_OK;}
		virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}
		virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}

		/// Returns if input data is currently being used for processing.
		virtual bool InputPending(void) {return false;}

	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		VirtualStreamingTerminatorUnit (IPhysicalUnit * physical) : VirtualNonthreadedStandardStreamingUnit(physical) {}

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// StreamingInOutTerminatorUnit
///////////////////////////////////////////////////////////////////////////////

class StreamingInOutTerminatorUnit : public StreamingTerminatorUnit
	{
	friend class VirtualStreamingInOutTerminatorUnit;

	public:
		StreamingInOutTerminatorUnit(VDRUID unitID) : StreamingTerminatorUnit(unitID) {}

		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	};


class VirtualStreamingInOutTerminatorUnit : public VirtualStreamingTerminatorUnit
	{
	protected:
		StreamingOutputConnector					outputConnector;	/// Output connector

	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		VirtualStreamingInOutTerminatorUnit (IPhysicalUnit * physical) : VirtualStreamingTerminatorUnit(physical),
																							  outputConnector(0, 0, this)
			{
			AddConnector(&outputConnector);
			}

		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
			{
			STFRES_RAISE(inputConnector->SendUpstreamNotification(message, param1, param2));
			}

		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
			{
			STFRES_RAISE(inputConnector->ProvideAllocator(allocator));
			}			

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// StandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////

/// Standard base implementation of a Streaming Unit with one input and one output
/// connector.
/// The majority of Streaming Units have this configuration, so it makes sense
/// to offer a specially optimized version for this case.
/// This class implements the standard method for handling Streaming Data Packets
/// and Streaming Commands.
/// Usually, you do not directly derive a class from this - derive from
/// the Virtual extensions instead (see below).
class StandardInOutStreamingUnit : public StandardStreamingUnit
	{
	protected:
		StreamingOutputConnector					outputConnector;	/// Output connector
		OutputConnectorStreamingFormatter		outputFormatter;	/// Streaming Formatter operating on output connector

		bool				startTimePending, endTimePending;
		bool				groupEndPending, groupEndNotification, groupStartPending, groupStartNotification;
		bool				cutDurationPending, skipDurationPending;
		uint16			groupNumber;
		bool				singleUnitGroup;

		STFHiPrec64BitTime		startTime, endTime;
		STFHiPrec32BitDuration	cutDuration, skipDuration;

		//
		// Streaming control
		//
		virtual STFResult ParseFlush(void);
		virtual STFResult ParseCommit(void);
		virtual STFResult ParseInterrupted(void);
		virtual STFResult LowLatencyCommit(void);

		//
		// Range information parsing
		//
		virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup);
		virtual STFResult ParseEndGroup(uint16 groupNumber, bool requestNotification);

 		//
		// Time information
		//
		virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time);
		virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration);
		virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration);
		
		//
		// Overridable virtual functions
		//

		/// Reset Streaming Data Packet parser
		virtual STFResult ResetParser(void);

		virtual STFResult	ResetInputProcessing(void);
		virtual STFResult ProcessFlushing(void);

		// Tag processing
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);
		virtual STFResult CompleteConnection(void);

		//
		// Helper fuctions
		//

		/// Set in and output connectors from the derived classes
		void SetConnectors(IStreamingInputConnector * inputConnector);
		

	public:
		/// Specific constructor
		/// @param numOutPackets: number of output packets managed by the output connector
		StandardInOutStreamingUnit(uint32 numOutPackets);
	};


///////////////////////////////////////////////////////////////////////////////
// NonthreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////
 

/// Non-threaded streaming unit with special handling of certain range information
class NonthreadedStandardInOutStreamingUnit : public StandardInOutStreamingUnit
	{
	protected:
		//
		// Range information parsing - special versions for nonthreaded units
		//
		virtual STFResult ParseDataDiscontinuity(void);
		virtual STFResult ParseTimeDiscontinuity(void);
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification);
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification);

	public:
		NonthreadedStandardInOutStreamingUnit(uint32 numOutPackets);
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////

/// Non-threaded Virtual Streaming Unit with unqueued input connector
class VirtualNonthreadedStandardInOutStreamingUnit : public VirtualUnit,
                                                     public NonthreadedStandardInOutStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualNonthreadedStandardInOutStreamingUnit (IPhysicalUnit * physical, uint32 numOutPackets);
		
		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(NonthreadedStandardInOutStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardInOutStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


/// Non-threaded Virtual Streaming Unit Collection with unqueued input connector
class VirtualNonthreadedStandardInOutStreamingUnitCollection : public VirtualUnitCollection,
                                                               public NonthreadedStandardInOutStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numChildren: number of child units to store in unit collection
		/// @param numOutPackets: number of output packets managed by the output connector
		VirtualNonthreadedStandardInOutStreamingUnitCollection(IPhysicalUnit * physical, 
																				 uint32 numChildren,
																				 uint32 numOutPackets);

		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(NonthreadedStandardInOutStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


///////////////////////////////////////////////////////////////////////////////
// ThreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////


/// Threaded Streaming Unit with one queued input and one output connector
/// Usually , you do not directly derive a class from this - derive from
/// the Virtual extensions instead (see below).
class ThreadedStandardInOutStreamingUnit : public StandardInOutStreamingUnit,
														 public STFThread
	{
	protected:
		//
		// STFThread overrides
		//
		void ThreadEntry(void);
		STFResult NotifyThreadTermination(void);

		virtual STFResult CompleteOutputProcessing()
			{
			STFRES_RAISE_OK;
			}

		//
		// Range information parsing - special versions for threaded units
		//
		virtual STFResult ParseDataDiscontinuity(void);
		virtual STFResult ParseTimeDiscontinuity(void);
		virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification);
		virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification);

	public:
		/// Specific constructor.
		/// @param numOutPackets: number of output packets managed by the output connector
		/// @param inQueueSize: size of input connector's queue
		/// @param inThreshold: (currently) number of packets after which SignalPacketArrival is called.
		/// @param stackSize: stack size of thread
		/// @param priority: priority of thread
		ThreadedStandardInOutStreamingUnit(uint32 numOutPackets,
													  uint32 inQueueSize,
													  int32 inThreshold,
													  STFString threadName);

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////


/// Threaded Virtual Streaming Unit
class VirtualThreadedStandardInOutStreamingUnit : public VirtualUnit,
                                                  public ThreadedStandardInOutStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numOutPackets: number of output packets managed by the output connector
		/// @param inQueueSize: size of input connector's queue
		/// @param inThreshold: (currently) number of packets after which SignalPacketArrival is called.
		/// @param stackSize: stack size of thread
		/// @param priority: priority of thread
		VirtualThreadedStandardInOutStreamingUnit(IPhysicalUnit * physical,
																uint32 numOutPackets,
																uint32 inQueueSize,
																int32 inThreshold,
																STFString threadName);
		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(ThreadedStandardInOutStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardInOutStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


/// Threaded Virtual Streaming Unit Collection
/// Use this in case your derived Virtual Streaming Unit has to control virtual subunits which
/// are not Streaming Units themselves.
class VirtualThreadedStandardInOutStreamingUnitCollection : public VirtualUnitCollection,
                                                            public ThreadedStandardInOutStreamingUnit
	{
	protected:
		void ReleaseDestruct(void);
	public:
		/// Specific constructor.
		/// @param physical: Pointer to interface of corresponding physical unit
		/// @param numChildren: number of child units to store in Virtual Unit Collection
		/// @param numOutPackets: number of output packets managed by the output connector
		/// @param inQueueSize: size of input connector's queue
		/// @param inThreshold: (currently) number of packets after which SignalPacketArrival is called.
		/// @param stackSize: stack size of thread
		/// @param priority: priority of thread
		VirtualThreadedStandardInOutStreamingUnitCollection(IPhysicalUnit * physical,
																			 uint32 numChildren,
																			 uint32 numOutPackets,
																			 uint32 inQueueSize,
																			 int32 inThreshold,
																			 STFString threadName);
		//
		// IVDRBase functions
		//
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		/// Override this if you need to update tags asynchronous from the stream handling.
		/// However, you should also call this InternalUpdate() from there to make sure
		/// that synchronous tag values are updated, too.
		virtual STFResult	InternalUpdate(void)
			{
			STFRES_RAISE(ThreadedStandardInOutStreamingUnit::InternalSynchronizedTagUpdate());	// Default implementation
			}
	};


#endif // #ifndef BASESTREAMINGUNIT_H

