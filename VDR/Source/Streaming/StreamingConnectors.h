#ifndef STREAMINGCONNECTOR_H
#define STREAMINGCONNECTOR_H

///
/// @file       VDR/Source/Streaming/StreamingConnectors.h
///
/// @brief      VDR Streaming Connectors Standard Implementation
///
/// @author     Stefan Herr
///
/// @date       2003-01-08 
///
/// @par OWNER: VDR Architecture Team
///
/// @par SCOPE: INTERNAL Header File
///
/// VDR Streaming Connectors Standard Implementation
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Interface/Base/IVDRMessage.h"
#include "STF/Interface/Types/STFQueue.h"
#include "VDR/Source/Streaming/IStreaming.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Base/VDRBase.h"

#include "STF/Interface/Types/STFList.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"

#if _DEBUG
#if ! DEBUG_ENABLE_UPSTREAM_NOTIFICATION_LOGGING // see StreamingDiagnostics.h
#define DEBUG_ENABLE_UPSTREAM_NOTIFICATION_LOGGING	0
#endif
#define DBG_VDRMID_STRING(m)	((m==VDRMID_STRM_PACKET_REQUEST)?"PACKET_REQUEST": \
	(m==VDRMID_STRM_PACKET_ARRIVAL)?"PACKET_ARRIVAL": \
	(m==VDRMID_STRM_STARVING)?"STARVING": \
	(m==VDRMID_STRM_PACKET_ARRIVAL_VOBU)?"PACKET_ARRIVAL VOBU": \
	(m==VDRMID_STRM_COMMAND_COMPLETED)?"COMMAND_COMPLETED": \
	(m==VDRMID_STRM_SEGMENT_START)?"SEGMENT_START": \
	(m==VDRMID_STRM_SEGMENT_START_TIME)?"SEGMENT_START_TIME": \
	(m==VDRMID_STRM_SEGMENT_END)?"SEGMENT_END": \
	(m==VDRMID_STRM_GROUP_START)?"GROUP_START": \
	(m==VDRMID_STRM_GROUP_END)?"GROUP_END": \
	(m==VDRMID_STRM_START_POSSIBLE)?"START_POSSIBLE": \
	(m==VDRMID_STRM_START_REQUIRED)?"START_REQUIRED": \
	(m==VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE)?"ALLOCATOR_BLOCKS_AVAILABLE": \
	(m==VDRMID_STRM_DATA_DISCONTINUITY_PROCESSED)?"DATA_DISCONTINUITY_PROCESSED": \
	"???")
#endif

#if DEBUG_ENABLE_UPSTREAM_NOTIFICATION_LOGGING
 #define DP_UPSTREAM_NOTIFICATION	DP
#if DIAGNOSTIC_STREAM_MIXER_STATISTICS
 #define LOG_THIS_VDRMID(m)  \
	((AUDIO_MIXER_IS_STARVING && \
	 (m!=VDRMID_STRM_GROUP_START) && \
	 (m!=VDRMID_STRM_GROUP_END)) || \
	 ((m!=VDRMID_STRM_PACKET_REQUEST) && \
	 (m!=VDRMID_STRM_PACKET_ARRIVAL) && \
	 (m!=VDRMID_STRM_GROUP_START) && \
	 (m!=VDRMID_STRM_GROUP_END)))
#else
 #define LOG_THIS_VDRMID(m)  \
	(((m!=VDRMID_STRM_PACKET_REQUEST) && \
	 (m!=VDRMID_STRM_PACKET_ARRIVAL) && \
	 (m!=VDRMID_STRM_GROUP_START) && \
	 (m!=VDRMID_STRM_GROUP_END)))
#endif // #if DIAGNOSTIC_STREAM_MIXER_STATISTICS
#else
 #define DP_UPSTREAM_NOTIFICATION	while (0) DebugPrintEmpty
 #define LOG_THIS_VDRMID(m)  (false)
#endif

///////////////////////////////////////////////////////////////////////////////
/// Generic Streaming Connector Implementation
///////////////////////////////////////////////////////////////////////////////

class StreamingConnector : public virtual IStreamingConnector
	{
	protected:
		IBaseStreamingUnit	* unit;

		uint32 id;

	public:
		StreamingConnector(uint32 id, IBaseStreamingUnit * unit);

		virtual ~StreamingConnector();

		//
		// IStreamingConnector functions
		//
		virtual uint32		GetID(void);

		virtual STFResult GetStreamingUnit(IBaseStreamingUnit *& unit);

		//! Convert the connector interface to an input connector interface, if it is one
		virtual IStreamingInputConnector * QueryInputConnector(void);

		//! Convert the connector interface to an output connector interface, if it is one
		virtual IStreamingOutputConnector * QueryOutputConnector(void);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};



///////////////////////////////////////////////////////////////////////////////
// Standard Input Connector Implementations
///////////////////////////////////////////////////////////////////////////////

/// Base Input Connector Implementation
class BaseStreamingInputConnector : public StreamingConnector,
												public virtual IStreamingInputConnector
	{
	protected:
		IStreamingOutputConnector * source;

	public:
		BaseStreamingInputConnector(uint32 id,
											 IBaseStreamingUnit * unit) : StreamingConnector(id, unit)
			{
			source = NULL;
			}
		~BaseStreamingInputConnector(void);
		
		virtual uint32 GetType(void)
			{
			return VDR_STRCTF_INPUT;
			}

		//
		// IStreamingConnector functions
		//
		virtual STFResult Unplug(void);
		virtual STFResult GetLink(IStreamingConnector *& linkedWith);

		//
		// IStreamingInputConnector functions
		//
		virtual STFResult Plug(IStreamingOutputConnector * connector);
		virtual STFResult GetLink(IStreamingOutputConnector *& linkedWith);

		virtual STFResult RequestPackets(void);
		virtual STFResult FlushPackets(void)
			{
			STFRES_RAISE_OK;
			}

		STFResult SendUpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
			{
#if _DEBUG // unavoidable because GetInformation does not exist in Release compile mode
			if (LOG_THIS_VDRMID(message))
				DP_UPSTREAM_NOTIFICATION("%s: (input) SendUpstreamNotification(%s)\n", (char *) this->unit->GetInformation(), DBG_VDRMID_STRING(message));
#endif
			if(source)
				{
			STFRES_RAISE(source->UpstreamNotification(message, param1, param2));
				}
			else
				{
				assert(0); //Soj; Added this here as this problem was happening on the RT07 board
						 //If you are merging to the RT07 stream, merge these changes in
						 //This function should be the same for both the projects.
				DP("SendUpstreamNotification::NULL source pointer, ignored\n");
				STFRES_RAISE_OK;
				}
			}

		virtual STFResult ProvideAllocator(IVDRMemoryPoolAllocator * allocator);

		// Override from StreamingConnector
		virtual IStreamingInputConnector * QueryInputConnector(void);

		virtual STFResult IsPushingChain(void);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};

///////////////////////////////////////////////////////////////////////////////
// Unqueued Input Connector Implementations
///////////////////////////////////////////////////////////////////////////////


class UnqueuedInputConnector : public BaseStreamingInputConnector
	{
	protected:
		IStreamingUnit * unit;

	public:
		UnqueuedInputConnector(uint32 id,
									  IStreamingUnit * unit) : BaseStreamingInputConnector(id, unit)
			{
			this->unit = unit;
			}

		//
		// IStreamingInputConnector functions
		//
		virtual STFResult ReceivePacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamTagIDs(VDRTID * & ids);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
		};


class UnqueuedNestedInputConnector : public BaseStreamingInputConnector
	{
	protected:
		IStreamingChainUnit * unit;

	public:
		UnqueuedNestedInputConnector(uint32 id,
											 IStreamingChainUnit * unit) : BaseStreamingInputConnector(id, unit)
			{
			this->unit = unit;
			}

		//
		// IStreamingInputConnector functions
		//
		virtual STFResult ReceivePacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamTagIDs(VDRTID * & ids);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};



///////////////////////////////////////////////////////////////////////////////
// Queued Input Connector Implementations
///////////////////////////////////////////////////////////////////////////////

class QueuedInputConnector : public BaseStreamingInputConnector
	{
#if _DEBUG
	friend class InputConnectorQueueStatus;
#endif
	private:
		IStreamingUnit * unit;

	protected:
		STFFixedQueue * queue;

		StreamingDataPacket * currentPacket;	/// The packet which is currently being processed

		volatile bool packetBounced;

	public:
		/// Constructor of queued Input Connector
		/// @param queueSize: how many data packets can be queued at max
		/// @param threshold: is obsolete and ignored
		/// @param id: connector ID
		/// @param unit: unit this connector belongs to
		QueuedInputConnector(uint32 queueSize,
									int32  threshold,
									uint32 id,
									IStreamingUnit * unit);
		~QueuedInputConnector(void);

		virtual uint32 GetType(void)
			{
			return BaseStreamingInputConnector::GetType() | VDR_STRCTF_QUEUE;
			}

		/// Function by which the owning unit can get a packet from the queue.
		/// The caller must return the packet to its origin after getting it with this function!
		STFResult DequeuePacket(StreamingDataPacket *& packet);
		
		//
		// IStreamingInputConnector functions
		//
		/// Called by Streaming Unit's thread
		virtual STFResult RequestPackets(void);
		virtual STFResult FlushPackets(void);
		virtual STFResult ReceivePacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamTagIDs(VDRTID * & ids);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};


class QueuedNestedInputConnector : public BaseStreamingInputConnector
	{
	private:
		IStreamingChainUnit * unit;

	protected:
		STFFixedQueue * queue;

		StreamingDataPacket * currentPacket;	/// The packet which is currently being processed

		volatile bool packetBounced;

	public:
		/// Constructor of nested queued Input Connector
		/// @param queueSize: how many data packets can be queued at max
		/// @param threshold: is obsolete and ignored
		/// @param id: connector ID
		/// @param unit: unit this connector belongs to
		QueuedNestedInputConnector(uint32 queueSize,
											int32  threshold,
											uint32 id,
											IStreamingChainUnit * unit);
		~QueuedNestedInputConnector(void);


		virtual uint32 GetType(void)
			{
			return BaseStreamingInputConnector::GetType() | VDR_STRCTF_QUEUE | VDR_STRCTF_PARENT;
			}


		/// Function by which the owning unit can get a packet from the queue.
		///	The caller must return the packet to its origin after getting it with this function!
		STFResult DequeuePacket(StreamingDataPacket *& packet);


		//
		// IStreamingInputConnector functions
		//

		/// Called by Streaming Unit's thread
		virtual STFResult RequestPackets(void);
		virtual STFResult FlushPackets(void);
		virtual STFResult ReceivePacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamTagIDs(VDRTID * & ids);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};


typedef QueuedNestedInputConnector * QueuedNestedInputConnectorPtr;

#if _DEBUG
class InputConnectorQueueStatus
	{
	class ConnectorNode : public STFNode
		{
		public:
			QueuedInputConnector * connector;
			
			ConnectorNode(QueuedInputConnector * connector)
				{
				this->connector = connector;
				};
		};
	
	STFList list;

	public:
		InputConnectorQueueStatus() {};

		STFResult AddInputConnector(QueuedInputConnector * connector) 
			{
			list.InsertLast(new ConnectorNode(connector));

			STFRES_RAISE_OK;
			}

		STFResult RemInputConnector(QueuedInputConnector * connector) 
			{
			STFIteratorHandle iter(list.CreateIterator());

			ConnectorNode * nextNode = NULL;
			while ( (nextNode = (ConnectorNode *) iter.Proceed()) != NULL )
				{
				if (nextNode->connector == connector)
					{
					list.Remove(nextNode);
					delete nextNode;

					STFRES_RAISE_OK;
					}
				}

			STFRES_RAISE_OK;
			}

		STFResult GetStatus() 
			{
			STFIteratorHandle iter(list.CreateIterator());
			ConnectorNode * nextNode = NULL;
			while ( (nextNode = (ConnectorNode *) iter.Proceed()) != NULL )
				{
				IBaseStreamingUnit * unit = NULL;
				nextNode->connector->GetStreamingUnit(unit);
				DP("Queue of unit %s, contains %x elements\n", (char*) unit->GetInformation(), nextNode->connector->queue->NumElements());
				}
			STFRES_RAISE_OK;
			};
	};

extern InputConnectorQueueStatus GlobalInputConnectorQueueStatus;
#endif


///////////////////////////////////////////////////////////////////////////////
// Standard Output Connector Implementations
///////////////////////////////////////////////////////////////////////////////

class BaseStreamingOutputConnector : public StreamingConnector,
												 public virtual IStreamingOutputConnector
	{
	protected:
		IStreamingInputConnector	*	target;
		STFInterlockedStack				packetStore;
		uint32	numPackets;
#if _DEBUG
		StreamingDataPacketPtr		*	totalPackets;
#endif
	public:
		BaseStreamingOutputConnector(uint32 numPackets,
											  uint32 id,
											  IBaseStreamingUnit * unit);

		~BaseStreamingOutputConnector(void);

		virtual uint32 GetType(void)
			{
			return VDR_STRCTF_OUTPUT;
			}

		// This function is called when a packet is returned to this output
		// connector, and the packet stack was empty. Overloading it can
		// be used to restart a processing that was blocked before by the
		// unavailability of packets in the packet store.
		virtual STFResult SignalPacketReturn(void) = 0;

		//
		// IStreamingConnector interface implementation
		//
		virtual STFResult Unplug(void);
		virtual STFResult GetLink(IStreamingConnector *& linkedWith);

		//
		// IStreamingOutputConnector interface implementation
		//
		virtual STFResult Plug(IStreamingInputConnector * connector);
 		virtual STFResult GetLink(IStreamingInputConnector *& linkedWith);

		virtual STFResult SendPacket(StreamingDataPacket * packet);
		virtual STFResult GetEmptyDataPacket(StreamingDataPacket *& packet);
		virtual STFResult ReturnDataPacket(StreamingDataPacket * packet);

		virtual STFResult GetStreamTagIDs(VDRTID * & ids);

		// Override from StreamingConnector
		virtual IStreamingOutputConnector * QueryOutputConnector(void);

		//
		// IStreamingDataPacketManager interface implementation
		//
#if _DEBUG
		// Debugging facilities
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
		virtual STFString GetInformation(void);
#endif
	};



/// Output Connector Implementation
///	An output connector connected to and outside streaming input connector
class StreamingOutputConnector : public BaseStreamingOutputConnector
	{
	private:
		IStreamingUnit * unit;

	protected:
		// Override from BaseStreamingOutputConnector
		virtual STFResult SignalPacketReturn(void);

	public:
		StreamingOutputConnector(uint32 numPackets,
										 uint32 id,
										 IStreamingUnit * unit) : BaseStreamingOutputConnector(numPackets, id, unit)
			{
			this->unit = unit;
			}

		//
		// IStreamingOutputConnector interface implementation
		//
		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
			{
#if _DEBUG // unavoidable because GetInformation does not exist in Release compile mode
			if (LOG_THIS_VDRMID(message))
				DP_UPSTREAM_NOTIFICATION("%s: (output) UpstreamNotification(%s)\n", (char *) this->unit->GetInformation(), DBG_VDRMID_STRING(message));
#endif
			// Forward the message to the Streaming Unit the connector belongs to. 
			// The unit will decide whatto do with the message (e.g. translate it into a message 
			// going further upstream or to its parent Streaming Unit (if there is one).
			STFRES_RAISE(unit->UpstreamNotification(id, message, param1, param2));
			}

		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator);

		virtual STFResult IsPushingChain(void);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};

typedef StreamingOutputConnector	*	StreamingOutputConnectorPtr;

/// Nested Output Connector
/// An Nested Output Connector belongs to a Streaming *Chain* Unit, and is the 
/// output connector connected to the input connector of an inside child Streaming
/// Unit of the Streaming Subchain.
class NestedOutputConnector : public BaseStreamingOutputConnector
	{
	private:
		IStreamingChainUnit * unit;

	protected:
		// Override from BaseStreamingOutputConnector
		virtual STFResult SignalPacketReturn(void);

	public:
		NestedOutputConnector(uint32 numPackets,
									 uint32 id,
									 IStreamingChainUnit * unit) : BaseStreamingOutputConnector(numPackets, id, unit)
			{
			this->unit = unit;
			}

		virtual uint32 GetType(void)
			{
			return BaseStreamingOutputConnector::GetType() | VDR_STRCTF_PARENT;
			}

		//
		// IStreamingOutputConnector interface implementation
		//
		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
			{
#if _DEBUG // unavoidable because GetInformation does not exist in Release compile mode
			if (LOG_THIS_VDRMID(message))
				DP_UPSTREAM_NOTIFICATION("%s: (nested output) NestedUpstreamNotification(%s)\n", (char *) this->unit->GetInformation(), DBG_VDRMID_STRING(message));
#endif
			// Forward the message to the Streaming *Chain* unit the connector belongs to.
			// The unit will decide what to do with the message (e.g. translate it into a message
			// going further upstream or to its parent Streaming Unit (if there is one).
			STFRES_RAISE(unit->NestedUpstreamNotification(id, message, param1, param2));
			}

		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator);

		virtual STFResult IsPushingChain(void);
#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};

typedef NestedOutputConnector * NestedOutputConnectorPtr;

#endif // #ifndef STREAMINGCONNECTOR_H
