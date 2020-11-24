///
/// @brief 		 Chain Links
///

#include "ChainLink.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "STF/Interface/STFTimer.h"

#define DPR_CLN	DP_EMPTY

///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateChainLinkInput, ChainLinkInput)


ChainLinkInput::~ChainLinkInput()
	{
	if (output)
		output->Release();
	}


STFResult ChainLinkInput::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualChainLinkInput(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult ChainLinkInput::Create(uint64 * createParams)
	{
	STFRES_RAISE_OK;
	}


STFResult ChainLinkInput::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			STFRES_REASSERT(source->QueryInterface(VDRIID_CHAIN_LINK_OUTPUT, (void*&) output));
			break;

		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult ChainLinkInput::Initialize(uint64 * depUnitsParams)
	{
	if (output == NULL)
		STFRES_RAISE(STFRES_BOARDCONSTUCTION_INCOMPLETE_CONFIGURATION);

	// Register this input at the output
	STFRES_RAISE(output->RegisterLinkInput(this));
	}


STFResult ChainLinkInput::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_CHAIN_LINK_INPUT, IChainLinkInput);
	VDRQI_END(ExclusivePhysicalUnit);

	STFRES_RAISE_OK;
	}


STFResult ChainLinkInput::SetVirtualChainLinkInput(VirtualChainLinkInput * unit)
	{
	if (allocator)
		{
		// If there is a previous virtual unit, clear out the allocator in its chain
		if (curVirtualInput)
			{
			STFRES_REASSERT(curVirtualInput->ReceiveAllocator(NULL));
			curVirtualInput = NULL;
			}

		// When a new virtual unit is activated, we provide the allocator to its chain
		if (unit)
			{
			curVirtualInput = unit;
			STFRES_REASSERT(curVirtualInput->ReceiveAllocator(allocator));
			}
		}
	else
		curVirtualInput = unit;

	STFRES_RAISE_OK;
	}


STFResult ChainLinkInput::SendUpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	// Forward the message to the currently active virtual Stream Mixer Input unit
	if (curVirtualInput)
		STFRES_RAISE(curVirtualInput->LinkInputNotification(message, param1, param2));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}


STFResult ChainLinkInput::ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
	{
	this->allocator = allocator;
	
	if (curVirtualInput)
		STFRES_REASSERT(curVirtualInput->ReceiveAllocator(allocator));

	STFRES_RAISE_OK;
	}

STFResult ChainLinkInput::IsPushingChain(void)
	{
	if (curVirtualInput)
		STFRES_RAISE(curVirtualInput->IsPushingChain());
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult ChainLinkInput::GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset)
	{
	if (curVirtualInput && curVirtualInput->streamingClock)
		STFRES_RAISE(curVirtualInput->streamingClock->GetCurrentStreamTimeOffset(systemOffset));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult ChainLinkInput::GetStreamingState(VDRStreamingState & state)
	{
	if (curVirtualInput)
		STFRES_RAISE(curVirtualInput->GetState(state));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Input Streaming Unit
///////////////////////////////////////////////////////////////////////////////


STFResult VirtualChainLinkInput::PreemptUnit (uint32 flags)
	{
	STFRES_REASSERT(VirtualStreamingUnit::PreemptUnit (flags));

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		physicalInput->SetVirtualChainLinkInput(NULL);
		}

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		physicalInput->SetVirtualChainLinkInput(this);
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualChainLinkInput::LinkInputNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
	}


STFResult VirtualChainLinkInput::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	// We ignore the state of the target chain and always signal success.
	// If the target stream should not be ready to accept data due to its state,
	// then this will be properly handled in ReceivePacket() below.
	
	this->SignalStreamingCommandCompletion(command, STFRES_OK);

	STFRES_RAISE_OK;
	}


STFResult VirtualChainLinkInput::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	VDRStreamingState curTargetChainState;

	// Try to get state of target chain. If that is not ready, we will not succeed the call
	if (STFRES_SUCCEEDED(physicalInput->output->GetStreamingState(curTargetChainState)))
		{
		// Check if target chain is in the right state to accept data, only then send it
		if (curTargetChainState == VDR_STRMSTATE_READY || curTargetChainState == VDR_STRMSTATE_STREAMING)
			STFRES_RAISE(physicalInput->output->SendPacket(packet));
		}

	// In all the other case, we check if there are any messages to generate, and otherwise burn the data packets
	// and their content here.

	// Check if there are any messages to generate
	if ((packet->vdrPacket.flags & VDR_MSMF_SEGMENT_END) && 
		 (packet->vdrPacket.flags & VDR_MSCF_SEGMENT_END_NOTIFICATION))
		{
		inputConnector.SendUpstreamNotification(VDRMID_STRM_SEGMENT_END, packet->vdrPacket.segmentNumber, 0);
		}

	if ((packet->vdrPacket.flags & VDR_MSMF_SEGMENT_START) && 
		 (packet->vdrPacket.flags & VDR_MSCF_SEGMENT_START_NOTIFICATION))
		{
		inputConnector.SendUpstreamNotification(VDRMID_STRM_SEGMENT_START, packet->vdrPacket.segmentNumber, 0);
		}

	if ((packet->vdrPacket.flags & VDR_MSMF_GROUP_END) && 
		 (packet->vdrPacket.flags & VDR_MSCF_GROUP_END_NOTIFICATION))
		{
		inputConnector.SendUpstreamNotification(VDRMID_STRM_GROUP_END, packet->vdrPacket.groupNumber, 0);
		}

	if ((packet->vdrPacket.flags & VDR_MSMF_GROUP_START) && 
		 (packet->vdrPacket.flags & VDR_MSCF_GROUP_START_NOTIFICATION))
		{
		inputConnector.SendUpstreamNotification(VDRMID_STRM_GROUP_START, packet->vdrPacket.groupNumber, 0);
		}

	//??? Currently not generated: Streaming Startup Messages. It is not clear yet if this is necessary
	//    for the purpse of the chain link unts.

	packet->ReleaseRanges();
	packet->ReturnToOrigin();

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkInput::IsPushingChain(void)
	{
	STFRES_RAISE(inputConnector.IsPushingChain());
	}

STFResult VirtualChainLinkInput::PropagateStreamingClock(IStreamingClock * streamingClock)
	{
	this->streamingClock = streamingClock;

	STFRES_RAISE_OK;
	}

///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateChainLinkOutput, ChainLinkOutput)


STFResult ChainLinkOutput::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualChainLinkOutput(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult ChainLinkOutput::Create(uint64 * createParams)
	{
	if (STFRES_FAILED(GetDWordParameter(createParams, 0, priority)))
		{
		priority = 0;
		}
	if (STFRES_FAILED(GetDWordParameter(createParams, 1, chainDelay)))
		{
		chainDelay = 0;
		}

	STFRES_RAISE_OK;
	}


ChainLinkOutput::~ChainLinkOutput()
	{
	}


STFResult ChainLinkOutput::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}


STFResult ChainLinkOutput::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


STFResult ChainLinkOutput::RegisterLinkInput(IChainLinkInput * input)
	{
	if (this->input != NULL)
		STFRES_RAISE(STFRES_OBJECT_IN_USE);

	this->input = input;

	STFRES_RAISE_OK;
	}


STFResult ChainLinkOutput::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_CHAIN_LINK_OUTPUT, IChainLinkOutput);
	VDRQI_END(ExclusivePhysicalUnit);

	STFRES_RAISE_OK;
	}


STFResult ChainLinkOutput::SendPacket(StreamingDataPacket * packet)
	{
	if (curOutputUnit)
		{
		// We switch to the curOutputUnit, as this pointer should be valid now
		STFRES_RAISE(curOutputUnit->ReceivePacket(0, packet));
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_CURRENT);
	}


STFResult ChainLinkOutput::GetStreamingState(VDRStreamingState & state)
	{
	if (curOutputUnit)
		STFRES_RAISE(curOutputUnit->GetState(state));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_CURRENT);	
	}

STFResult ChainLinkOutput::IsPushingChain(void)
	{
	if (curOutputUnit)
		STFRES_RAISE(input->IsPushingChain());
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_CURRENT);	
	}



///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

VirtualChainLinkOutput::VirtualChainLinkOutput(ChainLinkOutput * physicalOutput)
	: VirtualStreamingUnit(physicalOutput),
	  outputConnector(16, 0, this)
	{ 
	this->physicalOutput = physicalOutput;

	// Add external connector to StreamingUnit connector list
	AddConnector((IStreamingConnector*)&outputConnector);		// => ID 0

	isPushingChainLink   = false;
	isPullingChainLink   = false;

	flushRequest			= false;
	packetBounced			= false;
	processRequest			= false;
	stopRequest				= false;

	pendingPacket			= NULL;
	targetPacket         = NULL;

	insideSegment = false;
	insideGroup = false;
	discontinuityPending = false;
	firstPacket = true;

	segmentNumber = 0;
	groupNumber = 0;
	}


STFResult VirtualChainLinkOutput::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualChainLinkOutput::CalculatePacketTime(const	STFHiPrec64BitTime & inputTime, STFHiPrec64BitTime & outputTime)
	{
	STFHiPrec64BitTime		systemCaptureTime;
	STFHiPrec64BitDuration	systemCaptureOffset;

	STFRES_REASSERT(physicalOutput->input->GetCurrentStreamTimeOffset(systemCaptureOffset));
	systemCaptureTime = inputTime - systemCaptureOffset; 
	if (firstPacket)
		{
		streamingClock->GetCurrentStreamTimeOffset(currentSystemTimeOffset);
		firstPacket = false;
		}
	else
		{
		currentStreamTime += inputTime - previousInputTime;
		STFRES_REASSERT(streamingClock->SynchronizeClient(clockID, physicalOutput->priority, currentStreamTime - systemCaptureTime, currentSystemTimeOffset));
		}
	previousInputTime = inputTime;
	currentStreamTime = systemCaptureTime + currentSystemTimeOffset;

	outputTime = currentStreamTime + STFHiPrec64BitDuration(physicalOutput->chainDelay, STFTU_MILLISECS);

	DPR_CLN("VCLO %08x CPT Input %d Output %d SysCap %d sysOff %d\n", GetUnitID(), inputTime.Get32BitTime(), outputTime.Get32BitTime(), systemCaptureTime.Get32BitTime(), currentSystemTimeOffset.Get32BitDuration());

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::SendTargetPacket(void)
	{
	STFResult					result = STFRES_OK;
	uint32						flags;

	if (targetPacket)
		{
		flags = targetPacket->vdrPacket.flags;
		result = outputConnector.SendPacket(targetPacket);

		if (result != STFRES_OBJECT_FULL)
			{
			if (flags & VDR_MSMF_SEGMENT_START)
				insideSegment = true;
			if (flags & VDR_MSMF_GROUP_START)
				insideGroup = true;
			if (flags & VDR_MSMF_GROUP_END)
				{
				insideGroup = false;
				groupNumber++;
				}
			if (flags & VDR_MSMF_SEGMENT_END)
				{
				insideSegment = false;
				groupNumber = 0;
				segmentNumber++;
				}

			targetPacket = NULL;
			}
		}

	STFRES_RAISE(result);
	}

STFResult VirtualChainLinkOutput::ReleaseTargetPacket(void)
	{
	if (targetPacket)
		{
		targetPacket->ReleaseRanges();
		targetPacket->ReturnToOrigin();
		targetPacket = NULL;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::ReleasePendingPacket(void)
	{
	if (pendingPacket)
		{
		pendingPacket->ReleaseRanges();
		pendingPacket->ReturnToOrigin();
		pendingPacket = NULL;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::ProcessPendingPacket(void)
	{
	STFResult					result = STFRES_OK;

	//
	// Ensure only one path of control is inside the parsing segment,
	// we have to retry on collision, otherwise a packet request may
	// get lost due to a racing condition (thread rescheduled after
	// delivery failed, but before locking area left).
	//
	processRequest = true;
	do 
		{
		if (!pendingLock++)
			{
			processRequest = false;
			// Are we in the process of flushing?

			DPR_CLN("VCLO %08x PPP FR %d SR %d DP %d FP %d IS %d IG %d PP %08x TP %08x\n", 
				GetUnitID(), flushRequest, stopRequest, discontinuityPending, firstPacket, insideSegment, insideGroup, pendingPacket, targetPacket);

			if (flushRequest)
				{
				ReleasePendingPacket();
				ReleaseTargetPacket();

				insideGroup   = false;
				insideSegment = false;
				stopRequest   = false;

				groupNumber   = 0;
				segmentNumber = 0;
				firstPacket   = true;
				isPushingChainLink = false;
				isPullingChainLink = false;

				flushRequest  = false;
				this->SignalStreamingCommandCompletion(VDR_STRMCMD_FLUSH, result);
				}
			else if (stopRequest)
				{
				ReleasePendingPacket();
				ReleaseTargetPacket();

				if (insideSegment)
					{
					if (STFRES_SUCCEEDED(outputConnector.GetEmptyDataPacket(targetPacket)))
						{
						targetPacket->vdrPacket.flags |= VDR_MSMF_SEGMENT_END | VDR_MSMF_TIME_DISCONTINUITY;
						if (insideGroup)
							targetPacket->vdrPacket.flags |= VDR_MSMF_GROUP_END;
						targetPacket->vdrPacket.groupNumber = groupNumber;
						targetPacket->vdrPacket.segmentNumber = segmentNumber;

						stopRequest = false;
						this->SignalStreamingCommandCompletion(VDR_STRMCMD_BEGIN, result);

						SendTargetPacket();
						}
					}
				else
					{
					stopRequest = false;
					this->SignalStreamingCommandCompletion(VDR_STRMCMD_BEGIN, result);
					}
				}
			else 
				{
				SendTargetPacket();

				if (!targetPacket && discontinuityPending)
					{
					if (STFRES_SUCCEEDED(outputConnector.GetEmptyDataPacket(targetPacket)))
						{
//						DP("TIME_DISCONTIUITY %08x\n", GetUnitID());

						targetPacket->vdrPacket.flags = VDR_MSMF_TIME_DISCONTINUITY;
						if (insideGroup)
							targetPacket->vdrPacket.flags |= VDR_MSMF_GROUP_END;
						targetPacket->vdrPacket.numRanges = 0;
						targetPacket->vdrPacket.numTags = 0;
						targetPacket->vdrPacket.frameStartFlags = 0;
						targetPacket->vdrPacket.groupNumber = groupNumber;
						targetPacket->vdrPacket.segmentNumber = segmentNumber;

						firstPacket = true;
						discontinuityPending = false;

						SendTargetPacket();
						}
					}

				if (pendingPacket && !discontinuityPending)
					{
					//
					// Attempt to deliver the packet
					//
					if (!firstPacket || (pendingPacket->vdrPacket.flags & VDR_MSMF_GROUP_START) != 0)
						{
						if (!targetPacket && STFRES_SUCCEEDED(outputConnector.GetEmptyDataPacket(targetPacket)))
							{
							targetPacket->CopyFromVDRPacket(&(pendingPacket->vdrPacket));

							if (targetPacket->vdrPacket.flags & VDR_MSMF_START_TIME_VALID)
								{
								STFRES_REASSERT(CalculatePacketTime(pendingPacket->vdrPacket.startTime, targetPacket->vdrPacket.startTime));
								}

							if (targetPacket->vdrPacket.flags & VDR_MSMF_END_TIME_VALID)
								{
								STFRES_REASSERT(CalculatePacketTime(pendingPacket->vdrPacket.endTime, targetPacket->vdrPacket.endTime));
								}

							targetPacket->AddRefToRanges();
							if (!insideSegment)
								targetPacket->vdrPacket.flags |= VDR_MSMF_SEGMENT_START;
							targetPacket->vdrPacket.segmentNumber = segmentNumber;
							targetPacket->vdrPacket.groupNumber   = groupNumber;

							SendTargetPacket();
							}
						else
							discontinuityPending = true;
						}
					}

				ReleasePendingPacket();
				}
			}
		} while (--pendingLock == 0 && processRequest);

	STFRES_RAISE(result);
	}


STFResult VirtualChainLinkOutput::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	//StreamingDataPacket	*	tpacket;
	//STFResult					result;

	// We fake a bit here. Even though there is no "real" input connector, we receive packets
	// coming from our physical unit here.

	if (isPushingChainLink)
		{
		if (state == VDR_STRMSTATE_STREAMING)
			{
			ProcessPendingPacket();
			if (!pendingPacket)
				{
				pendingPacket = packet;
				ProcessPendingPacket();
				}
			else
				{
				packet->ReleaseRanges();
				packet->ReturnToOrigin();
				}
			}
		else
			{
			packet->ReleaseRanges();
			packet->ReturnToOrigin();
			}

		STFRES_RAISE_OK;
		}
	else
		{
		//DPR_CLN("VCLO %08x RCP PP %08x TP %08x\n", GetUnitID(), pendingPacket, targetPacket);

		STFRES_RAISE(outputConnector.SendPacket(packet));
		}
	}


STFResult VirtualChainLinkOutput::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
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
		case VDRMID_STRM_PACKET_REQUEST:
			if (isPushingChainLink)
				ProcessPendingPacket();
			else
				STFRES_RAISE(physicalOutput->input->SendUpstreamNotification(message, param1, param2));
			break;
		default:
			if (isPushingChainLink)
				STFRES_RAISE(parentStreamingUnit->UpchainNotification(message, param1, param2));
			else
				STFRES_RAISE(physicalOutput->input->SendUpstreamNotification(message, param1, param2));
			break;
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualChainLinkOutput::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_REASSERT(physicalOutput->input->ReceiveAllocator(allocator));

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::IsPushingChain(void)
	{
	STFResult	result;

	if (isPushingChainLink)
		return STFRES_TRUE;
	else if (isPullingChainLink)
		return STFRES_FALSE;
	else
		{
		result = physicalOutput->IsPushingChain();

		if (result == STFRES_TRUE)
			isPushingChainLink = true;
		else if (result == STFRES_FALSE)
			isPullingChainLink = true;

		STFRES_RAISE(result);
		}
	}

STFResult VirtualChainLinkOutput::IsPushingChain(uint32 connectorID)
	{
	STFRES_RAISE(IsPushingChain());
	}

STFResult VirtualChainLinkOutput::PropagateStreamingClock(IStreamingClock * streamingClock)
	{
	this->streamingClock = streamingClock;

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::CompleteConnection(void)
	{
	if (streamingClock)
		{
		STFRES_REASSERT(streamingClock->RegisterClient(this, clockID));
		DPR_CLN("Registered ChainLinkOutput Unit ID #%08x at Streaming Clock %08x -> clockID: %d\n", GetUnitID(), streamingClock, clockID);
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	VDRStreamingState						curSourceChainState;
	STFResult								result;
	StreamingClockClientStartupInfo	clientInfo;

	result = physicalOutput->input->GetStreamingState(curSourceChainState);

	if (STFRES_SUCCEEDED(result))
		{
		switch (command)
			{
			case VDR_STRMCMD_STEP:
				break;

			case VDR_STRMCMD_BEGIN:
				if (isPushingChainLink)
					{
					stopRequest = true;
					ProcessPendingPacket();
					STFRES_RAISE_OK;
					}
				else
					{
					STFRES_REASSERT(IsPushingChain());
					}
				break;

			case VDR_STRMCMD_DO:
 				clientInfo.nextRenderFrameNumber = 0;
				SystemTimer->GetTime(systemStartTime);					
				systemStartTime += STFHiPrec64BitDuration(physicalOutput->chainDelay, STFTU_MILLISECS);
				clientInfo.nextRenderFrameTime   = systemStartTime;
				clientInfo.renderFrameDuration   = STFHiPrec64BitDuration(1, STFTU_MILLISECS);
				clientInfo.streamStartTime       = STFHiPrec64BitTime(0, STFTU_MILLISECS);
				clientInfo.streamStartTimeValid  = isPushingChainLink;
				firstPacket                      = true;

				STFRES_RAISE(streamingClock->SetStartupDelay(clockID, clientInfo));
				break;

			case VDR_STRMCMD_FLUSH:
				if (isPushingChainLink)
					{
					flushRequest = true;
					ProcessPendingPacket();
					STFRES_RAISE_OK;
					}
				else
					{
					isPushingChainLink = false;
					isPullingChainLink = false;
					}
				break;

			case VDR_STRMCMD_NONE:
				break;

			default:
				DP("*** Unhandled STRMCMD in VirtualChainLinkInput::BeginStreamingCommand! ***\n");
			}
		}
	else
		DP("GetStreamingState failed Unit %08x Result %08x\n", GetUnitID(), result);

	this->SignalStreamingCommandCompletion(command, result);

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::SetStartupFrame(uint32 frameNumber, const STFHiPrec64BitTime & startTime)
	{
	DPR_CLN("SetStartupFrame %08x\n", GetUnitID());

	currentStreamTime       = startTime;
	currentSystemTimeOffset = currentStreamTime - (systemStartTime + STFHiPrec64BitDuration(frameNumber, STFTU_MILLISECS));

	this->SignalStreamingCommandCompletion(VDR_STRMCMD_DO, STFRES_OK);

	STFRES_RAISE_OK;
	}

STFResult VirtualChainLinkOutput::GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset)
	{
	systemOffset = currentSystemTimeOffset;

	STFRES_RAISE_OK;
	}


STFResult VirtualChainLinkOutput::PreemptUnit(uint32 flags)
	{
	STFRES_REASSERT(VirtualStreamingUnit::PreemptUnit(flags));

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		physicalOutput->SetVirtualChainLinkOutput(NULL);
		}

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		physicalOutput->SetVirtualChainLinkOutput(this);
		}

	STFRES_RAISE_OK;
	}



