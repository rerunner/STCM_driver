///
/// @file       VDR/Source/Streaming/StreamingUnit.cpp
///
/// @brief      VDR Streaming Unit Standard Implementation
///
/// @author     Stefan Herr
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Implementation File
///
/// @date       2003-01-08
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///


#include "StreamingUnit.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "STF/Interface/STFDebug.h"

#if 0
#define DPCMDS DPR
#else
#define DPCMDS while(0) DebugPrintEmpty
#endif

///////////////////////////////////////////////////////////////////////////////
//"StreamingPoolAllocator" Unit Implementation
///////////////////////////////////////////////////////////////////////////////

StreamingPoolAllocator::StreamingPoolAllocator(IStreamingUnit * unit, uint32 connectorID)
	{
	this->unit = unit;
	this->connectorID = connectorID;
	this->allocator = NULL;
	this->allocatorSink = NULL;
	}

StreamingPoolAllocator::~StreamingPoolAllocator(void)
	{
	if (allocatorSink)
		{
		allocatorSink->UnregisterMessageSink(this);
		allocatorSink->Release();
		}
	if (allocator)
		allocator->Release();
	}

STFResult StreamingPoolAllocator::SetAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	if (allocatorSink)
		{
		allocatorSink->UnregisterMessageSink(this);
		allocatorSink->Release();
		}
	if (this->allocator)
		this->allocator->Release();

	this->allocatorSink = NULL;
	this->allocator = NULL;

	if (allocator)
		{
		STFRES_REASSERT(allocator->QueryInterface(VDRIID_VDR_MESSAGE_SINK_REGISTRATION, (void*&) allocatorSink));

		this->allocator = allocator;
		allocator->AddRef();

		allocatorSink->RegisterMessageSink(this);
		}

	STFRES_RAISE_OK;
	}

STFResult StreamingPoolAllocator::GetMemoryBlocks (VDRMemoryBlock ** blocks, uint32 number, uint32 &done)
	{
	//lint --e{613}
	assert(allocator != NULL);
	STFRES_RAISE(allocator->GetMemoryBlocks(blocks, 0, number, done, unit));
	}

STFResult StreamingPoolAllocator::ReceiveMessage(STFMessage & message)
	{
	switch (message.message)
		{
		case VDRMID_MEMORY_POOL_ALLOCATOR_BLOCKS_AVAILABLE:
			unit->UpstreamNotification(connectorID, VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE, message.param1, message.param2);
			break;
		}

	message.Complete();

	STFRES_RAISE_OK;
	}

IStreamingUnit * StreamingPoolAllocator::GetAllocatorStreamingUnit(void)
	{
	return unit;
	}

///////////////////////////////////////////////////////////////////////////////
//"Streaming" Unit Implementation
///////////////////////////////////////////////////////////////////////////////

StreamingUnit::StreamingUnit()
	{
	numConnectors			= 0;
	
	totalConnectors		= 2;
	connectors				= new IStreamingConnectorPtr[totalConnectors];

	state						= VDR_STRMSTATE_IDLE;
	parentStreamingUnit	= NULL;
	}


StreamingUnit::~StreamingUnit()
	{
	delete[] connectors;
	}

void StreamingUnit::ReleaseDestruct(void)
	{
	uint32 i;

	for(i = 0; i < numConnectors; i++)
		{
		connectors[i]->Unplug();
		connectors[i] = NULL;
		}
	}

STFResult StreamingUnit::SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result)
	{
	// Notify parent of completion
	if (parentStreamingUnit)
		{
#if _DEBUG
		DPCMDS("Signal Complete cmd %d of %s, result: %08x\n", command, (char*)this->GetInformation(), result);
#endif
		STFRES_REASSERT(parentStreamingUnit->UpchainNotification(VDRMID_STRM_COMMAND_COMPLETED, command, result));
		}

	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	// Set new transitional state
	this->state = targetState;
	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	// Nothing to do in general version, so simply signal completion
	this->SignalStreamingCommandCompletion(command, STFRES_OK);

	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	// Set the final state
	this->state = targetState;
	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::GetState(VDRStreamingState & state)
	{
	state = this->state;

	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::AddConnector(IStreamingConnector * connector)
	{
	IStreamingConnectorPtr	*newConnectors;
	int newSize;
	uint32 i;

	if (numConnectors == totalConnectors)
		{
		// The array is too small. Allocate a new one with doubled size.
		newSize = totalConnectors * 2;
		newConnectors = new IStreamingConnectorPtr[newSize];
		if (newConnectors == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

		// Copy the array content.
		for (i = 0; i < numConnectors; i++)
			newConnectors[i] = connectors[i];

		// Replace the old array with the new one.
		delete[] connectors;
		connectors = newConnectors;
		totalConnectors = newSize;
		}

	// Add unit at the end.
	connectors[numConnectors] = connector;
	numConnectors++;

	STFRES_RAISE_OK;
	}


STFResult StreamingUnit::FindConnector(uint32 connectorID, IStreamingConnector *& connector)
	{
	if (numConnectors == 0)
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	if (connectorID > numConnectors - 1)
		STFRES_RAISE(STFRES_RANGE_VIOLATION);
	else
		connector = connectors[connectorID];

	STFRES_RAISE_OK;
	}

STFResult StreamingUnit::PropagateStreamingClock(IStreamingClock * streamingClock)
	{
	// We have nothing to do with the streaming clock, so we
	// simply ignore it...
	STFRES_RAISE_OK;
	}

STFResult StreamingUnit::SetParentStreamingUnit(IStreamingChainUnit * unit)
	{
	this->parentStreamingUnit = unit;
	STFRES_RAISE_OK;
	}

STFResult StreamingUnit::CompleteConnection(void)
	{
	// Nothing to do here...
	STFRES_RAISE_OK;
	}

STFResult StreamingUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	static const VDRTID	supportedTags[] =  {VDRTID_DONE};

	ids = (VDRTID *)supportedTags;

	STFRES_RAISE_OK;
	}

STFResult StreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	// This should not happen, as there is no external input connector
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	// This should not happen, as there is no external input connector
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	// This should not happen, as there is no external output connector
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	// This should not happen, as there is no external output connector
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

STFResult StreamingUnit::IsPushingChain(uint32 connectorID)
	{
	uint32								i;
	IStreamingInputConnector	*	connector;

	for(i=0; i<numConnectors; i++)
		{
		connector = connectors[i]->QueryInputConnector();
		if (connector)
			{
			if (connector->IsPushingChain() == STFRES_TRUE)
				return STFRES_TRUE;
			}
		}

	return STFRES_FALSE;
	}

#if _DEBUG

STFResult StreamingUnit::PrintDebugInfo (uint32 id)
	{
	DEBUGLOG(id, "StreamingUnit %08x : %s\n", this, (char*)this->GetInformation());
	STFRES_RAISE_OK;
	}

#endif


///////////////////////////////////////////////////////////////////////////////
// Virtual Streaming Unit Implementation
///////////////////////////////////////////////////////////////////////////////


STFResult VirtualStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}

void VirtualStreamingUnit::ReleaseDestruct(void)
	{
	StreamingUnit::ReleaseDestruct();
	VirtualUnit::ReleaseDestruct();
	}

///////////////////////////////////////////////////////////////////////////////
// Virtual Streaming Unit Collection Implementation
///////////////////////////////////////////////////////////////////////////////


void VirtualStreamingUnitCollection::ReleaseDestruct(void)
	{
	StreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

STFResult VirtualStreamingUnitCollection::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}



///////////////////////////////////////////////////////////////////////////////
// Streaming Chain Unit Implementation
///////////////////////////////////////////////////////////////////////////////


StreamingChainUnit::~StreamingChainUnit(void)
	{
	delete[] streamingSubUnits;
	}

void StreamingChainUnit::ReleaseDestruct(void)
	{
	int	i;

	for(i=0; i<numStreamingSubUnits; i++) 
		{
		streamingSubUnits[i]->Release();
		streamingSubUnits[i] = NULL;
		}
	}


STFResult StreamingChainUnit::GetState(VDRStreamingState & state)
	{
	state = VDR_STRMSTATE_IDLE;	// We fake the idleness

#if _DEBUG
	DP("!StreamingChainUnit::GetState() called after desctruction of Streaming Proxy Unit's vtables!\n");
#endif

	STFRES_RAISE_OK;
	}


STFResult StreamingChainUnit::StreamingCommandCompletedNotification(VDRStreamingCommand command, STFResult result)
	{
	// If this is the last one reporting command completion...
	if (--commandsIssued == 0)
		{
		// ...then signal completion upstream
		this->SignalStreamingCommandCompletion(command, result);
		}


	STFRES_RAISE_OK;
	}

//! Forward the prepare phase to all child units
STFResult StreamingChainUnit::ForwardPrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	int i;
	STFResult err = STFRES_OK;

	i = 0;
	while (i < numStreamingSubUnits && !STFRES_IS_ERROR(err))
		{
		err = streamingSubUnits[i]->PrepareStreamingCommand(command, param, targetState);
		i++;
		}

	STFRES_RAISE(err);
	}


//! Forward the begin phase to all child units
STFResult StreamingChainUnit::ForwardBeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	int i;
	STFResult err = STFRES_OK;																																						   

	i = 0;
	while (i < numStreamingSubUnits && !STFRES_IS_ERROR(err))
		{
#if _DEBUG
		DPCMDS("Begin cmd %d of %s\n", command, (char*)streamingSubUnits[i]->GetInformation());
#endif

		commandsIssued++;
		err = streamingSubUnits[i]->BeginStreamingCommand(command, param);
		i++;
		}

	if (STFRES_FAILED(err))
		commandsIssued--;

	STFRES_RAISE(err);
	}


//! Forward the complete phase to all child units
STFResult StreamingChainUnit::ForwardCompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	int i;
	STFResult err = STFRES_OK;

	i = 0;
	while (i < numStreamingSubUnits && !STFRES_IS_ERROR(err))
		{
#if _DEBUG
		DPCMDS("Forward Complete cmd %d of %s\n", command, (char*)streamingSubUnits[i]->GetInformation());
#endif

		err = streamingSubUnits[i]->CompleteStreamingCommand(command, targetState);
		i++;
		}

	STFRES_RAISE(err);
	}


STFResult StreamingChainUnit::CompleteSubUnitConnection(void)
	{
	int i;
	STFResult err = STFRES_OK;

	i = 0;
	while (i < numStreamingSubUnits && !STFRES_IS_ERROR(err))
		{
		err = streamingSubUnits[i]->CompleteConnection();
		i++;
		}

	STFRES_RAISE(err);
	}


STFResult StreamingChainUnit::PropagateSubUnitStreamingClock(IStreamingClock * streamingClock)
	{
	int i;
	STFResult err = STFRES_OK;

	i = 0;
	while (i < numStreamingSubUnits && !STFRES_IS_ERROR(err))
		{
		err = streamingSubUnits[i]->PropagateStreamingClock(streamingClock);
		i++;
		}

	STFRES_RAISE(err);
	}


STFResult StreamingChainUnit::PlugSubUnitConnectors(uint32 sourceUnitID, uint32 sourceConnectorID, uint32 destUnitID, uint32 destConnectorID)
	{
	IStreamingConnector			*	tempInConnector;
	IStreamingConnector			*	tempOutConnector;

	STFRES_REASSERT(streamingSubUnits[sourceUnitID]->FindConnector(sourceConnectorID, tempOutConnector));
	STFRES_REASSERT(streamingSubUnits[destUnitID]->FindConnector(destConnectorID, tempInConnector));

	STFRES_RAISE(PlugConnectors(tempInConnector, tempOutConnector));
	}


STFResult StreamingChainUnit::UnplugSubUnitConnectors(uint32 sourceUnitID, uint32 sourceConnectorID, uint32 destUnitID, uint32 destConnectorID)
	{
	IStreamingConnector	*	tempInConnector;
#if _DEBUG
	IStreamingConnector	*	tempOutConnector;
	IStreamingConnector	*	checkOutConnector;
#endif

	STFRES_REASSERT(streamingSubUnits[destUnitID]->FindConnector(destConnectorID, tempInConnector));

#if _DEBUG
	STFRES_REASSERT(streamingSubUnits[sourceUnitID]->FindConnector(sourceConnectorID, tempOutConnector));
	STFRES_REASSERT(tempInConnector->GetLink(checkOutConnector));
	assert(tempOutConnector == checkOutConnector);
#endif

	STFRES_RAISE(tempInConnector->Unplug());
	}

STFResult StreamingChainUnit::NestedGetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	static const VDRTID	supportedTags[] =  {VDRTID_DONE};

	ids = (VDRTID *)supportedTags;

	STFRES_RAISE_OK;
	}


STFResult StreamingChainUnit::NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}
	

STFResult StreamingChainUnit::NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet)
	{
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StreamingChainUnit::NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets)
	{
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StreamingChainUnit::NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

STFResult StreamingChainUnit::NestedIsPushingChain(uint32 nestedConnectorID)
	{
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

///////////////////////////////////////////////////////////////////////////////
// Virtual Streaming Chain Unit Implementation
///////////////////////////////////////////////////////////////////////////////

void VirtualStreamingChainUnit::ReleaseDestruct(void)
	{
	StreamingChainUnit::ReleaseDestruct();
	StreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

STFResult VirtualStreamingChainUnit::SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result)
	{
	STFRES_RAISE(StreamingUnit::SignalStreamingCommandCompletion(command, result));
	}


STFResult VirtualStreamingChainUnit::PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	// Set the new state
	this->state = targetState;

	// Forward to child units
	STFRES_RAISE(this->ForwardPrepareStreamingCommand(command, param, targetState));
	}


STFResult VirtualStreamingChainUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	STFResult	err = STFRES_OK;

	// Count "this" object in as a command processing unit
	commandsIssued++;

	// Forward the command phase to the child units
	err = this->ForwardBeginStreamingCommand(command, param);

	// Signal that this unit itself has finished
	this->StreamingCommandCompletedNotification(command, STFRES_OK);

	STFRES_RAISE(err);
	}


STFResult VirtualStreamingChainUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	STFResult	err = STFRES_OK;

	// Forward to child unit
	err = this->ForwardCompleteStreamingCommand(command, targetState);

	// Set final state
	this->state = targetState;

	STFRES_RAISE(err);
	}


STFResult VirtualStreamingChainUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT,			IStreamingUnit);
		VDRQI_IMPLEMENT(VDRIID_STREAMING_CHAIN_UNIT,	IStreamingChainUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingChainUnit::Initialize(void)
	{
	STFRES_REASSERT(VirtualUnitCollection::Initialize());

	// Register as parent Streaming Unit at all subunits
	for (int i = 0; i < numStreamingSubUnits; i++)
		streamingSubUnits[i]->SetParentStreamingUnit(this);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingChainUnit::CompleteConnection(void)
	{
	STFRES_RAISE(this->CompleteSubUnitConnection());
	}


STFResult VirtualStreamingChainUnit::PropagateStreamingClock(IStreamingClock * streamingClock)
	{
	STFRES_RAISE(PropagateSubUnitStreamingClock(streamingClock));
	}


//! Process messages from child units
STFResult VirtualStreamingChainUnit::UpchainNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	switch (message)
		{
		case VDRMID_STRM_COMMAND_COMPLETED:
			this->StreamingCommandCompletedNotification((VDRStreamingCommand)param1, param2);
			break;

		default:
			if (parentStreamingUnit)
				parentStreamingUnit->UpchainNotification(message, param1, param2);
			else
				DP("WARNING: Streaming Chain Unit without parent trying to send upchain notification!\n");
			break;
		}

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual Streaming Proxy Unit Implementation
///////////////////////////////////////////////////////////////////////////////


VirtualStreamingProxyUnit::VirtualStreamingProxyUnit(IPhysicalUnit * physicalUnit,
																	  int maxLeafUnits)
	: StreamingChainUnit(maxLeafUnits),
	  VirtualUnitCollection(physicalUnit, maxLeafUnits), 
	  MessageSinkRegistration(NULL)
	{
	state			= VDR_STRMSTATE_IDLE;
	direction	= 1;	// Forward
	speed			= 0;	// Paused
	}


VirtualStreamingProxyUnit::~VirtualStreamingProxyUnit()
	{
	}

void VirtualStreamingProxyUnit::ReleaseDestruct(void)
	{
	StreamingChainUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

STFResult VirtualStreamingProxyUnit::GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset)
	{
	STFRES_RAISE(masterClock->GetCurrentStreamTimeOffset(systemOffset));
	}


STFResult VirtualStreamingProxyUnit::SignalStreamingCommandCompletion(VDRStreamingCommand command, STFResult result)
	{
	// In case of an error during the command execution, we have to go to the terminated state
	if (STFRES_FAILED(result))
		{
		result = this->ForwardCompleteStreamingCommand(command, VDR_STRMSTATE_TERMINATED);
		state = VDR_STRMSTATE_TERMINATED;
		}
	else
		{
		// Otherwise, go to the new steady state (for all subunits and ourself)
		switch (command)
			{
			case VDR_STRMCMD_BEGIN:
				// Finally proceed to the <ready> state
				result = this->ForwardCompleteStreamingCommand(command, VDR_STRMSTATE_READY);
				state = VDR_STRMSTATE_READY;
				break;
			case VDR_STRMCMD_DO:
				// Finally proceed to the <streaming> state
				result = this->ForwardCompleteStreamingCommand(command, VDR_STRMSTATE_STREAMING);
				state = VDR_STRMSTATE_STREAMING;
				break;
			case VDR_STRMCMD_STEP:
				// Finally return to the <ready> state
				result = this->ForwardCompleteStreamingCommand(command, VDR_STRMSTATE_READY);
				state = VDR_STRMSTATE_READY;
				break;
			case VDR_STRMCMD_FLUSH:
				// Finally return to the <idle> state
				result = this->ForwardCompleteStreamingCommand(command, VDR_STRMSTATE_IDLE);
				state = VDR_STRMSTATE_IDLE;
				break;

			case VDR_STRMCMD_NONE:
				break;

			default:
				DP("*** Unhandled STRMCMD in VirtualStreamingProxyUnit::SignalStreamingCommandCompletion! ***\n");
			}
		}

	// Notify the application that the command has been completed.
	this->SendMessage(STFMessage(VDRMID_STRM_COMMAND_COMPLETED, command, result), false);

	STFRES_RAISE(result);
	}


/// This implements the Streaming State Machine
STFResult VirtualStreamingProxyUnit::SendCommand(VDRStreamingCommand command, int32 param)
	{
	STFAutoMutex	mutex(&proxyMutex);	// Blocks another thread sending commands if one is already being processed

	STFResult		result = STFRES_OK;
	bool				commandPrepared;

	// Only accept command if we are not already processing another (and thus are in a transitional state)
	if (state == VDR_STRMSTATE_STREAMING || state == VDR_STRMSTATE_IDLE || state == VDR_STRMSTATE_READY)
		{
		// Count this proxy in as a processing unit
		commandsIssued++;
		}
	else
		STFRES_RAISE(STFRES_PROCESSING_COMMAND);

	// Will be set to true if ForwardPrepareStreamingCommand is called for the command
	commandPrepared = false;

	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			// Validate parameter, either forward=1 or backward = -1
			if (param == VDR_STRMDIR_FORWARD || param == VDR_STRMDIR_BACKWARD)
				{
				switch (state)
					{
					case VDR_STRMSTATE_IDLE:
						// idle->preparing->ready
						state = VDR_STRMSTATE_PREPARING;
						commandPrepared = true;
						result = this->ForwardPrepareStreamingCommand(command, param, VDR_STRMSTATE_PREPARING);
						result = this->ForwardBeginStreamingCommand(command, param);
						break;

					case VDR_STRMSTATE_READY:
						// Check if direction has changed, if not this is a noop
						if (direction != param)
							{
							// ready->preparing->ready
							state = VDR_STRMSTATE_PREPARING;
							direction = param;
							commandPrepared = true;
							result = this->ForwardPrepareStreamingCommand(command, param, VDR_STRMSTATE_PREPARING);
							result = this->ForwardBeginStreamingCommand(command, param);
							}
						else
							{
							result = STFRES_OK;
							}
						break;

					case VDR_STRMSTATE_STREAMING:
						// streaming->stopping->ready
						state = VDR_STRMSTATE_STOPPING;
						commandPrepared = true;
						result = this->ForwardPrepareStreamingCommand(command, param, VDR_STRMSTATE_STOPPING);
						result = this->ForwardBeginStreamingCommand(command, param);
						break;

					default:
						result = STFRES_INVALID_STREAMING_STATE_FOR_COMMAND;
					}
				}
			else
				result = STFRES_INVALID_STREAMING_DIRECTION;
			break;

		case VDR_STRMCMD_DO:
			// Check validity of parameters, speed != 0 and sgn(speed) = direction
			if ((param ^ direction) >= 0 && param != 0)
				{
				switch (state)
					{
					case VDR_STRMSTATE_READY:
						// ready->starting->streaming
						state = VDR_STRMSTATE_STARTING;

						speed = param;

#if _DEBUG
						if (speed == 10000)
							{
							DP("################################################################################################\n");
							DP("### WARNING: Possible that playback speed of decimal 10000 specified instead of hex 0x10000! ###\n");
							DP("################################################################################################\n");
							}
#endif

						commandPrepared = true;
						result = this->ForwardPrepareStreamingCommand(command, speed, VDR_STRMSTATE_STARTING);

						// arm startup synchronisation clock
						masterClock->BeginStartupSequence(param);
						result = this->ForwardBeginStreamingCommand(command, speed);
						break;

					case VDR_STRMSTATE_STREAMING:
						// Check if speed has changed, if not, this is a noop
						if (speed != param)
							{
							// streaming->starting->streaming
							state = VDR_STRMSTATE_STARTING;
							commandPrepared = true;
							result = this->ForwardPrepareStreamingCommand(command, speed, VDR_STRMSTATE_STARTING);
							
							// arm startup synchronisation clock
							masterClock->BeginStartupSequence(param);
							speed = param; //kapil: fix for 1X-> 2X on the fly speed change
							result = this->ForwardBeginStreamingCommand(command, speed);
							}
						else
							{
							result = STFRES_OK;
							}
						break;

					default:
						result = STFRES_INVALID_STREAMING_STATE_FOR_COMMAND;
					}
				}
			else
				result = STFRES_INVALID_STREAMING_SPEED;
			break;

		case VDR_STRMCMD_STEP:
			// Check validity of parameters, sgn(duration) = direction
			if ((param ^ direction) >= 0)
				{
				if (state == VDR_STRMSTATE_READY)
					{
					// Check if the step size is nonzero, otherwise it is a noop
					if (param != 0)
						{
						// ready->stepping->ready
						state = VDR_STRMSTATE_STEPPING;
						commandPrepared = true;
						result = this->ForwardPrepareStreamingCommand(command, param, VDR_STRMSTATE_STEPPING);
						result = this->ForwardBeginStreamingCommand(command, param);
						}
					else
						{
						result = STFRES_OK;
						}
					}
				else
					result = STFRES_INVALID_STREAMING_STATE_FOR_COMMAND;
				}
			else
				result = STFRES_INVALID_STREAMING_STEPTIME;
			break;

		case VDR_STRMCMD_FLUSH:
			if (state == VDR_STRMSTATE_READY || state == VDR_STRMSTATE_STREAMING || state == VDR_STRMSTATE_IDLE)
				{
				// (ready|streaming)->flushing->idle
				state = VDR_STRMSTATE_FLUSHING;
				commandPrepared = true;
				result = this->ForwardPrepareStreamingCommand(command, param, VDR_STRMSTATE_FLUSHING);
				result = this->ForwardBeginStreamingCommand(command, param);
				}
#if 0
			else if (state == VDR_STRMSTATE_IDLE)
				{
				result = STFRES_OK;
				}
#endif
			else
				result = STFRES_INVALID_STREAMING_STATE_FOR_COMMAND;
			break;

		default:
			result = STFRES_INVALID_STREAMING_COMMAND;
		}

	// If the command was prepared, we must go the "normal" way of command completion
	if (commandPrepared)
		{
		// Notify command completion of this unit
		this->StreamingCommandCompletedNotification(command, result);
		}
	else	// otherwise the command completion message can immediately be sent
		{
		// The message is sent even if there was an error during the command execution
		commandsIssued--;
		this->SendMessage(STFMessage(VDRMID_STRM_COMMAND_COMPLETED, command, result), false);
		}

	STFRES_RAISE(result);
	}


STFResult VirtualStreamingProxyUnit::GetState(VDRStreamingState & state)
	{
	state = this->state;

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingProxyUnit::NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	switch (message)
		{
		case VDRMID_STRM_PACKET_REQUEST:

			// There is starvation over the inbound output connector that is delivering the notification.
			// Translate this into a packet request to the application.

#if _DEBUG
         {
			STFResult res = this->SendMessage(STFMessage(VDRMID_STRM_PACKET_REQUEST, nestedConnectorID, 0), false);
			if (res == STFRES_OBJECT_FULL)
				DP("VirtualStreamingProxyUnit %08x: Message Queue to application is full (VDRMID_STRM_PACKET_REQUEST)!\n", this->GetUnitID());
			STFRES_RAISE (res);
         }
#else
			STFRES_RAISE(this->SendMessage(STFMessage(VDRMID_STRM_PACKET_REQUEST, nestedConnectorID, 0), false));
#endif

		default:
			STFRES_RAISE(this->SendMessage(STFMessage(message, param1, param2), false));
		}
	}
	

STFResult VirtualStreamingProxyUnit::NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet)
	{
	// Signal the arrival of *one* data packet to the application
	STFRES_RAISE(this->SendMessage(STFMessage(VDRMID_STRM_PACKET_ARRIVAL, nestedConnectorID, 1), false));
	}


STFResult VirtualStreamingProxyUnit::NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets)
	{
	// Signal the arrival of probably several data packets to the application.
	STFRES_RAISE(this->SendMessage(STFMessage(VDRMID_STRM_PACKET_ARRIVAL, nestedConnectorID, numPackets), false));
	}


STFResult VirtualStreamingProxyUnit::UpchainNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	switch (message)
		{
		case VDRMID_STRM_COMMAND_COMPLETED:
			this->StreamingCommandCompletedNotification((VDRStreamingCommand)param1, param2);
			break;
		
		default:
			// Forward upchain messages to the application - e.g. state change and command completion notifications
			STFRES_RAISE(this->SendMessage(STFMessage(message, param1, param2), false));
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamingProxyUnit::NestedIsPushingChain(uint32 nestedConnectorID)
	{
	return STFRES_FALSE;
	}

STFResult VirtualStreamingProxyUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_VDR_STREAMING_PROXY_UNIT,		IVDRStreamingProxyUnit);
		VDRQI_IMPLEMENT(VDRIID_VDR_MESSAGE_SINK_REGISTRATION,	IVDRMessageSinkRegistration);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Physical Single Streaming Proxy Unit Implementation
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateSingleStreamingProxyUnit, PhysicalSingleStreamingProxyUnit)

STFResult PhysicalSingleStreamingProxyUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = new VirtualSingleStreamingProxyUnit(this);

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::Create(uint64 * createParams)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			messageDispatcherUnit = source;
			break;
		case 1:
			subStreamingUnit = source;
			break;
		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::GetTagIDs(VDRTID * & ids)
	{
	ids = NULL;
	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::InternalConfigureTags(TAG * tags)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalSingleStreamingProxyUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}

///////////////////////////////////////////////////////////////////////////////
// Virtual Single Streaming Proxy Unit Implementation
///////////////////////////////////////////////////////////////////////////////


VirtualSingleStreamingProxyUnit::VirtualSingleStreamingProxyUnit(PhysicalSingleStreamingProxyUnit * physicalUnit)
	: VirtualStreamingProxyUnit(physicalUnit, 2)
	{
	this->messageDispatcherUnit	=	NULL;
	this->physicalSingleProxy		=	physicalUnit;
	numInputConnectors				=	0;
	numOutputConnectors				=	0;
	allocators							=	NULL;
	inputConnectors					=	NULL;
	outputConnectors					=	NULL;
	masterClock							=	NULL;
	}


VirtualSingleStreamingProxyUnit::~VirtualSingleStreamingProxyUnit(void)
	{
	uint32 i;
	//lint --e{613}
	if (messageDispatcherUnit)
		messageDispatcherUnit->Release();

	if (masterClock)
		masterClock->Release();

	for (i = 0; i < numInputConnectors; i++)
		delete inputConnectors[i];

	for (i = 0; i < numOutputConnectors; i++)
		{
		delete outputConnectors[i];
		if (allocators[i]) allocators[i]->Release();
		}

	delete[] inputConnectors;
	delete[] outputConnectors;
	delete[] allocators;
	}


STFResult VirtualSingleStreamingProxyUnit::AllocateChildUnits(void)
	{
	IVirtualUnit			* tempVU;
	IStreamingUnit			* tempSUI;
	STFMessageDispatcher	* dispatcher;

	// Create virtual subunits

	STFRES_REASSERT(physicalSingleProxy->messageDispatcherUnit->CreateVirtual(tempVU, this, root ? root : this));
	STFRES_REASSERT(VirtualUnitCollection::AddLeafUnit(tempVU));	// Add to virtual unit collection
	STFRES_REASSERT(physicalSingleProxy->subStreamingUnit->CreateVirtual(tempVU, this, root ? root : this));
	STFRES_REASSERT(VirtualUnitCollection::AddLeafUnit(tempVU));	// Add to virtual unit collection

	// Get the Message Dispatcher Interface from the dispatcher unit

	STFRES_REASSERT(VirtualUnitCollection::leafUnits[0]->QueryInterface(VDRIID_MESSAGE_DISPATCHER, (void*&) messageDispatcherUnit));

	// Get the dispatcher from the dispatcher unit, and set our MessageSinkRegistration with it
	
	STFRES_REASSERT(messageDispatcherUnit->GetDispatcher(dispatcher));
	STFRES_REASSERT(SetDispatcher(dispatcher));


	// Get the Streaming Unit interface of the Streaming Unit for which "this" class is the proxy

	STFRES_REASSERT(VirtualUnitCollection::leafUnits[1]->QueryInterface(VDRIID_STREAMING_UNIT, (void*&) tempSUI));
	STFRES_REASSERT(StreamingChainUnit::AddStreamingSubUnit(tempSUI));	// Add to register of Streaming sub-units

	STFRES_RAISE_OK;
	}


STFResult VirtualSingleStreamingProxyUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualSingleStreamingProxyUnit::GetDataPackets(uint32 connectorID, VDRStreamingDataPacket * packets, 
																			 uint32 numPackets, uint32 & filledPackets)
	{
	STFResult err = STFRES_OK;
	StreamingDataPacket * tempPacket;
	uint32 i;

	filledPackets = 0;
	//lint --e{613}
	connectorID -= numOutputConnectors;

	if (connectorID < numInputConnectors)
		{
		i = 0;
		while (i < numPackets && !STFRES_IS_ERROR(err))
			{
			err = inputConnectors[connectorID]->DequeuePacket(tempPacket);
			if (!STFRES_IS_ERROR(err))
				{
				tempPacket->CopyToVDRPacket(&packets[i]);
				tempPacket->TransferRangesOwnership(NULL);	// NULL as owner because of transfer to application domain
				// Release the packet to its origin
				tempPacket->ReturnToOrigin();
				i++;
				}
			}

		// Enter the number of really copied packets
		filledPackets = i;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);	// The connector does not exist

	STFRES_RAISE_OK;
	}


STFResult VirtualSingleStreamingProxyUnit::RequestDataPackets(uint32 connectorID)
	{
	connectorID -= numOutputConnectors;

	if (connectorID < numInputConnectors)
		{
		STFRES_REASSERT(inputConnectors[connectorID]->RequestPackets());
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);	// The connector does not exist

	STFRES_RAISE_OK;
	}


/*
// This stuff is in here only temporarily for doing some quick measurements
// Do not take this as an example - it's not meant to be one :-)

#include <os21/task.h>
#include <os21/kernel.h>

static osclock_t sumProxyTime;
static uint32 numProxyCount = 0;
*/

STFResult VirtualSingleStreamingProxyUnit::DeliverDataPackets(uint32 connectorID, VDRStreamingDataPacket * packets,
																				  uint32 numPackets, uint32 & acceptedPackets)
	{
	STFResult err = STFRES_OK;
	StreamingDataPacket * tempPacket;
	//lint --e{613}
	acceptedPackets = 0;

/*
	task_status_t status;
	task_t *ptr;
*/	
	if (connectorID < numOutputConnectors)
		{
/*
		ptr = task_id();

		task_status(ptr, &status, 0);
		sumProxyTime -= status.task_time;
*/
		while (acceptedPackets < numPackets && !STFRES_IS_ERROR(err))
			{
			err = outputConnectors[connectorID]->GetEmptyDataPacket(tempPacket);
			if (!STFRES_IS_ERROR(err))
				{
				// Make copy of the VDRStreamingDataPacket into the StreamingDataPacket
				tempPacket->CopyFromVDRPacket(&packets[acceptedPackets]);

				// Transfer ownership of ranges to copied range:
				tempPacket->AddRefToRanges();

				// Send packet to output pin
				err = outputConnectors[connectorID]->SendPacket(tempPacket);

				if (!STFRES_IS_ERROR(err))
					{
					// Release the VDR Streaming Formatter's reference to the ranges
					for (int i = 0; i < packets[acceptedPackets].numRanges; i++)
						packets[acceptedPackets].tagRanges.ranges[packets[acceptedPackets].numTags + i].Release(NULL);	// Only NULL as holder possible 

					acceptedPackets++;
					}
				else
					{
					// Release ownership of ranges
					tempPacket->ReleaseRanges();
					// Packet must be returned again as the connector refused to accept it
					tempPacket->ReturnToOrigin();
					}
				}
			}

/*
		task_status(ptr, &status, 0);
		sumProxyTime += status.task_time;

		numProxyCount++;
		if (numProxyCount == 100)
			{
			DPR("### Proxy Time: %d\n", (sumProxyTime*100)/kernel_time());
			numProxyCount = 0;
			}
*/		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);	// The connector does not exist

	STFRES_RAISE(err);
	}


STFResult VirtualSingleStreamingProxyUnit::Initialize(void)
	{
	uint32 i;
	IStreamingConnector * tempConnector;
	STFResult res;
	uint32 inID;
	uint32 outID;
	uint32 connectorType;
	//lint --e{613}
	STFRES_REASSERT(VirtualUnitCollection::Initialize());

	masterClock = new StreamingClock();

	// Register as parent unit at the subunit
	STFRES_REASSERT(streamingSubUnits[0]->SetParentStreamingUnit(this));

	// Find out how many input and output connectors there are.
	i = 0;
	res = STFRES_OK;
	while (!STFRES_IS_ERROR(res))
		{
		res = streamingSubUnits[0]->FindConnector(i , tempConnector);

		if (STFRES_SUCCEEDED(res))
			{
			connectorType = tempConnector->GetType();

			if (connectorType & VDR_STRCTF_INPUT)
				{
				numOutputConnectors++;
				}
			else if (connectorType & VDR_STRCTF_OUTPUT)
				{
				numInputConnectors++;
				}

			i++;
			}
		}

	if (numInputConnectors > 0 || numOutputConnectors > 0)
		{
		// Allocate the input connectors.
		inputConnectors = NULL;
		if (numInputConnectors > 0)
			{
			inputConnectors = new QueuedNestedInputConnectorPtr[numInputConnectors];
			STFRES_ASSERT(inputConnectors != NULL, STFRES_NOT_ENOUGH_MEMORY);

			for(i=0; i<numInputConnectors; i++)
				inputConnectors[i] = NULL;
			}

		// Allocate the input connectors.
		outputConnectors	= NULL;
		allocators        = NULL;
		if (numOutputConnectors > 0)
			{
			outputConnectors	= new NestedOutputConnectorPtr[numOutputConnectors];
			allocators        = new IVDRMemoryPoolAllocatorPtr[numOutputConnectors];
			STFRES_ASSERT(outputConnectors != NULL  &&  allocators != NULL, STFRES_NOT_ENOUGH_MEMORY);

			for(i=0; i<numOutputConnectors; i++)
				{
				allocators[i] = NULL;
				outputConnectors[i] = NULL;
				}
			}

		i		= 0;
		inID	= 0;
		outID	= 0;
		res	= STFRES_OK;

		while (!STFRES_IS_ERROR(res))
			{
			res = streamingSubUnits[0]->FindConnector(i , tempConnector);

			if (STFRES_SUCCEEDED(res))
				{
				connectorType = tempConnector->GetType();

				if (connectorType & VDR_STRCTF_INPUT)
					{
					//??? It might be good to allow the number of packets to be programmable (TAGs...), instead of
					//    setting them fixed to 16.
					outputConnectors[outID] = new NestedOutputConnector(50, outID, this);

					res = PlugConnectors(tempConnector, outputConnectors[outID]);
									 
					outID++;
					}
				else if (connectorType & VDR_STRCTF_OUTPUT)
					{
					//??? Threshold and queue size should be programmable (TAGs...)
					inputConnectors[inID] = new QueuedNestedInputConnector(15, 1, inID, this);

					res = PlugConnectors(tempConnector, inputConnectors[inID]);

					inID++;
					}

				i++;
				}
			}
		}

	if (i == (numInputConnectors + numOutputConnectors))
		res = STFRES_OK;

	//
	// Now singal completion for the connection to all units in the
	// streaming graph.
	//
	if (res == STFRES_OK)
		{
		// Before the connection can be completed, the master Streaming Clock must have been distributed:
		STFRES_REASSERT(this->PropagateSubUnitStreamingClock(masterClock));
		STFRES_REASSERT(this->CompleteSubUnitConnection());
		}

	STFRES_RAISE(res);
	}


STFResult VirtualSingleStreamingProxyUnit::NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	allocators[nestedConnectorID] = allocator;
	allocator->AddRef();

	STFRES_RAISE_OK;
	}


STFResult VirtualSingleStreamingProxyUnit::ProvideAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	if (connectorID < numInputConnectors)
		{
		STFRES_REASSERT(inputConnectors[connectorID]->ProvideAllocator(allocator));
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);	// The connector does not exist

	STFRES_RAISE_OK;
	}


STFResult VirtualSingleStreamingProxyUnit::RequestAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * & allocator)
	{
	//lint --e{613}
	connectorID -= numInputConnectors;

	if (connectorID < numOutputConnectors)
		{
		allocator = allocators[connectorID];
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);	// The connector does not exist

	STFRES_RAISE_OK;
	}

#if _DEBUG

STFString VirtualSingleStreamingProxyUnit::GetInformation(void)
	{
	return STFString("VirtualSingleStreamingProxyUnit with ID: ") + STFString(GetUnitID(), 8, 16);
	}

STFResult VirtualSingleStreamingProxyUnit::PrintDebugInfo(uint32 id)
	{
	DEBUGLOG(id, "%s\n", (char*)this->GetInformation());
	STFRES_RAISE_OK;
	}

#endif
