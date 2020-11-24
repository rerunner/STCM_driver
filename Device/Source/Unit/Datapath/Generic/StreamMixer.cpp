///
/// @brief      Generic Stream Mixer Implementation
///

#include "StreamMixer.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

#define ENABLE_RESYNC						1 // this is a "temp" ha ha flag which will allow us to disable (=0) resyncing 

#define DEBUG_DUMP_START_TIMES			0
#define DEBUG_DUMP_RENDER_TIMES			0
#define DEBUG_DUMP_STREAMING_CONTROL	0
#define DEBUG_DUMP_STREAM_SYNC			0
#define DEBUG_DUMP_MIX_THREAD				0
#define DEBUG_DUMP_INPUT_PACKETS			0
#define DEBUG_DUMP_MIXER_SLEEP_STATES	0
#define USE_TIME_FRACT_MUL					1	// unused
// gbg added 5/27/04 on suggestion by chz.  This will increas the number of empty frame we mix.  Allow extra time to 
// perform the sync.
#define USE_LONGER_FRAME_START			0


#if DEBUG_DUMP_START_TIMES
#define DPST DPR
#else
#define DPST DP_EMPTY
#endif

#if DEBUG_DUMP_RENDER_TIMES
#define DPRT DPR
#else
#define DPRT DP_EMPTY
#endif

#if DEBUG_DUMP_STREAMING_CONTROL
#define DPCNTRL DPR
#else
#define DPCNTRL DP_EMPTY
#endif

#if DEBUG_DUMP_STREAM_SYNC
#define DPSYNC DPR
#else
#define DPSYNC DP_EMPTY
#endif

#if DEBUG_DUMP_MIX_THREAD
#define DPMT DPR
#else
#define DPMT DP_EMPTY
#endif

#if DEBUG_DUMP_INPUT_PACKETS
#define DPIPCK DPR
#else
#define DPIPCK DP_EMPTY
#endif

#if DIAGNOSTIC_STREAM_MIXER_STATISTICS 
// see VDR/Source/Streaming/StreamingDiagnostics.h
// global data, for advanced diagnostics related to stream mixer starvation
DiagnosticStreamMixerRuntimeDatabase diagnosticStreamMixerDatabase;
#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS


///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input and Output Nodes
///////////////////////////////////////////////////////////////////////////////

StreamMixerInputNode::StreamMixerInputNode(void)
	{
	allocator				= NULL;
	inputSink				= NULL;
	startStreamTime		= STFHiPrec64BitTime(0);
	startFrameNumber		= INFINITE_FRAME_NUMBER;
	frameNumber				= INFINITE_FRAME_NUMBER;
	speed						= 0x00010000;
	commandStop				= false;
	commandResynch			= false;
	commandPrepare			= false;
	starvation				= false;
	packetRequest			= false;
	configurePending		= false;
	packetBounced			= true;
	startupState			= MIXSS_INITIAL;
	notificationTail		= 0;
	notificationHead		= 0;
	receivedStreamFrames	= 0;
	}


uint32 StreamMixerInputNode::AvailNotificationQueue(void)
	{
	return STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE - (notificationHead - notificationTail);
	}


/// Insert a pending notification into the queue
STFResult StreamMixerInputNode::InsertPendingNotification(VDRMID message, const STFHiPrec64BitTime & dueTime, uint32 param1, uint32 param2)
	{
	// If this assertion fails STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE should be increased	
	assert((notificationHead - notificationTail) < STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE);

//	if (this->inputType == MIT_VIDEO)
//		DEBUGLOG(LOGID_EPG, "Inserting notification 0x%08x group no.%i for time %i ms into queue.\n", message, param1, dueTime.Get32BitTime(STFTU_MILLISECS));

	if ((notificationHead - notificationTail) < STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE)
		{
		uint32 notificationHeadIndex = notificationHead & STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK;
		pendingNotifications[notificationHeadIndex].dueTime = dueTime;
		pendingNotifications[notificationHeadIndex].message.message = message;
		pendingNotifications[notificationHeadIndex].message.param1 = param1;
		pendingNotifications[notificationHeadIndex].message.param2 = param2;
		notificationHead++;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_FULL);

	STFRES_RAISE_OK;
	}

STFResult StreamMixerInputNode::GetFirstDueMessage(const STFHiPrec64BitTime & time, STFMessage & message)
	{
	bool messageValid;

	if (notificationHead == notificationTail)
		STFRES_RAISE(STFRES_OBJECT_EMPTY);

	if (this->direction == MID_BACKWARD)
		messageValid = pendingNotifications[notificationTail & STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK].dueTime >= time;
	else
		messageValid = pendingNotifications[notificationTail & STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK].dueTime <= time;

	if (messageValid)
		{
		message = pendingNotifications[notificationTail & STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK].message;
//		if (this->inputType == MIT_VIDEO)
//			DEBUGLOG(LOGID_EPG, "Sending notification 0x%08x group no.%i for time %i ms into queue.\n", message.message, message.param1, pendingNotifications[notificationTail & STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK].dueTime.Get32BitTime(STFTU_MILLISECS));
		notificationTail++;
		}
	else
		{
//		if (this->inputType == MIT_VIDEO)
//			DEBUGLOG(LOGID_EPG, "No notification found for time %i ms.\n", time.Get32BitTime(STFTU_MILLISECS));

		STFRES_RAISE(STFRES_OBJECT_EMPTY);
		}

	STFRES_RAISE_OK;
	}

STFResult StreamMixerInputNode::ClearNotificationQueue()
	{
	notificationHead = notificationTail = 0;
	STFRES_RAISE_OK;
	}

StreamMixerOutputNode::StreamMixerOutputNode(void)
	{
	firstOutputPacket	= false;	// Due to the Stream Mixer implementation, it is not really required 
										// to initialize this variable (overlayed by "streaming")
	commandFlush		= false;
	commandPrepare		= false;
	streaming			= false;
	}


///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input Control Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateStreamMixerInputControl, StreamMixerInputControl)


STFResult StreamMixerInputControl::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamMixerInputControl(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInputControl::Create(uint64 * createParams)
	{
	STFRES_RAISE_OK;
	}


STFResult StreamMixerInputControl::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			physicalMixerInput = source;
			break;
		
		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInputControl::GetTagIDs (VDRTID * & ids)
	{
	STFRES_RAISE(physicalMixerInput->GetTagIDs(ids));
	}



STFResult StreamMixerInputControl::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerInputControl::InternalConfigureTags (TAG * tags)
	{
	physicalInput->physicalMixerInput->ConfigureTags(tags);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerInputControl::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}






///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateStreamMixerInput, StreamMixerInput)


StreamMixerInput::~StreamMixerInput()
	{
	if (mixer)
		mixer->Release();

	if (messageSink)
		delete messageSink;
	}


STFResult StreamMixerInput::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamMixerInput(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInput::Create(uint64 * createParams)
	{
	uint32 threadPriority;
	uint32 threadStackSize;
	char	 *threadName;

	STFRES_REASSERT(GetDWordParameter(createParams, 0, threadPriority));
	STFRES_REASSERT(GetDWordParameter(createParams, 1, threadStackSize));
	STFRES_REASSERT(GetStringParameter(createParams,2, threadName));
	
	STFRES_REASSERT(GetDWordParameter(createParams, 3, (uint32&)this->inputType));

	dispatcher = new TriggeredWaitableQueuedSTFMessageProcessingDispatcher(32);
	threadedDispatcher = new ThreadedSTFMessageDispatcher(STFString(threadName) + STFString(GetUnitID(), 8, 16),
																		   threadStackSize,
																			(STFThreadPriority)threadPriority, 
																			dispatcher);
	messageSink = new MixerInputMessageSink(this, threadedDispatcher);
		
	// Optional parameters

	// Set default values for optional parameters
	basePriority	= 1;
	trickPriority	= 1;
	useAllocator	= true;
	freeParameter	= 0;

	// Change values for optional parameters if available

	if (GetNumberOfParameters(createParams) >= 5)
		{
		uint32 useAllocatorLocal;
		STFRES_REASSERT(GetDWordParameter(createParams, 4, useAllocatorLocal));

		useAllocator = (useAllocatorLocal == 1) ? true : false;
		}

	if (GetNumberOfParameters(createParams) >= 7)
		{
		STFRES_REASSERT(GetDWordParameter(createParams, 5, basePriority));
		STFRES_REASSERT(GetDWordParameter(createParams, 6, trickPriority));
		}

	// Additional anonymous parameter going to the Stream Mixer
	if (GetNumberOfParameters(createParams) >= 8)
		{
		STFRES_REASSERT(GetDWordParameter(createParams, 7, freeParameter));
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInput::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			STFRES_REASSERT(source->QueryInterface(VDRIID_STREAM_MIXER, (void*&) mixer));
			break;
		
		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInput::Initialize(uint64 * depUnitsParams)
	{
	// Register this input at the Stream Mixer
	STFRES_RAISE(mixer->RegisterMixerInput(this, messageSink, inputType, freeParameter, inputID));
	}


STFResult StreamMixerInput::SetVirtualStreamMixerInput(VirtualStreamMixerInput * unit)
	{
	STFResult		result;
	IVirtualUnit * tempVU;

	if (useAllocator && allocator)
		{
		STFRES_REASSERT(allocator->QueryInterface(VDRIID_VIRTUAL_UNIT, (void *&)tempVU));

		if (curVirtualInput)
			{
			if (STFRES_FAILED(result = curVirtualInput->ReceiveAllocator(NULL)))
				{
				DP("*** WARNING: StreamMixerInput::SetVirtualStreamMixerInput NULL allocator reception failed! Input ID: %08x\n", GetUnitID());
				}
			
			curVirtualInput = NULL;
			
			if (STFRES_FAILED(result = tempVU->Passivate()))
				{
				SystemTimer->WaitDuration(STFLoPrec32BitDuration(100, STFTU_MILLISECS));

				if (STFRES_FAILED(result = tempVU->Passivate()))
					{
					DP("*** WARNING: StreamMixerInput::SetVirtualStreamMixerInput passivation failed! Input ID: %08x\n", GetUnitID());
					tempVU->Release();
					STFRES_RAISE(result);
					}
				}
			}

		// When a virtual unit is activated, we provide the allocator.
		if (unit)
			{
			curVirtualInput = unit;

			if (STFRES_FAILED(result = curVirtualInput->ReceiveAllocator(allocator)))
				{
				DP("*** WARNING: StreamMixerInput::SetVirtualStreamMixerInput allocator reception failed! Input ID: %08x\n", GetUnitID());
				tempVU->Release();
				STFRES_RAISE(result);
				}

			if (STFRES_FAILED(result = tempVU->ActivateAndLock(VDRUALF_FOREGROUND_PRIORITY|VDRUALF_WAIT, STFHiPrec64BitTime(), STFLoPrec32BitDuration())))
				{
				DP("*** WARNING: StreamMixerInput::SetVirtualStreamMixerInput Activate And Lock failed! Input ID: %08x\n", GetUnitID());
				tempVU->Release();
				STFRES_RAISE(result);
				}
			}

		tempVU->Release();
		}
	else
		curVirtualInput = unit;

	STFRES_RAISE_OK;
	}


STFResult StreamMixerInput::ReceiveMessage(STFMessage & message)
	{
	// Forward the message to the currently active virtual Stream Mixer Input unit
	if (curVirtualInput)
		STFRES_RAISE(curVirtualInput->MixerNotification(message.message, message.param1, message.param2));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}


STFResult StreamMixerInput::ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	this->allocator = allocator;
	
	STFRES_RAISE_OK;
	}


STFResult StreamMixerInput::GetTagIDs (VDRTID * & ids)
	{
	STFRES_RAISE(mixer->GetStreamTagIDs(inputID, ids));
	}

STFResult StreamMixerInput::InternalBeginConfigure()
	{
	STFRES_RAISE(mixer->BeginConfigureStream(inputID));
	}

// do this in the physical since it is coming from the input control which is virtualizing for us
STFResult StreamMixerInput::InternalConfigureTags (TAG * tags)
	{
	STFRES_RAISE(mixer->ConfigureStreamTags(inputID, tags));
	}

STFResult StreamMixerInput::InternalCompleteConfigure()
	{
	STFRES_RAISE(mixer->CompleteConfigureStream(inputID));
	}





///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Input Streaming Unit
///////////////////////////////////////////////////////////////////////////////


STFResult VirtualStreamMixerInput::CompleteConnection(void)
	{
	if (streamingClock)
		{
		STFRES_REASSERT(streamingClock->RegisterClient(this, clockID));
		DPR("Registered Mixer Input Unit ID 0x%08x at Streaming Clock 0x%08x -> clockID: %d inputID: %d type: %d\n", GetUnitID(), streamingClock, clockID, physicalInput->inputID, physicalInput->inputType);
		}

	STFRES_RAISE(VirtualStreamingUnit::CompleteConnection());
	}


STFResult VirtualStreamMixerInput::PreemptUnit (uint32 flags)
	{
	STFRES_REASSERT(VirtualStreamingUnit::PreemptUnit (flags));

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		STFRES_REASSERT(physicalInput->SetVirtualStreamMixerInput(NULL));
		}

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		STFRES_REASSERT(physicalInput->SetVirtualStreamMixerInput(this));
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerInput::PropagateStreamingClock(IStreamingClock * streamingClock)
	{
	this->streamingClock = streamingClock;

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerInput::SetStartupFrame(uint32 frameNumber, const STFHiPrec64BitTime & startTime)
	{
	STFResult result;

	// pendingParam is the speed
	result = physicalInput->mixer->StartStream(physicalInput->inputID, frameNumber, pendingParam, startTime);

	DPST("Setting startup frame to %d for client %d, speed: %08x\n", frameNumber, clockID, pendingParam);	
	
	this->SignalStreamingCommandCompletion(VDR_STRMCMD_DO, result);

	STFRES_RAISE(result);
	}


STFResult VirtualStreamMixerInput::GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset)
	{
	STFRES_RAISE(physicalInput->mixer->GetStreamTimeOffset(physicalInput->inputID, systemOffset));
	}

STFResult VirtualStreamMixerInput::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	STFRES_RAISE(physicalInput->mixer->GetStreamTagIDs(physicalInput->inputID, ids));
	}

STFResult VirtualStreamMixerInput::MixerNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	StreamingClockClientStartupInfo	clientInfo;
	STFHiPrec64BitDuration				systemOffset, reqSystemOffset;

	switch (message)
		{
		case VDRMID_STRM_MIXER_STOPPED:
			if (state == VDR_STRMSTATE_STOPPING)
				{
				STFRES_REASSERT(physicalInput->mixer->PrepareStream(physicalInput->inputID, pendingParam));
				}
			else if (state == VDR_STRMSTATE_STARTING)
				{
				STFRES_REASSERT(physicalInput->mixer->GetStreamStartupInfo(physicalInput->inputID, clientInfo));
				// DP("Setting startup delay for clockID: %d\n", clockID);
				STFRES_REASSERT(streamingClock->SetStartupDelay(clockID, clientInfo));
				}
			else if (state == VDR_STRMSTATE_FLUSHING)
				{
				STFRES_REASSERT(physicalInput->mixer->FlushStream(physicalInput->inputID, pendingParam));
				}
			break;
		case VDRMID_STRM_MIXER_PREPARED:
			STFRES_REASSERT(this->SignalStreamingCommandCompletion(VDR_STRMCMD_BEGIN, STFRES_OK));
			break;
		case VDRMID_STRM_MIXER_STEPPED:
			STFRES_REASSERT(this->SignalStreamingCommandCompletion(VDR_STRMCMD_STEP, param1));
			break;
		case VDRMID_STRM_MIXER_FLUSHED:
			STFRES_REASSERT(this->SignalStreamingCommandCompletion(VDR_STRMCMD_FLUSH, STFRES_OK));
			break;

		case VDRMID_STRM_MIXER_PACKET_REQUEST:
		case VDRMID_STRM_MIXER_STARVATION:
			STFRES_REASSERT(inputConnector.RequestPackets());
			break;

		case VDRMID_STRM_MIXER_START_POSSIBLE:
			STFRES_REASSERT(inputConnector.SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0));
			break;

		case VDRMID_STRM_MIXER_START_REQUIRED:
			STFRES_REASSERT(inputConnector.SendUpstreamNotification(VDRMID_STRM_START_REQUIRED, 0, 0));
			break;

		case VDRMID_STRM_MIXER_SYNCH_REQUEST:
			if (state == VDR_STRMSTATE_STREAMING || state == VDR_STRMSTATE_STEPPING)
				{
			STFRES_REASSERT(physicalInput->mixer->GetStreamTimeOffset(physicalInput->inputID, systemOffset));
			STFRES_REASSERT(streamingClock->SynchronizeClient(clockID, 
											(direction == VDR_STRMDIR_FORWARD && speed == 0x10000 && state == VDR_STRMSTATE_STREAMING) ? physicalInput->basePriority : physicalInput->trickPriority, 
															systemOffset, reqSystemOffset));

// *** as long as the resync mechanism is not debugged, we keep it switched off ***
// Note: the audio system offset got +15ms glitches. This is disturbing video and has to be fixed at first
#if ENABLE_RESYNC
			if (systemOffset != reqSystemOffset)
#else
			if (false)
#endif
				{
				STFRES_REASSERT(physicalInput->mixer->SetStreamTimeOffset(physicalInput->inputID, reqSystemOffset));
//				if ((reqSystemOffset - systemOffset).GetAbsoluteDuration().Get32BitDuration() > 20)
//					DP("%d: %d\n", physicalInput->inputID, (reqSystemOffset - systemOffset).Get32BitDuration());
				switch (physicalInput->inputID)
					{
					case 0:
						DPSYNC("%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 1:
						DPSYNC("						%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 2:
						DPSYNC("												%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 3:
						DPSYNC("																		%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					default:
						DPSYNC("																								%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
					}
				}
			else
				{
				switch (physicalInput->inputID)
					{
					case 0:
						DPSYNC("%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 1:
						DPSYNC("						%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 2:
						DPSYNC("												%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					case 3:
						DPSYNC("																		%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
						break;
					default:
						DPSYNC("																								%d (%d): %u\n", clockID, physicalInput->inputID, (systemOffset).Get32BitDuration());
					}
				}
				}
			break;

		case VDRMID_STRM_SEGMENT_START:
		case VDRMID_STRM_SEGMENT_END:
		case VDRMID_STRM_SEGMENT_START_TIME:
		case VDRMID_STRM_GROUP_END:
		case VDRMID_STRM_GROUP_START:
			STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
			break;
		}

	STFRES_RAISE_OK;
	}



STFResult VirtualStreamMixerInput::PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	this->previousState = this->state;
	this->state = targetState;
	STFRES_RAISE_OK;
	}					 


STFResult VirtualStreamMixerInput::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	StreamingClockClientStartupInfo	clientInfo;

	DPCNTRL("vMxI:BeginStreamingCommand command %d 0x%x\n", command, param);

	pendingCommand = command;
	pendingParam = param;

	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			direction = param;
			if (state == VDR_STRMSTATE_STOPPING)
				STFRES_REASSERT(physicalInput->mixer->StopStream(physicalInput->inputID));
			else
				{
				STFRES_REASSERT(physicalInput->mixer->PrepareStream(physicalInput->inputID, param));
				}
			break;
		case VDR_STRMCMD_DO:
			speed = param;
			if (previousState == VDR_STRMSTATE_STREAMING)
				STFRES_REASSERT(physicalInput->mixer->StopStream(physicalInput->inputID));
			else
				{
				STFRES_REASSERT(physicalInput->mixer->GetStreamStartupInfo(physicalInput->inputID, clientInfo));
				// DP("Setting startup delay for clockID: %d\n", clockID);
				STFRES_REASSERT(streamingClock->SetStartupDelay(clockID, clientInfo));
				}
			break;
		case VDR_STRMCMD_STEP:
			STFRES_REASSERT(physicalInput->mixer->StepStream(physicalInput->inputID, param));
			break;
		case VDR_STRMCMD_FLUSH:
			if (previousState == VDR_STRMSTATE_STREAMING)
				STFRES_REASSERT(physicalInput->mixer->StopStream(physicalInput->inputID));
			else
				STFRES_REASSERT(physicalInput->mixer->FlushStream(physicalInput->inputID, param));
			break;

		case VDR_STRMCMD_NONE:
			break;

		default:
			DP("*** Unhandled STRMCMD in VirtualStreamMixerInput::BeginStreamingCommand! ***\n");
			
		}

	STFRES_RAISE_OK;
	}
		
STFResult VirtualStreamMixerInput::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{ 	
	STFRES_REASSERT(VirtualStreamingUnit::CompleteStreamingCommand(command, targetState));
	if (this->state == VDR_STRMSTATE_READY)
		{
		STFRES_REASSERT(inputConnector.RequestPackets());
		}

	STFRES_RAISE_OK;
	}



STFResult VirtualStreamMixerInput::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFResult res = STFRES_OK;
	res = physicalInput->mixer->ReceivePacket(physicalInput->inputID, packet);
	if (res == STFRES_OBJECT_FULL)
		{
		// DEBUG ONLY. SHOULD NOT BE CHECKED IN !
		//DP("receive packet returened object full!\n");		
		res = physicalInput->mixer->ReceivePacket(physicalInput->inputID, packet);	
		}
	
	STFRES_RAISE(res);
	}


STFResult VirtualStreamMixerInput::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateStreamMixerOutput, StreamMixerOutput)


STFResult StreamMixerOutput::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamMixerOutput(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult StreamMixerOutput::Create(uint64 * createParams)
	{
	uint32						numPackets;
	StreamingDataPacket *	tempPacket;

	if (GetNumberOfParameters(createParams) >= 1)
		{
		STFRES_REASSERT(GetDWordParameter(createParams, 0, numPackets));
		}
	else
		numPackets = 16;	// Default value

	for (uint32 i = 0; i < numPackets; i++)
		{
		tempPacket = new StreamingDataPacket(this);
		packetStore.Push(tempPacket);
		}

	STFRES_RAISE_OK;
	}


StreamMixerOutput::~StreamMixerOutput()
	{
	StreamingDataPacket * tempPacket;

	while ((tempPacket = (StreamingDataPacket*) packetStore.Pop()) != NULL)
		{
		delete tempPacket;
		}

	if (streamMixer)
		streamMixer->Release();
	}


STFResult StreamMixerOutput::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			STFRES_REASSERT(source->QueryInterface(VDRIID_STREAM_MIXER, (void*&) streamMixer));
			break;

		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixerOutput::Initialize(uint64 * depUnitsParams)
	{
	// Register this output at the Stream Mixer
	STFRES_RAISE(streamMixer->RegisterMixerOutput(this, outputID));
	}


STFResult StreamMixerOutput::GetEmptyDataPacket(StreamingDataPacket *& packet)
	{
	// Get a packet from the packet store
	packet = (StreamingDataPacket*) packetStore.Pop();

	if (packet)
		STFRES_RAISE_OK;
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}


STFResult StreamMixerOutput::ReturnDataPacket(StreamingDataPacket * packet)
	{
	// Put packet back into the packet store
	packetStore.Push(packet);
	
	STFRES_RAISE_OK;
	}


STFResult StreamMixerOutput::SendPacket(StreamingDataPacket * packet)
	{
	// New data is arriving here. We can immediately discard it if there is either
	// no virtual unit active and locked or the output is muted ("hardware mute")

	if (currentUnit)
		{
		// We switch to the curOutputUnit, as this pointer should be valid now
		STFRES_RAISE(curOutputUnit->ReceivePacket(0, packet));
		}
	else
		{
		// Burn the packets and data ranges in the NULL device
		packet->ReleaseRanges();
		packet->ReturnToOrigin();
		}

	STFRES_RAISE_OK;
	}

STFResult StreamMixerOutput::MixerNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	if (currentUnit)
		STFRES_RAISE(curOutputUnit->MixerNotification(message, param1, param2));
	else
		STFRES_RAISE_OK;
	}

#if _DEBUG
STFResult StreamMixerOutput::PrintDebugInfo(uint32 id)
	{
	DEBUGLOG(id, "StreamMixerOutput with outputID %d, belonging to unit --get from IStreamMixer later--\n", outputID);
	STFRES_RAISE_OK;
	}
#endif


///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

VirtualStreamMixerOutput::VirtualStreamMixerOutput(StreamMixerOutput * physicalOutput)
	: VirtualStreamingUnit(physicalOutput),
	  pcmOutputConnector(16, 0, this),
	  outputPoolAllocator(this, 0)
	{ 
	this->physicalOutput = physicalOutput;

	// Add external connector to StreamingUnit connector list
	AddConnector((IStreamingConnector*)&pcmOutputConnector);		// => ID 0
	}


STFResult VirtualStreamMixerOutput::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerOutput::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	// We fake a bit here. Even though there is no "real" input connector, we receive packets
	// coming from our physical unit here.

	// Forward them to the next unit in the chain...
	STFRES_RAISE(pcmOutputConnector.SendPacket(packet));
	}


STFResult VirtualStreamMixerOutput::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerOutput::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	switch (message)
		{
		case VDRMID_STRM_START_POSSIBLE:
		case VDRMID_STRM_START_REQUIRED:
			// Forward streaming startup messages directly up the chain, so that the application can
			// send the DO command when receiving these messages in the READY state
			// The application should not send the DO command before receiving one of these messages.
			STFRES_RAISE(parentStreamingUnit->UpchainNotification(message, param1, param2));
			break;

		case VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE:
			DPMT("mxO blks available\n");
		case VDRMID_STRM_PACKET_REQUEST:
			// If we get a packet request, we forward it to the Mixer, so that it can do something about it.
			STFRES_RAISE(physicalOutput->streamMixer->MixerOutputNotification(physicalOutput->outputID, VDRMID_STRM_PACKET_REQUEST, 0, 0));
			break;

		// Some filtering...
		case VDRMID_STRM_SEGMENT_START:
		case VDRMID_STRM_SEGMENT_START_TIME:
			STFRES_RAISE(physicalOutput->streamMixer->MixerOutputNotification(physicalOutput->outputID, message, param1, param2));
			break;

		case VDRMID_STRM_GROUP_START:
		case VDRMID_STRM_GROUP_END:
			STFRES_RAISE(physicalOutput->streamMixer->MixerOutputNotification(physicalOutput->outputID, message, param1, param2));
			break;

		case VDRMID_STRM_DATA_DISCONTINUITY_PROCESSED:
			break;

		default:
			DP("### Getting unhandled mixer output msg: %x\n", message);
			break;
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerOutput::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	// Register the allocator as StreamingPoolAllocator
	outputPoolAllocator.SetAllocator(allocator);

	// Forward to the output channel of the PCM Mixer
	STFRES_REASSERT(physicalOutput->streamMixer->ReceiveAllocator(physicalOutput->outputID, allocator));

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamMixerOutput::PreemptUnit(uint32 flags)
	{
	STFRES_REASSERT(VirtualStreamingUnit::PreemptUnit(flags));

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		physicalOutput->SetVirtualStreamMixerOutput(NULL);
		}

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		physicalOutput->SetVirtualStreamMixerOutput(this);
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamMixerOutput::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	DPCNTRL("vMxO BeginStreamingCommand command %d, param 0x%x\n", command, param);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			STFRES_REASSERT(physicalOutput->streamMixer->PrepareMixerOutput(physicalOutput->outputID));
			break;
		case VDR_STRMCMD_DO:
			STFRES_REASSERT(VirtualStreamingUnit::BeginStreamingCommand(command, param));
			break;
		case VDR_STRMCMD_STEP:
			STFRES_REASSERT(VirtualStreamingUnit::BeginStreamingCommand(command, param));
			break;
		case VDR_STRMCMD_FLUSH:
			STFRES_REASSERT(physicalOutput->streamMixer->FlushMixerOutput(physicalOutput->outputID));
			break;
		default:
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamMixerOutput::MixerNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	switch (message)
		{
		case VDRMID_STRM_MIXER_PREPARED:
			STFRES_REASSERT(this->SignalStreamingCommandCompletion(VDR_STRMCMD_BEGIN, STFRES_OK));
			break;
		case VDRMID_STRM_MIXER_FLUSHED:
			STFRES_REASSERT(this->SignalStreamingCommandCompletion(VDR_STRMCMD_FLUSH, STFRES_OK));
			break;
		}

	STFRES_RAISE_OK;
	}

///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Unit
///////////////////////////////////////////////////////////////////////////////


UNIT_CREATION_FUNCTION(CreateStreamMixer, StreamMixer)


StreamMixer::StreamMixer(VDRUID unitID) : ExclusivePhysicalUnit(unitID),
														STFThread(STFString(""))
	{
	physicalFrameMixer	= NULL;
	virtualFrameMixer		= NULL;

	frameMixer				= NULL;

	registeredOutputs		= 0;
	streamingOutputs		= 0;

	totalOutputs			= 2;
	mixerOutputs			= new StreamMixerOutputNode[totalOutputs];
	outputPackets			= new StreamingDataPacket* [totalOutputs];

	for (uint32 i = 0; i < totalOutputs; i++)
		outputPackets[i] = NULL;
	
	registeredInputs		= 0;
	mixerFrameDuration	= STFHiPrec64BitDuration();	// Dummy init - will be properly set in GetStreamStartupInfo()
	mixerFrameNum			= 0;
	configWriteLock		= false;
	configReadLock			= false;
	configStackLevel		= 0;

	outputSegmentNumber	= 0;

	// This must be reset whenver output is stopped and then restarted again(???)
	renderStartFrame		= 0;
	lastNum = 0;

#if DIAGNOSTIC_STREAM_MIXER_STATISTICS
	// global data, for advanced diagnostics related to stream mixer starvation
	(void) diagnosticStreamMixerDatabase.AddStreamMixer(unitID, &diagnosticStats); // see StreamingDiagnostics.h
#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS
	}


StreamMixer::~StreamMixer()
	{
	if (virtualFrameMixer)
		{
		virtualFrameMixer->Passivate();
		virtualFrameMixer->Release();
		}

	if (frameMixer)
		frameMixer->Release();

	delete[] mixerOutputs;
	delete[] outputPackets;
	}


STFResult StreamMixer::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAM_MIXER, IStreamMixer);
	VDRQI_END(ExclusivePhysicalUnit);

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::NotifyThreadTermination(void)
	{
	SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamMixer(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::Create(uint64 * createParams)
	{
	uint32 threadPriority;
	uint32 threadStackSize;
	char	 *threadName;

	STFRES_REASSERT(this->GetDWordParameter(createParams, 0, threadPriority));
	STFRES_REASSERT(this->GetDWordParameter(createParams, 1, threadStackSize));
	STFRES_REASSERT(this->GetStringParameter(createParams,2, threadName));
						 
	STFRES_REASSERT(this->SetThreadPriority((STFThreadPriority) threadPriority));
	STFRES_REASSERT(this->SetThreadStackSize(threadStackSize));
	STFRES_REASSERT(this->SetThreadName(STFString(threadName) + STFString(GetUnitID(), 8, 16)));

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			this->physicalFrameMixer = source;
			break;
		
		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::Initialize(uint64 * depUnitsParams)
	{
	STFHiPrec64BitTime currentTime;

	STFRES_REASSERT(physicalFrameMixer->CreateVirtual(virtualFrameMixer));

	STFRES_REASSERT(virtualFrameMixer->QueryInterface(VDRIID_FRAME_MIXER, (void*&) frameMixer));

	// Take current time for immediate activation
	SystemTimer->GetTime(currentTime);
	STFRES_REASSERT(virtualFrameMixer->ActivateAndLock(VDRUALF_REALTIME_PRIORITY, currentTime, STFHiPrec32BitDuration(0)));

	// Start the thread
	STFRES_REASSERT(this->StartThread());

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::GetTagIDs(VDRTID * & ids)
	{
	// For now we proxy the call to the specific frame mixer. We could of course add tag type IDs of the generic
	// StreamMixer here, too (if there were any).
	STFRES_RAISE(virtualFrameMixer->GetTagIDs(ids));
	}


// Called by the producer of the render frame time. By writing it twice to different
// variables, the consumer (running on a different thread) can check if a render frame
// time is really valid: This is the case when frame0 == frame1 or frame 1 == frame 2 -
// see GetRenderFrameTime() below.
STFResult StreamMixer::SetRenderFrameTime(STFHiPrec64BitTime renderTime, uint32 renderFrame)
	{
#if DEBUG_DUMP_RENDER_TIMES
	STFHiPrec64BitDuration	frameDuration;
	frameMixer->GetFrameDuration(frameDuration);
	DPRT("0x%08x SetRenderFrameTime: renderTime %d - renderFrame %d startTime %d\n", this, renderTime.Get32BitTime(), renderFrame, (renderTime - frameDuration * renderFrame).Get32BitTime());
#endif

	asyncRenderFrameTime.frame0	= renderFrame;
	asyncRenderFrameTime.time0		= renderTime;
	asyncRenderFrameTime.frame1	= renderFrame;
	asyncRenderFrameTime.time1		= renderTime;
	asyncRenderFrameTime.frame2	= renderFrame;

	// Forward this information to the frame mixer
	STFRES_RAISE(frameMixer->SetRendererInformation(renderTime, renderFrame));
	}


// This is a mutex replacement designed to avoid priority inversion. The reader of
// the render Frame time can check if a value pair is "steady", or if it is still being
// written by the producing thread.
STFResult StreamMixer::GetRenderFrameTime(STFHiPrec64BitTime & renderTime, uint32 & renderFrame)
	{
	ASyncRenderFrameTime	localRenderFrameTime;

	for(;;)
		{
		localRenderFrameTime = asyncRenderFrameTime;
		if (localRenderFrameTime.frame2 == localRenderFrameTime.frame1)
			{
			renderTime = localRenderFrameTime.time1;
			renderFrame = localRenderFrameTime.frame1;

			break;
			}
		else if (localRenderFrameTime.frame1 == localRenderFrameTime.frame0)
			{
			renderTime = localRenderFrameTime.time0;
			renderFrame = localRenderFrameTime.frame0;

			break;
			}
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::RegisterMixerInput(IStreamMixerInput * input,
														STFMessageSink * inputSink,
														VDRMixerInputType inputType,
														uint32 freeParameter,
														uint32 & inputID)
	{
	StreamMixerInputNode	*	node;

	inputID = registeredInputs++;
	
	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));
	
	node->input			= input;
	node->inputSink	= inputSink;

	STFRES_REASSERT(node->SetInputType(inputType));
	STFRES_REASSERT(node->SetFreeParameter(freeParameter));

	// In a final version, preparation of the mixing could be deferred to a later point in time
	// (e.g. when "starting" the mixer)
	STFRES_REASSERT(frameMixer->PrepareStream(inputID));

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::RegisterMixerOutput(IStreamMixerOutput * output, uint32 & outputID)
	{
	StreamMixerOutputNode *newOutputs;
	uint32 newSize;
	uint32 i;

	if (registeredOutputs == totalOutputs)
		{
		// The array is too small. Allocate a new one with doubled size.
		newSize = totalOutputs * 2;
		newOutputs = new StreamMixerOutputNode[newSize];
		if (newOutputs == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

		// Copy the array content.
		for (i = 0; i < registeredOutputs; i++)
			newOutputs[i] = mixerOutputs[i];

		// Replace the old array with the new one.
		delete[] mixerOutputs;
		delete[] outputPackets;
		outputPackets = new StreamingDataPacket*[newSize];
		for (i = 0; i < newSize; i++)
			outputPackets[i] = NULL;

		mixerOutputs = newOutputs;
		totalOutputs = newSize;
		}

	// Add unit at the end.
	mixerOutputs[registeredOutputs].output = output;

	outputID = registeredOutputs++;

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::ReceiveAllocator(uint32 outputID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(frameMixer->ReceiveAllocator(outputID, allocator));
	}

STFResult StreamMixer::PrepareMixerOutput(uint32 outputID)
	{
	mixerOutputs[outputID].commandPrepare = true;
	this->SetThreadSignal();

	STFRES_RAISE_OK;
	}

STFResult StreamMixer::FlushMixerOutput(uint32 outputID)
	{
	mixerOutputs[outputID].commandFlush = true;
	this->SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::BeginConfigureStream(uint32 inputID)
	{
	//
	// Mutex prevents multiple threads to perform a parallel deliver
	//
	configMutex.Enter();

	//
	// Check if we are the first in stack...
	//
	if (configStackLevel++ == 0)
		{
		//
		// Set the write lock, to prevent parallel update
		//
		configWriteLock = true;
		//
		// Check, if we actually got the lock
		//
		while (configReadLock)
			{
			//
			// If not, we relinquish the lock, and wait for a signal that
			// the readLock may have been removed
			//
			configWriteLock = false;
			configSignal.WaitSignal();

			//
			// Retry the write lock
			//
			configWriteLock = true;
			}
		}

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::GetStreamTagIDs (uint32 inputID, VDRTID * & ids)
	{
	STFRES_RAISE(frameMixer->GetStreamTagIDs(inputID, ids));
	}


STFResult StreamMixer::ConfigureStreamTags(uint32 inputID, TAG * tags)
	{
	STFRES_RAISE(frameMixer->ConfigureStreamTags(inputID, tags));
	}


STFResult StreamMixer::InternalUpdateStreamTags(uint32 inputID)
	{
	STFRES_RAISE(frameMixer->InternalUpdateStreamTags(inputID));
	}


STFResult StreamMixer::CompleteConfigureStream(uint32 inputID)
	{
	StreamMixerInputNode	*	node;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	//
	// If this is concluding the stack
	//
	if (--configStackLevel == 0)
		{
		//
		// Set pending configure flag
		//
		node->configurePending = true;

		//
		// Release config write lock
		//
		configWriteLock = false;
		}

	//
	// Release configure for other threads
	//
	configMutex.Leave();

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::GetStreamStartupInfo(uint32 inputID, StreamingClockClientStartupInfo & info)
	{
	StreamMixerInputNode	*	node;
	STFHiPrec64BitTime		renderFrameTime;
	uint32						renderFrameNum;

	// Get the duration of one mixer frame from the frame mixer, which is used for all further timing calculations.
	STFRES_REASSERT(frameMixer->GetFrameDuration(mixerFrameDuration));

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	STFRES_REASSERT(this->GetRenderFrameTime(renderFrameTime, renderFrameNum));

	info.renderFrameDuration	= mixerFrameDuration;
#if USE_LONGER_FRAME_START
	info.nextRenderFrameNumber	= mixerFrameNum + 100 / mixerFrameDuration.Get32BitDuration(); // perhaps even +1 is OK
#else
	info.nextRenderFrameNumber	= mixerFrameNum + 4; // perhaps even +1 is OK
#endif // USE_TEMP_START_FRAME_NUMBER_WORKAROUND
	info.nextRenderFrameTime	= renderFrameTime + mixerFrameDuration * (info.nextRenderFrameNumber - renderFrameNum);
	info.streamStartTime	      = node->startStreamTime;
	info.streamStartTimeValid  = node->startStreamTimeValid;

	DPST("info.renderFrameDuration   %d\n", info.renderFrameDuration.Get32BitDuration());
	DPST("info.nextRenderFrameNumber %d\n", info.nextRenderFrameNumber);
	DPST("renderFrameNum             %d\n", renderFrameNum);
	DPST("renderFrameTime            %d\n", renderFrameTime.Get32BitTime());
	DPST("info.nextRenderFrameTime   %d\n", info.nextRenderFrameTime.Get32BitTime());
	if (node->startStreamTimeValid)
		DPST("info.streamStartTime       %d\n", info.streamStartTime.Get32BitTime());
	else
		DPST("info.streamStartTime       INVALID\n");
	DPST("\n");
	
	STFRES_RAISE_OK;
	}


STFResult StreamMixer::GetStreamTimeOffset(uint32 inputID, STFHiPrec64BitDuration & systemOffset)
	{
	StreamMixerInputNode	*	node;
	STFHiPrec64BitTime		renderFrameTime;
	uint32						renderFrameNum;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	//
	// Retrieve system time and number of the currenly rendered frame
	//
	STFRES_REASSERT(this->GetRenderFrameTime(renderFrameTime, renderFrameNum));

	//
	// If the played frame does belong to our group...
	//
	if (node->startFrameNumber != INFINITE_FRAME_NUMBER) //<= renderFrameNum)
		{
		if (node->direction == MID_BACKWARD)
			systemOffset = node->startStreamTime - (renderFrameTime + mixerFrameDuration * (renderFrameNum - node->startFrameNumber)).FractMul(node->speed);
		else
			systemOffset = node->startStreamTime - (renderFrameTime - mixerFrameDuration * (renderFrameNum - node->startFrameNumber)).FractMul(node->speed);
		}
	else
		{
		systemOffset = node->startStreamTime - renderFrameTime.FractMul(node->speed);
		}
#if 0
	if (inputID == 3  &&  renderFrameNum >= lastNum+10)
		{
		DPR("SST: %8u RFT: %8u RFN: %8u SFN: %8u\n", node->startStreamTime.Get32BitTime(), renderFrameTime.Get32BitTime(), renderFrameNum, node->startFrameNumber);
		lastNum = renderFrameNum;
		}
#endif
	STFRES_RAISE_OK;
	}

STFResult StreamMixer::SetStreamTimeOffset(uint32 inputID, const STFHiPrec64BitDuration & systemOffset)
	{
	StreamMixerInputNode	*	node;
	STFHiPrec64BitTime		renderFrameTime;
	uint32						renderFrameNum;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	//
	// Retrieve the currenly rendered frame and system time
	//
	STFRES_REASSERT(this->GetRenderFrameTime(renderFrameTime, renderFrameNum));

	//
	// If the played frame does belong to our group...
	//
	if (node->startFrameNumber <= renderFrameNum)
		{
		node->reqStartStreamTime = systemOffset + (renderFrameTime - mixerFrameDuration * (renderFrameNum - node->startFrameNumber)).FractMul(node->speed);
		node->commandResynch = true;
		DPST("SetStreamTimeOffset id %d reqStartStreamTime %d ms\n", inputID, node->reqStartStreamTime.Get32BitTime());
		}
	else if (node->startFrameNumber == INFINITE_FRAME_NUMBER)
		{
		node->reqStartStreamTime = systemOffset + renderFrameTime.FractMul(0);
		node->commandResynch = true;
		DPST("SetStreamTimeOffset INFINITE_FRAME_NUMBER id %d reqStartStreamTime %d ms\n", inputID, node->reqStartStreamTime.Get32BitTime());
		}


	STFRES_RAISE_OK;
	}


STFResult StreamMixer::StartStream(uint32 inputID, uint32 mixFrameNumber, int32 speed, const STFHiPrec64BitTime & startTime)
	{
	StreamMixerInputNode	*	node;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	// When mixing is started, set the startup state to initial
	node->startupState = MIXSS_INITIAL;

	//
	// Set the speed startup related parameters, setting the frameNumber
	// starts the mixing in the next iteration
	//
	node->speed						= speed;
	node->startFrameNumber		= mixFrameNumber;	
	node->frameNumber				= mixFrameNumber;
	node->startStreamTimeValid = true;
	node->startStreamTime	   = startTime;

	// chztbd need to request some buffers before we need them.  Either here or sometime amount of frames before we need them..
	node->packetRequest		= true;				
	STFRES_RAISE_OK;
	}


STFResult StreamMixer::StopStream(uint32 inputID)
	{
	StreamMixerInputNode	*	node;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	//
	// Set the stop request
	//
	node->commandStop = true;

	//
	// Signal the mixer thread to process the stop
	//
	this->SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::PrepareStream(uint32 inputID, int32 direction)
	{
	StreamMixerInputNode	*	node;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	// We start the preparation with the assumption that there isn't enough data
	node->startupState = MIXSS_NOT_ENOUGH_DATA;

	node->commandPrepare = true;

	DPST("prepare node inputID %d, unitID %08x\n", inputID, GetUnitID());

	//
	// Signal the mixer thread to process the prepare
	//
	this->SetThreadSignal();

	switch (direction)
		{
		case VDR_STRMDIR_FORWARD: node->direction = MID_FORWARD; break;
		case VDR_STRMDIR_BACKWARD: node->direction = MID_BACKWARD; break;
		default: 
			{
			node->direction = MID_UNKNOWN; break;
			DP("StreamMixer::PrepareStream() invalid streaming direction (%i)\n", direction);
			STFRES_RAISE(STFRES_INVALID_PARAMETERS);
			}
		}

	STFRES_RAISE_OK;
	}

STFResult StreamMixer::StepStream(uint32 inputID, uint32 numFrames)
	{
	STFResult					res = STFRES_OK;
	StreamMixerInputNode	*	node;

	res = frameMixer->GetInputNode(inputID, node);

	if (STFRES_SUCCEEDED(res))
		res = frameMixer->StepStream(inputID, numFrames);

	//since  node->startFrameNumber == INFINITE_FRAME_NUMBER, and the other streams have probably moved
	node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_SYNCH_REQUEST, 1, 0), false);

	node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_STEPPED, res, 0), false);

	STFRES_RAISE_OK;
	}

STFResult StreamMixer::FlushStream(uint32 inputID, int32 mode)
	{
	StreamMixerInputNode	*	node;

	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	STFRES_REASSERT(node->ClearNotificationQueue());

	//
	// Flushing is synchronous in this implementation, because it assumes the
	// mixer to be already stopped.
	//
	STFRES_REASSERT(frameMixer->FlushStream(inputID, mode));
	
	node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_FLUSHED, 0, 0), false);

	STFRES_RAISE_OK;
	}


STFResult StreamMixer::ReceivePacket(uint32 inputID, StreamingDataPacket * packet)
	{
	STFResult						res;
	StreamMixerStartupRequest	req;
	STFHiPrec64BitTime			startTime, endTime;
	StreamMixerInputNode	*	node;

	// Get the current input node
	STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

	if (node->AvailNotificationQueue() >= 5)
		{
		// Retrieve start time
		if ((packet->vdrPacket.flags & VDR_MSMF_START_TIME_VALID) == VDR_MSMF_START_TIME_VALID)
			{
			startTime = packet->vdrPacket.startTime;
			}
		else
			{
			// Retrieve start time from the frame mixer
			frameMixer->GetCurrentInputStreamTime(inputID, startTime);
			}

		res = frameMixer->SendInputPacket(inputID, packet, req);


		if (req != MIXSUPREQ_NONE)
			{
			if (req == MIXSUPREQ_START_POSSIBLE)
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_START_POSSIBLE, 0, 0), false);
			else if (req == MIXSUPREQ_START_REQUIRED)
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_START_REQUIRED, 0, 0), false);
			}

		if (STFRES_SUCCEEDED(res))
			{
			// Check if the packet contains segment start notification requests
			if ((packet->vdrPacket.flags & (VDR_MSMF_SEGMENT_START | VDR_MSCF_SEGMENT_START_NOTIFICATION)) == (VDR_MSMF_SEGMENT_START | VDR_MSCF_SEGMENT_START_NOTIFICATION))
				{
				// Insert two notification into our notification queue, first a segment start, then a segment start time
				node->InsertPendingNotification(VDRMID_STRM_SEGMENT_START, startTime, packet->vdrPacket.segmentNumber, 0);

				// We cannot determine the start time of the segment's presentation yet, so we store dummies
				node->InsertPendingNotification(VDRMID_STRM_SEGMENT_START_TIME, startTime, 0, 0);
				}

			// Check if the packet contains group start notification requests
			if ((packet->vdrPacket.flags & (VDR_MSMF_GROUP_START | VDR_MSCF_GROUP_START_NOTIFICATION)) == (VDR_MSMF_GROUP_START | VDR_MSCF_GROUP_START_NOTIFICATION))
				{
				// We cannot yet set the second parameter, because we do not yet knwo the delta
				node->InsertPendingNotification(VDRMID_STRM_GROUP_START, startTime, packet->vdrPacket.groupNumber, 0);
				}

			// Retrieve end time
			if ((packet->vdrPacket.flags & VDR_MSMF_END_TIME_VALID) == VDR_MSMF_END_TIME_VALID)
				{
				endTime = packet->vdrPacket.endTime;
				}
			else
				{
				// Retrieve end time from the frame mixer
				frameMixer->GetCurrentInputStreamTime(inputID, endTime);
				}

			// Check if the packet contains group end notification requests
			if ((packet->vdrPacket.flags & (VDR_MSMF_GROUP_END | VDR_MSCF_GROUP_END_NOTIFICATION)) == (VDR_MSMF_GROUP_END | VDR_MSCF_GROUP_END_NOTIFICATION))
				{
				// We cannot yet set the second parameter, because we do not yet knwo the delta
				node->InsertPendingNotification(VDRMID_STRM_GROUP_END, endTime, packet->vdrPacket.groupNumber, 0);
				}

			// Check if the packet contains segment end notification requests
			if ((packet->vdrPacket.flags & (VDR_MSMF_SEGMENT_END | VDR_MSCF_SEGMENT_END_NOTIFICATION)) == (VDR_MSMF_SEGMENT_END | VDR_MSCF_SEGMENT_END_NOTIFICATION))
				{
				// Insert two notification into our notification queue, first a segment start, then a segment start time
				node->InsertPendingNotification(VDRMID_STRM_SEGMENT_END, endTime, packet->vdrPacket.segmentNumber, 0);
				}
			}
//		else if (res == STFRES_OBJECT_FULL)
//			node->packetRequest = true;
		}
	else
		{
		node->packetBounced = true;
		res = STFRES_OBJECT_FULL;
		}
	
	this->SetThreadSignal();

	STFRES_RAISE(res);
	}


void StreamMixer::ThreadEntry(void)
	{			
	enum StreamMixerState
		{
		MIXSTATE_PACKET_ALLOCATION,
		MIXSTATE_MIX_FRAME,
		MIXSTATE_SEND_FRAME
		} mixState = MIXSTATE_PACKET_ALLOCATION;

	uint32						i, numReadyPackets;
	STFResult					result;
	uint32						inputID;
	StreamMixerInputNode	*	node;
	bool							idledLoop;

	for (i = 0; i < registeredOutputs; i++)
		outputPackets[i] = NULL;

	numReadyPackets = 0;

	// Whenever processing is done in the thread loop, this variable is set to false,
	// and the thread will not wait on the signal. However, if nothing had to be done
	// during one loop, the thread is put to sleep.
	idledLoop = true;

	while (!terminate)
		{
		if (idledLoop)
			{
#if DEBUG_DUMP_MIXER_SLEEP_STATES
			STFHiPrec64BitTime	mt;
			SystemTimer->GetTime(mt);
			DPR("Mix %08x at %d Sleep %d\n", this, mt.Get32BitTime(), mixState);
#endif // DEBUG_DUMP_MIXER_SLEEP_STATES

			WaitThreadSignal();

#if DEBUG_DUMP_MIXER_SLEEP_STATES
			SystemTimer->GetTime(mt);
			DPR("Mix %08x at %d Wake %d\n", this, mt.Get32BitTime(), mixState);
#endif // DEBUG_DUMP_MIXER_SLEEP_STATES
			}

		idledLoop = true;

		//
		// Try to get an output packet, if we don't have one already
		//
		switch (mixState)
			{
			case MIXSTATE_PACKET_ALLOCATION:
				
				if (streamingOutputs == 0)
					break;

//				DPMT("mx Alloc\n");
	
				// Try to get an empty packet for each output
				for (i = 0; i < registeredOutputs; i++)
					{
					if (outputPackets[i] == NULL && mixerOutputs[i].streaming)
						{
						if (STFRES_SUCCEEDED(result = mixerOutputs[i].output->GetEmptyDataPacket(outputPackets[i])))
							{
							idledLoop = false;

							numReadyPackets++;

							//
							// Initialize packet
							//							
							outputPackets[i]->vdrPacket.flags	=	VDR_MSCF_GROUP_START_NOTIFICATION |	// Request to be informed about group start
																				VDR_MSCF_GROUP_END_NOTIFICATION |
																				VDR_MSMF_GROUP_START | VDR_MSMF_GROUP_END;

							if (mixerOutputs[i].firstOutputPacket)
								{
								DPMT("generate first packet for output %d, unit %08x\n", i, GetUnitID());
								outputPackets[i]->vdrPacket.flags |= VDR_MSCF_SEGMENT_START_NOTIFICATION |	// Request to be informed about segment start
																				 VDR_MSMF_SEGMENT_START | VDR_MSMF_DATA_DISCONTINUITY;
								mixerOutputs[i].firstOutputPacket = false;
								}

							outputPackets[i]->vdrPacket.segmentNumber		= outputSegmentNumber;
							outputPackets[i]->vdrPacket.groupNumber		= mixerFrameNum;
							outputPackets[i]->vdrPacket.numRanges			= 0;
							outputPackets[i]->vdrPacket.numTags				= 0;
							outputPackets[i]->vdrPacket.frameStartFlags	= 1;
							}
						else
							{
							DPR("0x%x StreamMixer GetEmptyDataPacket FAILED, known mixer stall, increase mixer output packets\n", this);
							}
						}
					}

				// If a packet was allocated for all outputs, go to the mixing state
				if (numReadyPackets == streamingOutputs)
					{
					mixState = MIXSTATE_MIX_FRAME;
					}
				else 
					break;

				// Intended fall-through!

			case MIXSTATE_MIX_FRAME:
//				DPMT("mx fr\n");
				// Success is returned when all packets have been filled - then they can be sent.
				if (streamingOutputs != 0)
					{
					if (STFRES_SUCCEEDED(result = frameMixer->MixFrame(outputPackets)))
						{
						idledLoop = false;
						mixState = MIXSTATE_SEND_FRAME;
						}
					else
						{
						DPMT("mx fr fail\n");
						break;
						}
					}
				else
					{
					mixState = MIXSTATE_SEND_FRAME;
					}

				// Intended fall-through!

			case MIXSTATE_SEND_FRAME:
//				DPMT("mx snd\n");

				// Try to send packets that have not already been sent
				for (i = 0; i < registeredOutputs; i++)
					{
					if (outputPackets[i] != NULL)
						{
						if (STFRES_SUCCEEDED(mixerOutputs[i].output->SendPacket(outputPackets[i])))
							{
							idledLoop = false;
							outputPackets[i] = NULL;
							numReadyPackets--;
							}
						else
							{
							DPMT("mx snd fail\n");
							}
						}
					}

				// The counter is 0 again when all packets have been sent.
				if (numReadyPackets == 0)
					{
					idledLoop = false;

					// Go to the next mixer frame number.
					mixerFrameNum++;

					mixState = MIXSTATE_PACKET_ALLOCATION;
					}
				
				// Intended fall-through!
			}

		//
		// Perform output state transitions in a safe state, no processing
		// pending etc.
		//			
		if (numReadyPackets == 0)
			{
			for(i = 0; i < registeredOutputs; i++)
				{
				if (mixerOutputs[i].commandPrepare)
					{
					if (!mixerOutputs[i].streaming)
						{
						DPMT("commandPrepare (%p) output %d, unit %08x\n", this, i, GetUnitID());
						if (STFRES_IS_ERROR(frameMixer->BeginOutput(i)))
							{
							DPMT("frameMixer->BeginOutput FAILED\n");
							}
						mixerOutputs[i].streaming = true;
						mixerOutputs[i].firstOutputPacket = true;
						mixerOutputs[i].renderStarted = false;
						streamingOutputs++;						
						}
					mixerOutputs[i].output->MixerNotification(VDRMID_STRM_MIXER_PREPARED, 0, 0);
					mixerOutputs[i].commandPrepare = false;
					idledLoop = false;
					}


				else if (mixerOutputs[i].commandFlush)
					{
					DPMT("flush (%p) output %d, unit %08x\n", this, i, GetUnitID());

					if (STFRES_IS_ERROR(frameMixer->FlushOutput(i)))
						{
						DPMT("frameMixer->BeginOutput FAILED\n");
						}

					if (mixerOutputs[i].streaming)
						{
						mixerOutputs[i].streaming = false;
						streamingOutputs--;
						}					
					mixerOutputs[i].output->MixerNotification(VDRMID_STRM_MIXER_FLUSHED, 0, 0);
					mixerOutputs[i].commandFlush = false;
					idledLoop = false;					
					}
				}
			}

		//
		// Iterate through all clients, and check for pending processing
		//
		for (inputID = 0; inputID < registeredInputs; inputID++)
			{
			frameMixer->GetInputNode(inputID, node);

			//
			// Is it time to send a sync request?
			//
			if ((mixerFrameNum & 15) == 0 
				&& mixState == MIXSTATE_PACKET_ALLOCATION 
				&& node->frameNumber != node->startFrameNumber
				&& node->startFrameNumber != INFINITE_FRAME_NUMBER)
				{
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_SYNCH_REQUEST, 1, 0), false);
				}

			//
			// See if we are low on data
			//
			if (node->packetRequest)
				{
				idledLoop = false;

				node->packetRequest = false;
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_PACKET_REQUEST, 1, 0), false);
				}

			//
			// notify if a starvation occured
			//
			if (node->starvation)
				{
				idledLoop = false;

				node->starvation = false;
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_STARVATION, 1, 0), false);
				}
			
			//
			// Perform stop if requested
			//
			if (node->commandStop)
				{
				idledLoop = false;

				node->commandStop = false;								

				uint32 oldTime = node->startStreamTime.Get32BitTime(STFTU_MILLISECS);

				if (node->direction == MID_BACKWARD)
					node->startStreamTime = node->startStreamTime - (mixerFrameDuration * (mixerFrameNum - node->startFrameNumber)).FractMul(node->speed);				
				else
				node->startStreamTime = node->startStreamTime + (mixerFrameDuration * (mixerFrameNum - node->startFrameNumber)).FractMul(node->speed);				

				DPSYNC("SM-Thread - type: %d SST: %8u\n", node->inputType, node->startStreamTime.Get32BitTime());
				DPST("new start time is %x\n", node->startStreamTime.Get32BitTime(STFTU_MILLISECS));
				DPST("mixerFrameNum is %d, node->startFrameNumber is %d, mixerFrameDuration is %d, old start time %x\n", mixerFrameNum, node->startFrameNumber, mixerFrameDuration.Get32BitDuration(STFTU_MILLISECS), oldTime);
				node->startFrameNumber	= INFINITE_FRAME_NUMBER;
				node->frameNumber			= INFINITE_FRAME_NUMBER;
				node->speed = 0;
				
				node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_STOPPED, 0, 0), false);
				}

			//
			// Perform Prepare if requested
			//
			if (node->commandPrepare)
				{
				if (mixState == MIXSTATE_SEND_FRAME)
					{
					// give some time back or we tend to deadlock if send frame fails
					SystemTimer->WaitDuration(5);
					}
				bool allOutputsStarted = true;

				for(i = 0; i < registeredOutputs; i++)
					{
					if (!mixerOutputs[i].renderStarted)
						{
						DPMT("mx commandPrepare mixerOutputs[%d].renderStarted = %d\n", i, mixerOutputs[i].renderStarted);
						allOutputsStarted = false;
						}
					}

				if (allOutputsStarted)
					{
					DPMT("commandPrepare (%p) inputID %d, unit %08x\n", this, inputID, GetUnitID());
					node->commandPrepare = false;
					node->inputSink->SendMessage(STFMessage(VDRMID_STRM_MIXER_PREPARED, 0, 0), false);
					}

				}

			//
			// Update config data
			//
			if (node->configurePending)
				{
				//
				// Aquire read lock
				//
				configReadLock = true;

				if (!configWriteLock)
					{
					idledLoop = false;
	
					frameMixer->InternalUpdateStreamTags(inputID);
					node->configurePending = false;
					idledLoop = false;
					}
				//
				// Signal read lock released
				//
				configReadLock = false;
				configSignal.SetSignal();
				}
			}
		}
	}


STFResult StreamMixer::MixerOutputNotification(uint32 outputID, VDRMID message, uint32 param1, uint32 param2)
	{
	STFHiPrec64BitTime		systemTime;
	STFHiPrec64BitTime		streamTime;
	STFHiPrec64BitDuration	systemToStreamOffset, delta;
	STFInt64						ticks;
	uint32						inputID;
	uint32						dummy, t;
	STFMessage					upstreamMessage;
	StreamMixerInputNode*	node;

	switch (message)
		{
		// Start of first segment
		case VDRMID_STRM_SEGMENT_START_TIME:
			if (outputID == 0)	// check for ID of "master" output
				{
				this->SetRenderFrameTime(STFHiPrec64BitTime(STFInt64(param1, param2), STFTU_108MHZTICKS), renderStartFrame);
				}
			this->mixerOutputs[outputID].renderStarted = true;
			// trigger mixing thread which should be in prepare state
			this->SetThreadSignal();
			break;

		// Start of group
		case VDRMID_STRM_GROUP_START:
			if (outputID == 0  &&  param2 != 0)	// check for ID of "master" output
				{
				t = mixerFrameNum & 0xffff0000;
				if ((param1 & 0x8000) > (mixerFrameNum & 0x8000))
					t -= 0x00010000;
				param1 = param1 + t;

				this->SetRenderFrameTime(asyncRenderFrameTime.time0 + STFHiPrec64BitDuration(param2, STFTU_108MHZTICKS), param1);
				}
			break;

		case VDRMID_STRM_GROUP_END:
			if (outputID == 0)	// check for ID of "master" output
				{
				t = mixerFrameNum & 0xffff0000;
				if ((param1 & 0x8000) > (mixerFrameNum & 0x8000))
					t -= 0x00010000;
				param1 = param1 + t;

				this->SetRenderFrameTime(asyncRenderFrameTime.time0 + STFHiPrec64BitDuration(param2, STFTU_108MHZTICKS), param1 + 1);				
				}
			break;

		// Packet request or memory block available triggers mixing thread (the output unit transforms both into one packet request)
		case VDRMID_STRM_PACKET_REQUEST:
			this->SetThreadSignal();
			break;
		}

	if (outputID == 0)	// check for ID of "master" output
		{
		// Check if this is one of the "timed messages" we must forward upstream
		if ((message == VDRMID_STRM_GROUP_START) || (message == VDRMID_STRM_GROUP_END) || (message == VDRMID_STRM_SEGMENT_START) || (message == VDRMID_STRM_SEGMENT_END))
			{
			// Loop through all inputs
			for (inputID = 0; inputID < this->registeredInputs; inputID ++)
				{
				STFRES_REASSERT(frameMixer->GetInputNode(inputID, node));

				// Read the renderer system time
				this->GetRenderFrameTime(systemTime, dummy);

				STFRES_REASSERT(this->GetStreamTimeOffset(inputID, systemToStreamOffset));

				streamTime = systemTime.FractMul(node->speed) + systemToStreamOffset;

				// Send all messages that are due in the input
				while (node->GetFirstDueMessage(streamTime, upstreamMessage) != STFRES_OBJECT_EMPTY)
					{
					// We need to check which message we will send, because some need special parameters to be set
					switch(upstreamMessage.message)
						{
						case VDRMID_STRM_SEGMENT_START:
						case VDRMID_STRM_SEGMENT_END:
							{
							} break;
						case VDRMID_STRM_SEGMENT_START_TIME:
							{
							ticks = systemTime.Get64BitTime(STFTU_108MHZTICKS);
							upstreamMessage.param1 = ticks.Lower();
							upstreamMessage.param2 = ticks.Upper();
							} break;
						case VDRMID_STRM_GROUP_END:
						case VDRMID_STRM_GROUP_START:
							{
							delta = systemTime - node->lastTimedMessageTime;
							upstreamMessage.param2 = delta.Get32BitDuration(STFTU_108MHZTICKS);
							} break;
						default:
							{
							DPTN("StreamMixer::MixerOutputNotification() Unhandled message id: 0x%08x\n", upstreamMessage.message);
							} break;
						case VDRMID_STRM_PACKET_REQUEST:
							break;
						}

					// Remeber message time
					node->lastTimedMessageTime = systemTime;

					// Send message to the input to be forwarded
					STFRES_REASSERT(node->inputSink->SendMessage(upstreamMessage, false)); 
					}
				}
			}
		}

	STFRES_RAISE_OK;
	}



///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Unit
///////////////////////////////////////////////////////////////////////////////


STFResult VirtualStreamMixer::ConfigureTags(TAG * tags)
	{
	// Only accept configuration if this virtual unit is not the currently active one
	if (STFRES_FALSE == IsUnitCurrent())
		{
		physicalMixer->virtualFrameMixer->ConfigureTags(tags);
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_CURRENT);

	STFRES_RAISE_OK;
	}




