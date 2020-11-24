///
/// @file       VDR/Source/Streaming/StreamingConnectors.cpp
///
/// @brief      VDR Streaming Connectors Standard Implementation
///
/// @author     Stefan Herr
///
/// @date       2003-01-08 
///
/// @par OWNER: VDR Architecture Team
///
/// @par SCOPE: INTERNAL Implementation File
///
/// VDR Streaming Connectors Standard Implementation
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "StreamingConnectors.h"

#include "STF/Interface/STFDebug.h"

#if _DEBUG
InputConnectorQueueStatus GlobalInputConnectorQueueStatus;
#endif

///////////////////////////////////////////////////////////////////////////////
// Generic Streaming Connector Implementation
///////////////////////////////////////////////////////////////////////////////

StreamingConnector::StreamingConnector(uint32 id, IBaseStreamingUnit * unit)
	{
	this->id				= id;
	this->unit			= unit;
	}


StreamingConnector::~StreamingConnector()
	{
	}


uint32 StreamingConnector::GetID(void)
	{
	return id;
	}

STFResult StreamingConnector::GetStreamingUnit(IBaseStreamingUnit *& unit)
	{
	// Typecast is allowed here
	unit = (IBaseStreamingUnit*) this->unit;
	// AddRef() here?
	STFRES_RAISE_OK;
	}

IStreamingInputConnector * StreamingConnector::QueryInputConnector(void)
	{
	return NULL;
	}

IStreamingOutputConnector * StreamingConnector::QueryOutputConnector(void)
	{
	return NULL;
	}


///////////////////////////////////////////////////////////////////////////////
// Base Input Connector Implementations
///////////////////////////////////////////////////////////////////////////////

BaseStreamingInputConnector::~BaseStreamingInputConnector(void)
	{
	assert(source == NULL);
	}

STFResult BaseStreamingInputConnector::Plug(IStreamingOutputConnector * connector)
	{
	VDRStreamingState curState;
	STFResult			res = STFRES_OK;

	// Check the state of the unit this connector belongs to
	STFRES_REASSERT(unit->GetState(curState));

	if (curState == VDR_STRMSTATE_IDLE)
		{
		if (source)
			{
			if (source != connector)
				{
				// We are already connected!
				STFRES_RAISE(STFRES_OBJECT_IN_USE);
				}
			else
				STFRES_RAISE_OK;
			}
		else
			{
			source = connector;
			res = source->Plug(this);
			if (STFRES_FAILED(res))
				source = NULL;
			}
		}
	else
		STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);	// Our Streaming unit is not in the idle state

	STFRES_RAISE(res);
	}

STFResult BaseStreamingInputConnector::Unplug(void)
	{
	VDRStreamingState					curState;
	IStreamingOutputConnector	*	tsource;

	// Check if we are connected, else just ignore the call
	if (source)
		{
		// Check the state of the unit this connector belongs to
		STFRES_REASSERT(unit->GetState(curState));
		if (curState == VDR_STRMSTATE_IDLE)
			{
			tsource = source;
			source = NULL;
			tsource->Unplug();
			}
		else
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);	// Our Streaming unit is not in the idle state
		}

	STFRES_RAISE_OK;
	}

STFResult BaseStreamingInputConnector::GetLink(IStreamingOutputConnector *& linkedWith)
	{
	linkedWith = source;

	STFRES_RAISE_OK;
	}

STFResult BaseStreamingInputConnector::GetLink(IStreamingConnector *& linkedWith)
	{
	linkedWith = source;

	STFRES_RAISE_OK;
	}

IStreamingInputConnector * BaseStreamingInputConnector::QueryInputConnector(void)
	{
	return this;
	}


/// Called from the Streaming Unit to request and retrieve packets
STFResult BaseStreamingInputConnector::RequestPackets(void)
	{
#if _DEBUG 
	// see StreamingDiagnostics.h, DEBUG_REQUEST_PACKETS, and DIAGNOSTIC_STREAM_MIXER_STATISTICS
	CONDITIONAL_REQPKT_DP("%s (input) RequestPackets from %s\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
	//lint --e{613}
	assert(source != NULL);
	STFRES_RAISE(source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0));
	}
		
STFResult BaseStreamingInputConnector::ProvideAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	assert(source != NULL);
	STFRES_RAISE(source->ReceiveAllocator(allocator));
	}

STFResult BaseStreamingInputConnector::IsPushingChain(void)
	{
	//lint --e{613}
	assert(source != NULL);
	STFRES_RAISE(source->IsPushingChain());
	}


///////////////////////////////////////////////////////////////////////////////
// Unqueued Input Connector
///////////////////////////////////////////////////////////////////////////////


STFResult UnqueuedInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{
	VDRStreamingState curState;

	STFRES_REASSERT(unit->GetState(curState));

	switch (curState)
		{
		case VDR_STRMSTATE_STOPPING:
		case VDR_STRMSTATE_PREPARING:
			// We are not accepting packets during stop and prepare, because we don't no
			// the requested sequence number of direction yet.  We simply claim that
			// we are full, so we might get the packet later.			
			STFRES_RAISE(STFRES_OBJECT_FULL);
			
		case VDR_STRMSTATE_FLUSHING:
			// In the case of flushing, we simply eat all incomming data until the chain
			// dries up.
			packet->ReleaseRanges();
			packet->ReturnToOrigin();

			STFRES_RAISE_OK;
			
		case VDR_STRMSTATE_IDLE:
			// We are not accepting any data in the idle state, so we fail with an invalid
			// state.
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);
			
		default:
			STFRES_RAISE(unit->ReceivePacket(id, packet));
		}
	}

STFResult UnqueuedInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(unit->GetStreamTagIDs(id, ids));
	}


///////////////////////////////////////////////////////////////////////////////
// Unqueued Nested Input Connector
///////////////////////////////////////////////////////////////////////////////


STFResult UnqueuedNestedInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{
	VDRStreamingState curState;

	STFRES_REASSERT(unit->GetState(curState));

	switch (curState)
		{
		case VDR_STRMSTATE_STOPPING:
		case VDR_STRMSTATE_PREPARING:
			// We are not accepting packets during stop and prepare, because we don't no
			// the requested sequence number of direction yet.  We simply claim that
			// we are full, so we might get the packet later.
			STFRES_RAISE(STFRES_OBJECT_FULL);
		
		case VDR_STRMSTATE_FLUSHING:
			// In the case of flushing, we simply eat all incomming data until the chain
			// dries up.
			packet->ReleaseRanges();
			packet->ReturnToOrigin();

			STFRES_RAISE_OK;
		
		case VDR_STRMSTATE_IDLE:
			// We are not accepting any data in the idle state, so we fail with an invalid
			// state.
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);
		
		default:
			STFRES_RAISE(unit->NestedReceivePacket(id, packet));
		}
	}

STFResult UnqueuedNestedInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(unit->NestedGetStreamTagIDs(id, ids));
	}


///////////////////////////////////////////////////////////////////////////////
// Queued Input Connector
///////////////////////////////////////////////////////////////////////////////

QueuedInputConnector::QueuedInputConnector(uint32 queueSize,
														 int32 threshold,
														 uint32 id,
														 IStreamingUnit * unit) 
	: BaseStreamingInputConnector(id, unit)
	{	
	DEBUG_EXECUTE(GlobalInputConnectorQueueStatus.AddInputConnector(this));
	
	this->unit = unit;
	queue	= new STFFixedQueue(queueSize);
	
	currentPacket	= NULL;
	packetBounced	= false;
	}

QueuedInputConnector::~QueuedInputConnector(void)
	{
	DEBUG_EXECUTE(GlobalInputConnectorQueueStatus.RemInputConnector(this));

	delete queue;
	}

STFResult QueuedInputConnector::DequeuePacket(StreamingDataPacket *& packet)
	{
	if (currentPacket == NULL && queue->IsEmpty())
		{
		packet = NULL;
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
		}
	else
		{
		if (currentPacket != NULL)
			{
			packet = currentPacket;
			packet->RemPacketOwner(unit);
			currentPacket = NULL;
			}
		else
			{
			STFRES_REASSERT(queue->Dequeue((void*&) packet));
			packet->RemPacketOwner(unit);
			}
		}

	STFRES_RAISE_OK;
	}

				
/// Called by Streaming Unit's thread
STFResult QueuedInputConnector::RequestPackets(void)
	{
	STFResult res = STFRES_OK;
	//lint --e{613}
	// First check if there is anything at all
	if (currentPacket == NULL && queue->IsEmpty())
		{
		// If there are no more packets and there was a bounce before, it is now
		// necessary to send a packet request upstream notification.
		if (packetBounced)
			{
#if _DEBUG
			CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets from %s (packetBounced,queue-empty)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
			packetBounced = false; // The order is important here: packetBounced must be set to false before issuing the packet request!
			source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
			}
#if _DEBUG
		else 
			CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets doing nothing (!%s) (!packetBounced,queue-empty)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif

		STFRES_RAISE_OK;
		}
	else
		{
		// Get top element, dequeue when no error
		while (!STFRES_IS_ERROR(res))
			{
			if (currentPacket == NULL)
				{
				res = queue->Dequeue((void*&)currentPacket);
#if _DEBUG
				CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets currentPacket=0x%0X\n", (char *) unit->GetInformation(), (int) currentPacket);
#endif

				if (res == STFRES_OBJECT_EMPTY)
					{
					// If the queue has become empty, we only should send a packet
					// request if a bounce happened before.
					if (packetBounced)
						{
#if _DEBUG
						CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets from %s (packetBounced,OBJECT_EMPTY)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
						packetBounced = false;
						source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
						}
#if _DEBUG
					else 
						CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets (!%s) (!packetBounced,OBJECT_EMPTY)\n", (char *) unit->GetInformation(), DebugUnitNameFromID(source->GetID()));
#endif
					STFRES_RAISE_OK;
					}
				else if (STFRES_IS_ERROR(res))
					{
#if _DEBUG
					CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets DeQueue returns 0x%0X\n", (char *) unit->GetInformation(), res);
#endif
					// An unexpected error happened - so we must return that
					STFRES_RAISE(res);
					}
				else if (packetBounced)
					{
					// If we get here, a packet bounced because the queue was full before.
					// It is required to send an upstream notification because there is now
					// room again in the queue (as the Dequeue() operation did not return an error)
#if _DEBUG
					CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets from %s (packetBounced,!OBJECT_EMPTY)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
					packetBounced = false;
					source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
					// Continue and send the current packet to the unit.
					}
#if _DEBUG
				else 
					CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets from %s (!packetBounced,!OBJECT_EMPTY)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
				}

#if _DEBUG
			CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets calling ReceivePacket\n", (char *) unit->GetInformation());
#endif
			currentPacket->RemPacketOwner(unit);
			res = unit->ReceivePacket(id, currentPacket);

			if (res != STFRES_OBJECT_FULL)
				currentPacket = NULL;
			else
				currentPacket->AddPacketOwner(unit);
			}
		}

#if _DEBUG
	if (STFRES_FAILED(res))
		CONDITIONAL_REQPKT_DP("%s (queuedinput) RequestPackets returns 0x%08X\n", (char *) unit->GetInformation(), res);
#endif

	STFRES_RAISE(res);
	}


STFResult QueuedInputConnector::FlushPackets(void)
	{
	StreamingDataPacket * tempPacket;

	if (currentPacket)
		{
		currentPacket->ReleaseRanges();
		currentPacket->RemPacketOwner(unit);
		currentPacket->ReturnToOrigin();
		currentPacket = NULL;
		}

	while (!STFRES_IS_ERROR(queue->Dequeue((void*&)tempPacket)))
		{
		tempPacket->ReleaseRanges();
		tempPacket->RemPacketOwner(unit);
		tempPacket->ReturnToOrigin();
		}
	
	STFRES_RAISE_OK;
	}


STFResult QueuedInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{
	STFResult res;

	VDRStreamingState curState;

	STFRES_REASSERT(unit->GetState(curState));

	switch (curState)
		{
		case VDR_STRMSTATE_STOPPING:
		case VDR_STRMSTATE_PREPARING:
			// We are not accepting packets during stop and prepare, because we don't no
			// the requested sequence number of direction yet.  We simply claim that
			// we are full, so we might get the packet later. We have also to remember
			// that we bounced the packet, so that we can later send a packet request 
			// upstream notification.
			packetBounced = true;
			STFRES_RAISE(STFRES_OBJECT_FULL);
		
		case VDR_STRMSTATE_FLUSHING:
			// In the case of flushing, we simply eat all incomming data until the chain
			// dries up.
			packet->ReleaseRanges();
			packet->ReturnToOrigin();

			STFRES_RAISE_OK;
			
		case VDR_STRMSTATE_IDLE:
			// We are not accepting any data in the idle state, so we fail with an invalid
			// state.
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);
			
		default:
			// First Add the new Owner and then enque it, otherwise
			// the possibilty of a racing condition releasing the owner before
			// adding it, is present.
			packet->AddPacketOwner(unit);
			if (STFRES_FAILED(queue->Enqueue(packet)))
				{				
				packet->RemPacketOwner(unit);
				// As the queue was full, we must remember the bouncing condition.
				packetBounced = true;
				STFRES_RAISE(STFRES_OBJECT_FULL);
				}

			//
			// A time discontinuity signals, that no packets can be expected in
			// the near future, so we have to try to kick off processing in the unit.
			// Other than that, we always try to signal the unit's thread if there
			// are still elements in the queue.
			//
			if (((packet->vdrPacket.flags & VDR_MSMF_TIME_DISCONTINUITY) != 0) || queue->NumElements() > 0)
				{
				res = unit->SignalPacketArrival(id, queue->NumElements());

				if (res == STFRES_OBJECT_FULL)
					res = STFRES_OK;

				STFRES_RAISE(res);
				}
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult QueuedInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(unit->GetStreamTagIDs(id, ids));
	}



///////////////////////////////////////////////////////////////////////////////
// Queued Nested Input Connector
///////////////////////////////////////////////////////////////////////////////

QueuedNestedInputConnector::QueuedNestedInputConnector(uint32 queueSize,
																			 int32 threshold,
																			 uint32 id,
																			 IStreamingChainUnit * unit)
	: BaseStreamingInputConnector(id, unit)
	{
	this->unit = unit;
	queue	= new STFFixedQueue(queueSize);
	
	currentPacket	= NULL;
	packetBounced	= false;
	}

QueuedNestedInputConnector::~QueuedNestedInputConnector(void)
	{
	if (queue)
	   delete queue;
	}


STFResult QueuedNestedInputConnector::DequeuePacket(StreamingDataPacket *& packet)
	{
	if (currentPacket == NULL && queue->IsEmpty())
		{
		packet = NULL;
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
		}
	else
		{
		if (currentPacket != NULL)
			{
			packet = currentPacket;
			currentPacket = NULL;
			}
		else
			{
			STFRES_REASSERT(queue->Dequeue((void*&) packet));
			}
		}

	STFRES_RAISE_OK;
	}

				
/// Called by Streaming Unit's thread
STFResult QueuedNestedInputConnector::RequestPackets(void)
	{
	STFResult res = STFRES_OK;
	//lint --e{613}
	// First check if there is anything at all
	if (currentPacket == NULL && queue->IsEmpty())
		{
		// If there are no more packets and there was a bounce before, it is now
		// necessary to send a packet request upstream notification.
		if (packetBounced)
			{
#if _DEBUG
			CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets from %s (packetBounced,queue-empty)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
			packetBounced = false;
			source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
			}
#if _DEBUG
		else 
			CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets (!%s) (!packetBounced,queue-empty)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif

		STFRES_RAISE_OK;
		}
	else
		{
		// Get top element, dequeue when no error
		while (!STFRES_IS_ERROR(res))
			{
			if (currentPacket == NULL)
				{
				res = queue->Dequeue((void*&)currentPacket);
#if _DEBUG
				CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets currentPacket=0x%0X\n", (char *) unit->GetInformation(), (int) currentPacket);
#endif

				if (res == STFRES_OBJECT_EMPTY)
					{
					// If the queue has become empty, we only should send a packet
					// request if a bounce happened before.
					if (packetBounced)
						{
#if _DEBUG
						CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets from %s (packetBounced,OBJECT_FULL)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
						packetBounced = false;
						source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
						}
#if _DEBUG
					else 
						CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets from %s (!packetBounced,OBJECT_FULL)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
					STFRES_RAISE_OK;
					}
				else if (STFRES_IS_ERROR(res))
					{
#if _DEBUG
					CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets Dequeue returns 0x%08X\n", (char *) unit->GetInformation(), res);
#endif
					// An unexpected error happened - so we must return that
					STFRES_RAISE(res);
					}
				else if (packetBounced)
					{
					// If we get here, a packet bounced because the queue was full before.
					// It is required to send an upstream notification because there is now
					// room again in the queue (as the Dequeue() operation did not return an error)
#if _DEBUG
					CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets from %s (packetBounced,!OBJECT_FULL)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
					packetBounced = false;
					source->UpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0);
					// Continue and send the current packet to the unit.
					}
#if _DEBUG
				else 
					CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets from %s (!packetBounced,!OBJECT_FULL)\n", (char *) unit->GetInformation(), (char *) source->GetInformation());
#endif
				}

#if _DEBUG
			CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets calling NestedReceivePacket\n", (char *) unit->GetInformation());
#endif
			res = unit->NestedReceivePacket(id, currentPacket);

			if (res != STFRES_OBJECT_FULL)
				currentPacket = NULL;
			else
				DP("Nested receive returned failure %x", res);
			}
		}

#if _DEBUG
	if (STFRES_FAILED(res))
		CONDITIONAL_REQPKT_DP("%s (queuednestedinput) RequestPackets returns 0x%08X\n", (char *) unit->GetInformation(), res);
#endif
	STFRES_RAISE(res);
	}


STFResult QueuedNestedInputConnector::FlushPackets(void)
	{
	StreamingDataPacket * tempPacket;

	if (currentPacket)
		{
		currentPacket->ReleaseRanges();
		currentPacket->ReturnToOrigin();
		currentPacket = NULL;
		}

	while (!STFRES_IS_ERROR(queue->Dequeue((void*&)tempPacket)))
		{
		tempPacket->ReleaseRanges();
		tempPacket->ReturnToOrigin();
		}
	
	STFRES_RAISE_OK;
	}


STFResult QueuedNestedInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{
	STFResult res;

	VDRStreamingState curState;

	STFRES_REASSERT(unit->GetState(curState));

	switch (curState)
		{
		case VDR_STRMSTATE_STOPPING:
		case VDR_STRMSTATE_PREPARING:
			// We are not accepting packets during stop and prepare, because we don't no
			// the requested sequence number of direction yet.  We simply claim that
			// we are full, so we might get the packet later. We have also to remember
			// that we bounced the packet, so that we can later send a packet request 
			// upstream notification.
			packetBounced = true;
			STFRES_RAISE(STFRES_OBJECT_FULL);
			
		case VDR_STRMSTATE_FLUSHING:
			// In the case of flushing, we simply eat all incomming data until the chain
			// dries up.
			packet->ReleaseRanges();
			packet->ReturnToOrigin();

			STFRES_RAISE_OK;
			
		case VDR_STRMSTATE_IDLE:
			// We are not accepting any data in the idle state, so we fail with an invalid
			// state.
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);
			
		default:
			// First Add the new Owner and then enque it, otherwise
			// the possibilty of a racing condition releasing the owner before
			// adding it, is present.
			if (STFRES_FAILED(queue->Enqueue(packet)))
				{
				// As the queue was full, we must remember the bouncing condition.
				packetBounced = true;
				STFRES_RAISE(STFRES_OBJECT_FULL);
				}

			//
			// A time discontinuity signals, that no packets can be expected in
			// the near future, so we have to try to kick off processing in the unit.
			// Other than that, we always try to signal the unit's thread if there
			// are still elements in the queue.
			//
			if (((packet->vdrPacket.flags & VDR_MSMF_TIME_DISCONTINUITY) != 0) || queue->NumElements() > 0)
				{
				res = unit->NestedSignalPacketArrival(id, queue->NumElements());

				if (res == STFRES_OBJECT_FULL)
					res = STFRES_OK;

				STFRES_RAISE(res);
				}
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult QueuedNestedInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(unit->NestedGetStreamTagIDs(id, ids));
	}



///////////////////////////////////////////////////////////////////////////////
// Base Output Connector
///////////////////////////////////////////////////////////////////////////////

STFResult BaseStreamingOutputConnector::Plug(IStreamingInputConnector * connector)
	{
	VDRStreamingState curState;
	STFResult			res = STFRES_OK;

	// Check the state of the unit this connector belongs to
	STFRES_REASSERT(unit->GetState(curState));

	if (curState == VDR_STRMSTATE_IDLE)
		{
		if (target)
			{
			if (target != connector)
				{
				// We are already connected!
				STFRES_RAISE(STFRES_OBJECT_IN_USE);
				}
			else
				STFRES_RAISE_OK;
			}
		else
			{
			target = connector;
			res = target->Plug(this);
			if (STFRES_FAILED(res))
				target = NULL;
			}
		}
	else
		STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);	// Our Streaming unit is not in the idle state

	STFRES_RAISE(res);
	}

STFResult BaseStreamingOutputConnector::Unplug(void)
	{
	VDRStreamingState					curState;
	IStreamingInputConnector	*	ttarget;

	// Check if we are connected, else just ignore the call
	if (target)
		{
		// Check the state of the unit this connector belongs to
		STFRES_REASSERT(unit->GetState(curState));
		if (curState == VDR_STRMSTATE_IDLE)
			{
			ttarget = target;
			target = NULL;
			ttarget->Unplug();
			}
		else
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);	// Our Streaming unit is not in the idle state
		}

	STFRES_RAISE_OK;
	}

STFResult BaseStreamingOutputConnector::GetLink(IStreamingInputConnector *& linkedWith)
	{
	linkedWith = target;

	STFRES_RAISE_OK;
	}

STFResult BaseStreamingOutputConnector::GetLink(IStreamingConnector *& linkedWith)
	{
	linkedWith = target;

	STFRES_RAISE_OK;
	}

BaseStreamingOutputConnector::BaseStreamingOutputConnector(uint32 numPackets,
																			  uint32 id,
																			  IBaseStreamingUnit * unit) : StreamingConnector(id, unit)
	{
	StreamingDataPacket * tempPacket;

	this->numPackets = numPackets;

#if _DEBUG
	if (numPackets <= 0)
		totalPackets = NULL;
	else
		totalPackets = new StreamingDataPacketPtr[numPackets];
#endif

	// numPackets could be 0 - in that case, we can only forward packets from
	// further upstream...
	for (uint32 i = 0; i < numPackets; i++)
		{
		tempPacket = new StreamingDataPacket(this);
#if _DEBUG
		totalPackets[i] = tempPacket;
#endif
		packetStore.Push(tempPacket);
		}

	target = NULL;
	}

BaseStreamingOutputConnector::~BaseStreamingOutputConnector(void)
	{
	StreamingDataPacket	*	tempPacket;
	assert(target == NULL);

#if _DEBUG
	int total, i;
	total = numPackets;
#endif

	while ((tempPacket = (StreamingDataPacket*) packetStore.Pop()))
		{
#if _DEBUG
		i = 0;
		while (i < total && totalPackets[i] != tempPacket)
			i++;
		assert(i < total);
		totalPackets[i] = NULL;
#endif

		tempPacket->Release();
		numPackets--;
		}

#if _DEBUG
	for(i=0; i<total; i++)
		assert(totalPackets[i] == NULL);

	delete[] totalPackets;
#endif
	assert(numPackets == 0);
	}

STFResult BaseStreamingOutputConnector::SendPacket(StreamingDataPacket * packet)
	{
	STFResult res;

	if (!target)
		res = STFRES_NOT_CONNECTED;
	else
		{
		// Send the packet to the attached input connector
		res = target->ReceivePacket(packet);
		}

	STFRES_RAISE(res);
	}


STFResult BaseStreamingOutputConnector::GetEmptyDataPacket(StreamingDataPacket *& packet)
	{
	VDRStreamingState					curState;

	STFRES_REASSERT(unit->GetState(curState));
	switch (curState)
		{
		case VDR_STRMSTATE_STOPPING:
		case VDR_STRMSTATE_PREPARING:
		case VDR_STRMSTATE_FLUSHING:
			packet = NULL;
			STFRES_RAISE(STFRES_OBJECT_EMPTY);
			
		case VDR_STRMSTATE_IDLE:
			packet = NULL;
			STFRES_RAISE(STFRES_ILLEGAL_STREAMING_STATE);
			
		default:
			// Get a packet from the packet store
			packet = (StreamingDataPacket*) packetStore.Pop();

			if (packet)
				STFRES_RAISE_OK;
			else
				{
				STFRES_RAISE(STFRES_OBJECT_EMPTY);
				}
		}
	}


STFResult BaseStreamingOutputConnector::ReturnDataPacket(StreamingDataPacket * packet)
	{
	bool storeWasEmpty = packetStore.Empty();

	// Put packet back into the packet store
	packetStore.Push(packet);

	// If the store was empty when te packet was returned, we
	// should signal the streaming unit about it, in case it
	// was blocked before due to the unavailability of an empty
	// packet.
	if (storeWasEmpty)
		STFRES_RAISE(SignalPacketReturn());
	
	STFRES_RAISE_OK;
	}

STFResult BaseStreamingOutputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	if (!target)
		STFRES_RAISE(STFRES_NOT_CONNECTED);
	else
		{
		// Send the packet to the attached input connector
		STFRES_REASSERT(target->GetStreamTagIDs(ids));
		}

	STFRES_RAISE_OK;
	}


IStreamingOutputConnector * BaseStreamingOutputConnector::QueryOutputConnector(void)
	{
	return this;
	}

///////////////////////////////////////////////////////////////////////////////
// Streaming Output Connector
///////////////////////////////////////////////////////////////////////////////

STFResult StreamingOutputConnector::SignalPacketReturn(void)
	{
	STFRES_RAISE(unit->UpstreamNotification(id, VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE, 0, 0));
	}


STFResult StreamingOutputConnector::ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(unit->ReceiveAllocator(id, allocator));
	}

STFResult StreamingOutputConnector::IsPushingChain(void)
	{
	STFRES_RAISE(unit->IsPushingChain(id));
	}


///////////////////////////////////////////////////////////////////////////////
// Nested Output Connector
///////////////////////////////////////////////////////////////////////////////

STFResult NestedOutputConnector::SignalPacketReturn(void)
	{
	STFRES_RAISE(unit->NestedUpstreamNotification(id, VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE, 0, 0));
	}

STFResult NestedOutputConnector::ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(unit->NestedReceiveAllocator(id, allocator));
	}
		
STFResult NestedOutputConnector::IsPushingChain(void)
	{
	STFRES_RAISE(unit->NestedIsPushingChain(id));
	}


///////////////////////////////////////////////////////////////////////////////
// Diagnostic functions
///////////////////////////////////////////////////////////////////////////////
#if _DEBUG
STFResult BaseStreamingOutputConnector::PrintDebugInfo(uint32 id)
	{
	STFString name = unit->GetInformation();
	char buf[200];
	name.Get(buf, 200);
	DEBUGLOG(id, "Output connector with id %d belonging to unit %s\n",id ,buf);

	STFRES_RAISE_OK;
	}

STFString StreamingConnector::GetInformation(void)
	{
	return unit->GetInformation() + STFString("Connector") + STFString(this->GetID());
	}

STFString BaseStreamingInputConnector::GetInformation(void)
	{
	return unit->GetInformation() + STFString("InputConnector") + STFString(this->GetID());
	}

STFString UnqueuedInputConnector::GetInformation(void)
	{
	return unit->GetInformation() + STFString("UnqueuedInputConnector") + STFString(this->GetID());
	}

STFString UnqueuedNestedInputConnector::GetInformation(void)
	{
	return unit->GetInformation() + STFString("UnqueuedNestedInputConnector") + STFString(this->GetID());
	}

STFString QueuedInputConnector::GetInformation(void)
	{
	char buf[100];
	uint8 empty = this->queue->IsEmpty();
	uint8 pktbounced = this->packetBounced;
	sprintf(buf, "QInConn of unit %s pktBounced %d empty %d numElem %d curPkt %x\n", (char *)unit->GetInformation(),pktbounced, empty, this->queue->NumElements(), (uint32) currentPacket); 
	return STFString(buf);
	//return unit->GetInformation() + STFString("QueuedInputConnector") + STFString(this->GetID());
	}

STFString QueuedNestedInputConnector::GetInformation(void)
	{
	char buf[100];
	uint8 empty = this->queue->IsEmpty();
	uint8 pktbounced = this->packetBounced;
	sprintf(buf, "QNestInConn of unit %s pktBounced %d empty %d numElem %d\n", (char *)unit->GetInformation(), pktbounced, empty, this->queue->NumElements()); 
	return STFString(buf);
	//return unit->GetInformation() + STFString("QueuedNestedInputConnector") + STFString(this->GetID());
	}

STFString BaseStreamingOutputConnector::GetInformation(void)
	{
	STFString str = this->target->GetInformation();

	return (unit->GetInformation() + STFString("OpConn") + STFString(this->GetID()) + str);
	}

STFString StreamingOutputConnector::GetInformation(void)
	{
	STFString str = this->target->GetInformation();
	return (unit->GetInformation() + STFString("OpConn") + STFString(this->GetID()) + str);
	}

STFString NestedOutputConnector::GetInformation(void)
	{
	STFString str = this->target->GetInformation();
	return (unit->GetInformation() + STFString("NstOpConn") + STFString(this->GetID()) + str);
	}
#endif // _DEBUG

