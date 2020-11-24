///
/// @file       VDR/Source/Streaming/InfiniteStreamReplicator.cpp
///
/// @brief      Replicates a single input stream onto any number of output streams
///
/// @author     Ulrich Sigmund
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Implementation File
///
/// @date       2003-26-09
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///


#include "InfiniteStreamReplicator.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

UNIT_CREATION_FUNCTION(CreateStreamReplicatorStreamingUnit, StreamReplicatorStreamingUnit)

STFResult StreamReplicatorStreamingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamReplicatorStreamingUnit(this, numOutputs, numPacketsPerOutput, messageMode));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult StreamReplicatorStreamingUnit::Create(uint64 * createParams)
	{
	STFRES_REASSERT(GetDWordParameter(createParams, 0, numOutputs));

	if (STFRES_FAILED(GetDWordParameter(createParams, 1, (uint32 &)messageMode)))
		{
		// If the parameter is not specified, the default handling is used: collect group and segment end messages to forward them
		messageMode = SRMFM_FORWARD_DEFAULT;
		}

	if (STFRES_FAILED(GetDWordParameter(createParams, 2, numPacketsPerOutput)))
		{
		// If the parameter is not specified, the default setting of 16 packets per output is used.
		numPacketsPerOutput = 16;
		}

	STFRES_RAISE_OK;
	}


STFResult StreamReplicatorStreamingUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}


STFResult StreamReplicatorStreamingUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}

void VirtualStreamReplicatorStreamingUnit::ReleaseDestruct(void)
	{
	VirtualNonthreadedStandardStreamingUnit::ReleaseDestruct();
	}

STFResult VirtualStreamReplicatorStreamingUnit::ArmUpstreamCounter(StreamReplicatorMessageCounters counter, uint32 key, int32 value)
	{
	UpstreamEventCounter	*	ec = &(upstreamCounters[key & MASK_REPLICATOR_UPSTREAM_COUNTERS][counter]);
	uint32 k;

	// first check, if the counter we want to use is free or still in use
	if (messageMode != SRMFM_FORWARD_ALL  &&  ec->counter > 0)
		{
		// this counter is still in use, we have to reject processing
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}

	switch (messageMode)
		{
		case SRMFM_FORWARD_DEFAULT:
			ec->counter = value;
			break;
		case SRMFM_FORWARD_FIRST:
		case SRMFM_FORWARD_MAIN:
			ec->counter = 1;
			break;
		case SRMFM_FORWARD_ALL:
			ec->counter = 0;	// the counter is not used, we keep it reset
			break;
		case SRMFM_FORWARD_COMBINE:
			ec->counter = numOutputs;
			break;
		case SRMFM_TOTAL:
		default:
			ec->counter = value;
			DP("! ISR: unsupported message mode choosen\n");
		}

	ec->key = key;

	// now reset the flags
	for(k=0; k < numOutputs; k++)
		ec->id[k] = false;

	STFRES_RAISE_OK;
	}

// counter is message-id, key is number 
STFResult VirtualStreamReplicatorStreamingUnit::TriggerUpstreamCounter(StreamReplicatorMessageCounters counter, uint32 key, uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	UpstreamEventCounter	*	ec = &(upstreamCounters[key & MASK_REPLICATOR_UPSTREAM_COUNTERS][counter]);
	int32							delta, current;

	if (messageMode == SRMFM_FORWARD_ALL)
		{
		// we make no use of the upstream counters, but forward all notifications
		STFRES_REASSERT(inputConnector->SendUpstreamNotification(message, param1, param2));
		}
	else if (ec->key == key)
		{
		// the upstream event counter is armed
		if (ec->id[connectorID])
			{
			DP("! Replicator: connector %d reported message 0x%08X - %d again\n", connectorID, message, key);
			STFRES_RAISE_OK;
			}

		ec->id[connectorID] = true;

		if (messageMode != SRMFM_FORWARD_MAIN  ||  connectorID == 0)
			{
			if (--(ec->counter) == 0)
				{
				switch (message)
					{
					case VDRMID_STRM_SEGMENT_START_TIME:
						lastMessageTime = outputStreamTimes[connectorID];
						break;
					case VDRMID_STRM_GROUP_START:
					case VDRMID_STRM_GROUP_END:
						current = lastMessageTime;
						delta = outputStreamTimes[connectorID] - current;
						while (delta > 0 && lastMessageTime.CompareExchange(current, outputStreamTimes[connectorID]) != current)
							{
							current = lastMessageTime;
							delta = outputStreamTimes[connectorID] - current;
							}

						if (delta > 0)
							param2 = delta;
						else
							param2 = 0;
						break;
					}

				STFRES_REASSERT(inputConnector->SendUpstreamNotification(message, param1, param2));

				if (packetBounced  &&  sendingState != SRSS_REPLICATE_PACKETS  &&  sendingState != SRSS_SEND_PACKETS)
					STFRES_REASSERT(inputConnector->SendUpstreamNotification(VDRMID_STRM_PACKET_REQUEST, 0, 0));
				}
			}
		}
	else
		{
		DP("! Replicator: upstream event counter unarmed, skipping message 0x%08X #%d from connector %d\n", message, key, connectorID);
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamReplicatorStreamingUnit::ResetUpStreamCounters(void)
	{
	uint32 i, j, k;

	for(i=0; i < NUM_REPLICATOR_UPSTREAM_COUNTERS; i++)
		{
		for(j=0; j<SRMC_TOTAL; j++)
			{
			upstreamCounters[i][j].counter = 0;

			for(k=0; k < numOutputs; k++)
				upstreamCounters[i][j].id[k] = false;
			}
		}

	for(i=0; i<numOutputs; i++)
		{
		outputStreamTimes[i] = 0;
		outputSegmentNumbers[i] = 0;
		}

	startupPossibleCounter = numOutputs;
	startupRequiredCounter = 1;

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamReplicatorStreamingUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	uint32		i, j, n;
	VDRTID	*	tids;

	if (!streamingTAGIDs)
		{
		n = 0;
		for(i = 0; i < numOutputs; i++)
			{
			STFRES_REASSERT(outputConnectors[i]->GetStreamTagIDs(tids));
			j = 0;
			while (tids[j++]) n++;
			}

		streamingTAGIDs = new VDRTID[n+1];
		n = 0;
		for(i=0; i<numOutputs; i++)
			{
			STFRES_REASSERT(outputConnectors[i]->GetStreamTagIDs(tids));
			j = 0;
			while (tids[j])
				streamingTAGIDs[n++] = tids[j++];
			}

		streamingTAGIDs[n] = VDRTID_DONE;
		}

	ids = streamingTAGIDs;

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamReplicatorStreamingUnit::CompleteConnection(void)
	{
	VDRTID	*	tids;

	STFRES_REASSERT(VirtualNonthreadedStandardStreamingUnit::CompleteConnection());

	// Call this at connection time to ensure caching of stream
	// type IDs before streaming starts.  If this would be done during
	// streaming, a racing condition may occur.
	//
	STFRES_RAISE(GetStreamTagIDs(0, tids));
	}

STFResult VirtualStreamReplicatorStreamingUnit::ParseReceivePacket(const StreamingDataPacket * packet)
	{
	STFResult	result = STFRES_OK;
	uint32		i, flags;
	uint16		segmentID, groupID;

	flags = packet->vdrPacket.flags;	
	segmentID = packet->vdrPacket.segmentNumber;
	groupID = packet->vdrPacket.groupNumber;

	// if this is the first time, we see a packet, we remember its flags inside
	// the upstream event counters. Arming a counter can fail, in case the specific counter is still in use
	switch (sendingState)
		{
		case SRSS_ARM_SEGMENT_START:
			if ((flags & VDR_MSMF_SEGMENT_START) != 0 && (flags & VDR_MSCF_SEGMENT_START_NOTIFICATION) != 0)
				{
				STFRES_REASSERT(ArmUpstreamCounter(SRMC_SEGMENT_START, segmentID, 1));
				}
			sendingState = SRSS_ARM_SEGMENT_START_TIME;

		case SRSS_ARM_SEGMENT_START_TIME:
			if ((flags & VDR_MSMF_SEGMENT_START) != 0 && (flags & VDR_MSCF_SEGMENT_START_NOTIFICATION) != 0)
				{
				STFRES_REASSERT(ArmUpstreamCounter(SRMC_SEGMENT_START_TIME, segmentID, 1));
				}
			sendingState = SRSS_ARM_SEGMENT_END;

		case SRSS_ARM_SEGMENT_END:
			if ((flags & VDR_MSMF_SEGMENT_END) != 0 && (flags & VDR_MSCF_SEGMENT_END_NOTIFICATION) != 0)
				{
				STFRES_REASSERT(ArmUpstreamCounter(SRMC_SEGMENT_END, segmentID, numOutputs));
				}
			sendingState = SRSS_ARM_GROUP_START;

		case SRSS_ARM_GROUP_START:
			if ((flags & VDR_MSMF_GROUP_START) != 0 && (flags & VDR_MSCF_GROUP_START_NOTIFICATION) != 0)
				{
				STFRES_REASSERT(ArmUpstreamCounter(SRMC_GROUP_START, groupID, 1));
				}
			sendingState = SRSS_ARM_GROUP_END;

		case SRSS_ARM_GROUP_END:
			if ((flags & VDR_MSMF_GROUP_END) != 0 && (flags & VDR_MSCF_GROUP_END_NOTIFICATION) != 0)
				{
				STFRES_REASSERT(ArmUpstreamCounter(SRMC_GROUP_END, groupID, numOutputs));
				}
			sendingState = SRSS_REPLICATE_PACKETS;

		case SRSS_REPLICATE_PACKETS:
			// Replicating Packets for all output connectors
			while (replicatedPackets < numOutputs && STFRES_SUCCEEDED(result = outputConnectors[replicatedPackets]->GetEmptyDataPacket(pendingOutputPackets[replicatedPackets])))
				{
				pendingOutputPackets[replicatedPackets]->CopyFromVDRPacket(&(packet->vdrPacket));
				pendingOutputPackets[replicatedPackets]->AddRefToRanges();
				replicatedPackets++;
				}

			if (result == STFRES_OBJECT_EMPTY)
				{
				STFRES_RAISE(STFRES_OBJECT_FULL);
				}

			sendingState = SRSS_SEND_PACKETS;

		case SRSS_SEND_PACKETS:
			// Sending replicated packets on all output connectors
			result = STFRES_OK;

			for(i=0; i<numOutputs; i++)
				{
				if (pendingOutputPackets[i])
					{
					result = outputConnectors[i]->SendPacket(pendingOutputPackets[i]);
					if (STFRES_SUCCEEDED(result))
						{
						pendingOutputPackets[i] = NULL;
						deliveredPackets++;
						}
#if 0
					else
						{
						DPR("Bounce for %d\n", i);
						}
#endif
					}
				}

			if (deliveredPackets < numOutputs)
				STFRES_RAISE(STFRES_OBJECT_FULL);

			deliveredPackets = 0;
			replicatedPackets = 0;
			sendingState = SRSS_ARM_SEGMENT_START;
			break;
		}

	STFRES_RAISE_OK;
	}

STFResult VirtualStreamReplicatorStreamingUnit::ProcessFlushing(void)
	{
	uint32 i;

	for(i=0; i<numOutputs; i++)
		{
		if (pendingOutputPackets[i])
			{
			pendingOutputPackets[i]->ReleaseRanges();
			pendingOutputPackets[i]->ReturnToOrigin();
			pendingOutputPackets[i] = NULL;

			deliveredPackets = 0;
			replicatedPackets = 0;
			sendingState = SRSS_ARM_SEGMENT_START;
			}
		}

	STFRES_REASSERT(ResetUpStreamCounters());

	// flush the output formatter
	STFRES_RAISE(StreamingParser::Flush());
	}

VirtualStreamReplicatorStreamingUnit::VirtualStreamReplicatorStreamingUnit(IPhysicalUnit * physical, uint32 numOutputs, uint32 numPacketsPerOutput, StreamReplicatorMessageForwardMode messageMode)
	: VirtualNonthreadedStandardStreamingUnit(physical)
	{
	this->numOutputs = numOutputs;
	this->numPacketsPerOutput = numPacketsPerOutput;
	this->messageMode = messageMode;

	deliveredPackets = 0;
	replicatedPackets = 0;
	sendingState = SRSS_ARM_SEGMENT_START;
	
	pendingOutputPackets = NULL;
	outputConnectors = NULL;
	pendingPacket = NULL;
	flushRequest = false;
	processRequest = false;
	streamingTAGIDs = NULL;
	outputStreamTimes = NULL;
	outputSegmentNumbers = NULL;
	}


VirtualStreamReplicatorStreamingUnit::~VirtualStreamReplicatorStreamingUnit(void)
	{
	uint32	i, j;

	if (outputConnectors)
		{
		for(i=0; i<numOutputs; i++)
			delete outputConnectors[i];

		delete[] outputConnectors;
		}

	delete[] pendingOutputPackets;
	delete[] streamingTAGIDs;
	delete[] outputStreamTimes;
	delete[] outputSegmentNumbers;

	for(i=0; i < NUM_REPLICATOR_UPSTREAM_COUNTERS; i++)
		{
		for(j=0; j<SRMC_TOTAL; j++)
			{
			delete[] upstreamCounters[i][j].id;
			}
		}
	}


STFResult VirtualStreamReplicatorStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamReplicatorStreamingUnit::Initialize(void)
	{
	uint32	i, j;

	STFRES_REASSERT(VirtualUnit::Initialize());

	outputConnectors = new StreamingOutputConnectorPtr[numOutputs];
	pendingOutputPackets = new StreamingDataPacketPtr[numOutputs];
	outputStreamTimes = new uint32[numOutputs];
	outputSegmentNumbers = new uint32[numOutputs];

	//this->AddConnector(&inputConnector);

	for(i=0; i<numOutputs; i++)
		{
		pendingOutputPackets[i] = NULL;
		outputConnectors[i] = NULL;
		}

	for(i=0; i<numOutputs; i++)
		{
		outputConnectors[i] = new StreamingOutputConnector(numPacketsPerOutput, i, this);
		this->AddConnector(outputConnectors[i]);
		}

	for(i=0; i < NUM_REPLICATOR_UPSTREAM_COUNTERS; i++)
		{
		for(j=0; j<SRMC_TOTAL; j++)
			{
			upstreamCounters[i][j].id = new bool[numOutputs];
			}
		}

	STFRES_REASSERT(ResetUpStreamCounters());

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamReplicatorStreamingUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualStreamReplicatorStreamingUnit::PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState)
	{
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			// We need to receive the "startup possible" message from all outputs
			// before we can generate the message towards upstream:		
			startupPossibleCounter	= numOutputs;
			// In order to avoid multiple messages, we remember if a startup was required before. Reset the flag here:
			startupRequiredCounter	= 1;
			break;

		case VDR_STRMCMD_DO:
		case VDR_STRMCMD_FLUSH:
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;

		default:
			DP("*** Unhandled STRMCMD in VirtualStreamReplicatorStreamingUnit::PrepareStreamingCommand! ***\n");
		}

	STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::PrepareStreamingCommand(command, param, targetState));
	}


STFResult VirtualStreamReplicatorStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	// Note: even if we don't want to collect the messages, but send the first incoming in general, we are using TriggerUpstreamCounter()
	//       1. to prevent a certain output from resending one and the same message but launching a warning print.
	//       2. to prevent that one and the same message from all output connectors will be sent upstream
	//lint --e{613}
	switch (message)
		{
		case VDRMID_STRM_START_POSSIBLE:
			// The "start possible" message is only generated if we receive it from all outputs (logical "AND")
			if (--startupPossibleCounter == 0)
				STFRES_REASSERT(inputConnector->SendUpstreamNotification(message, param1, param2));
			break;

		case VDRMID_STRM_START_REQUIRED:
			if (--startupRequiredCounter == 0)
				STFRES_REASSERT(inputConnector->SendUpstreamNotification(message, param1, param2));
			break;

		case VDRMID_STRM_SEGMENT_START:
			// Start messages are collected in "OR" fashion
			outputSegmentNumbers[connectorID] = param1;
			STFRES_REASSERT(TriggerUpstreamCounter(SRMC_SEGMENT_START, param1, connectorID, message, param1, param2));
			break;

		case VDRMID_STRM_SEGMENT_START_TIME:
			// Start messages are collected in "OR" fashion

			STFRES_REASSERT(TriggerUpstreamCounter(SRMC_SEGMENT_START_TIME, outputSegmentNumbers[connectorID], connectorID, message, param1, param2));
			break;

		case VDRMID_STRM_SEGMENT_END:
			// End messages are collected in "AND" fashion
			STFRES_REASSERT(TriggerUpstreamCounter(SRMC_SEGMENT_END, param1, connectorID, message, param1, param2));
			break;

		case VDRMID_STRM_GROUP_START:
			outputStreamTimes[connectorID] += param2;
			STFRES_REASSERT(TriggerUpstreamCounter(SRMC_GROUP_START, param1, connectorID, message, param1, param2));
			break;

		case VDRMID_STRM_GROUP_END:
			outputStreamTimes[connectorID] += param2;
			STFRES_REASSERT(TriggerUpstreamCounter(SRMC_GROUP_END, param1, connectorID, message, param1, param2));
			break;

		default:
			// Default is to let the Standard Streaming Unit handle the message
			// (e.g. packet requests, starvation messages)
			STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::UpstreamNotification(connectorID, message, param1, param2));
		}

	STFRES_RAISE_OK;
	}


#if _DEBUG
STFString VirtualStreamReplicatorStreamingUnit::GetInformation(void)
	{
	// By default, we do not know anything about ourself!
	return STFString("InfiniteStreamReplicator ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif

