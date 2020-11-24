///
/// @file VDR/Source/Streaming/BaseStreamingUnit.cpp
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
/// INTERNAL Implementation File
///
/// Base Implementations for Threaded and Non-Threaded Streaming Units.
/// Generic Data Handling.
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "BaseStreamingUnit.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

#define ENABLE_PACKET_BOMBING 0

#define DBG_VDR_CMD_STRING(c)	((c==VDR_STRMCMD_NONE)?"NONE": \
	(c==VDR_STRMCMD_BEGIN)?"BEGIN": \
	(c==VDR_STRMCMD_DO)?"DO": \
	(c==VDR_STRMCMD_STEP)?"STEP": \
	(c==VDR_STRMCMD_FLUSH)?"FLUSH": \
	"???")
																								 
#define DEBUG_STREAMING_CMD 0

#if _DEBUG && DEBUG_STREAMING_CMD
	#define DP_STREAMCMD(unit, str, cmd) DebugPrint("%s : "str" (%s)\n", (char*) unit->GetInformation(), DBG_VDR_CMD_STRING(cmd)) 
#else 
	#define DP_STREAMCMD(unit, str, cmd) do {} while(0); 
#endif		  


///////////////////////////////////////////////////////////////////////////////
// StandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////


StandardStreamingUnit::StandardStreamingUnit()
	{
	inputConnector			= NULL;

	flushRequest			= false;
	packetBounced			= false;

	changeSet				= 0;
	pendingChangeSet		= 0;
	pendingPacket			= NULL;
	pendingFlags			= 0;
	tagsPending				= false;
	numPendingTags			= 0;
	numSentTags				= 0;
	}

StandardStreamingUnit::~StandardStreamingUnit()
	{
	delete inputConnector;
	}

void StandardStreamingUnit::SetConnectors(IStreamingInputConnector * inputConnector)
	{
	// Add the two connectors to the StreamingUnit external connector array
	this->inputConnector = inputConnector;
	AddConnector(inputConnector);
	}


STFResult StandardStreamingUnit::InternalSynchronizedTagUpdate()
	{   
	uint32 temp;   // Get mask of change set groups that must be handled synchronously   
	uint32 syncMask = GetSynchronousChangeFlags();   
	// Merge the synchronous change set groups to the change set   
	// to be processed synchronously:   
	do 
		{      
		temp = pendingChangeSet;
		} while (pendingChangeSet.CompareExchange((int32)temp, (int32)(temp | (changeSet & syncMask))) != (int32)temp);
		
	// Try to process the pending packet. If there is none or there is   
	// no other thread in ProcessPendingPacket(), the tags will be    
	// updated immediately. Otherwise, the streaming thread currently   
	// in ProcessPendingPacket() will take care of them.   
	// Here we ignore the return value on purpose.   
	this->ProcessPendingPacket();   
	STFRES_RAISE_OK;   
	}


STFResult StandardStreamingUnit::ProcessSynchronizedTags(void)   
	{   
	uint32 temp;   
	do 
		{      
		temp = pendingChangeSet;      
		}   while (pendingChangeSet.CompareExchange((int32) temp, 0) != (int32)temp);

	syncChangeSet |= temp;   
	STFRES_RAISE_OK;   
	}


STFResult StandardStreamingUnit::ProcessPendingPacket(bool lowLatencyCommit)
	{
	STFResult	result = STFRES_OK;
        //lint --e{613}
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
			if (flushRequest)
				{
				result = ProcessFlushing();	// For further flush processing by a derived class
				inputConnector->FlushPackets();


				if (pendingPacket)
					{
					pendingPacket->ReleaseRanges();
					pendingPacket->RemPacketOwner(this);
					pendingPacket->ReturnToOrigin();
					pendingPacket = NULL;
					}

				this->SignalStreamingCommandCompletion(VDR_STRMCMD_FLUSH, result);

				flushRequest = false;
				}
			else 
				{
				// Synchronize tag proecessing with stream processing
				if (pendingChangeSet)
					{
					changeSet = pendingChangeSet;
					pendingChangeSet = 0;
					}

				if (pendingPacket)
					{
					//
					// Attempt to deliver the packet
					//
					result = this->ParseReceivePacket(pendingPacket);					

					if (result != STFRES_OBJECT_FULL)
						{

						//
						// If the packet was consumed, or an error other than object
						// full appeared, remove the packet from the pending state.
						//
						pendingPacket->ReleaseRanges();
						pendingPacket->RemPacketOwner(this);
						pendingPacket->ReturnToOrigin();
						pendingPacket = NULL;

						if (lowLatencyCommit)
							this->LowLatencyCommit();
						}
					else
						{
						//
						// In case of input queue full, we keep the packet and signal
						// an ok.
						//
						result = STFRES_OK;
						}
					}

				if (packetBounced && pendingPacket == NULL)
					{
					// We must reset packetBounced before requesting more packets,
					// otherwise we're flooding the upstream units with packet requests.
					packetBounced = false;
					inputConnector->RequestPackets();
					}
				}
			}
		} while (--pendingLock == 0 && processRequest);

	STFRES_RAISE(result);
	}

STFResult StandardStreamingUnit::LowLatencyCommit(void)
	{
	STFRES_RAISE_OK;
	}

STFResult StandardStreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	packetBounced = false;
	//
	// Process a possibly pending packet
	//
	STFRES_REASSERT(this->ProcessPendingPacket());

	//
	// Process new packet if there is no current pending packet
	//
	packetBounced = true;	// To avoid racing condition, we set it to true here

	if (pendingPacket == NULL)
		{
		//
		// Make the new packet the pending packet and try to process it
		//
		packetBounced = false;
		packet->AddPacketOwner(this);
		pendingPacket = packet;
		STFRES_RAISE(this->ProcessPendingPacket(true));
		}
	else
		{
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}


STFResult StandardStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	//
	// We are not queueing, so nothing we could actually do...
	//
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}


STFResult StandardStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	//lint --e{613}
	//
	// If it is a packet request, we might be able to satisfy it immediately
	//
	if (message == VDRMID_STRM_PACKET_REQUEST || message == VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE)
		{
		// Process a pending packet, this may lead to further packet request upstream notifications
		this->ProcessPendingPacket(true);

		STFRES_RAISE_OK;
		}

	//
	// Forward the request to the upstream filter via the input connector
	//
	STFRES_RAISE(inputConnector->SendUpstreamNotification(message, param1, param2));
	}


STFResult StandardStreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	STFResult result = STFRES_OK;

	DP_STREAMCMD(this, "BeginStreamingCommand", command);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:		
			this->SignalStreamingCommandCompletion(command, result);
			break;

		case VDR_STRMCMD_FLUSH:
			// Call process pending packet in order to get rid of the pending packet.
			flushRequest = true;	// Signal that a flush is requested
			result = ProcessPendingPacket();			
			break;

		default:
			this->SignalStreamingCommandCompletion(command, result);
		}


	STFRES_RAISE(result);
	}

STFResult StandardStreamingUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	STFRES_REASSERT(StreamingUnit::CompleteStreamingCommand(command, targetState));
	if (this->state == VDR_STRMSTATE_READY)
		{
		packetBounced = true;
		ProcessPendingPacket();
		} 

	STFRES_RAISE_OK;
	}



STFResult StandardStreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	STFRES_RAISE(inputConnector->ProvideAllocator(allocator));
	}


//
// Streaming control
//
STFResult StandardStreamingUnit::ParseFlush(void)
	{
	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ParseCommit(void)
	{
	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ParseInterrupted(void)
	{
	STFRES_RAISE(this->ResetParser());
	}


//
// Tag list parsing
//
STFResult StandardStreamingUnit::ParseBeginConfigure(void)
	{
	if (InputPending())
		{
		if (tagsPending)
			{
			DP("### tagsPending flag set - fix me in StandardStreamingUnit::ParseBeginConfigure!\n");
			//STFRES_RAISE(STFRES_OBJECT_FULL);
			}
		else	
			{
			numPendingTags = 0;
			numSentTags = 0;

			tagsPending = true;
			// Set this flag is pending for the next frame to be parsed 
			pendingFlags |= VDR_MSMF_TAGS_VALID; 
			}
		}
	else
		{
		numPendingTags = 0;
		numSentTags = 0;

		tagsPending = false;
		}
	
	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ParseConfigure(TAG *& tags)
	{
	// To check if all the tags are not consumed by this streaming unit
	if (tags)
		{
		while (tags->id != VDRTID_DONE)
			{
			pendingTags[numPendingTags] = *tags;
			tags += tags->skip;
			numPendingTags++;
			}
		}
	else if (tagsPending) 
		tagsPending = false;
		
	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ParseCompleteConfigure(void)
	{
	pendingTags[numPendingTags++] = TAGDONE;

	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ResetInputProcessing(void)
	{
	// This ResetInputProcessing will actually tell the streaming unit that 
	// it needs to flush the last chunk of data which is not yet completed (may be a audio/video frame)
	// because it will never come !!! there is a discontinuity in the stream

	// InputPending() should actually reflect that there is some data or 
	// Properties pending inside the streaming unit needs to be delivered

	// Resetting these pending flags is responsibility of ResetInputProcessing() 
	// if it still has some data pending with which these flags have to be delivered 
	// then they should be kept pending


	// Its the responsibilty of the Flush command to reset every thing in the 
	// Streaming unit i/p, data being processed , processed data (if any) , pending properties
	// everything it can queue


	if (!InputPending())
		{
		numPendingTags			= 0;
		numSentTags				= 0;
		tagsPending				= false;
		}

	STFRES_RAISE_OK;

	}


STFResult StandardStreamingUnit::ResetParser(void)
	{
	if (InputPending())
		{
		numPendingTags			= 0;
		numSentTags				= 0;
		}

	STFRES_RAISE_OK;
	}


STFResult StandardStreamingUnit::ProcessFlushing(void)
	{
	if (InputPending())
		{
		numPendingTags			= 0;
		numSentTags				= 0;
		tagsPending				= false;
		}

	STFRES_RAISE(StreamingParser::Flush());
	}


//
// Range information parsing
//
STFResult StandardStreamingUnit::ParseFrameStart(void)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////


VirtualNonthreadedStandardStreamingUnit::VirtualNonthreadedStandardStreamingUnit(IPhysicalUnit * physical)
	: VirtualUnit(physical)
	{
	// This is merely a workaround for a ST40 compiler bug or limitation
	SetConnectors(new UnqueuedInputConnector(0, this));
	}
		
void VirtualNonthreadedStandardStreamingUnit::ReleaseDestruct(void)
	{
	StandardStreamingUnit::ReleaseDestruct();
	VirtualUnit::ReleaseDestruct();
	}

STFResult VirtualNonthreadedStandardStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


VirtualNonthreadedStandardStreamingUnitCollection::VirtualNonthreadedStandardStreamingUnitCollection(IPhysicalUnit * physical,
																																	  uint32 numChildren)
	: VirtualUnitCollection(physical, numChildren)
	{
	// This is merely a workaround for a ST40 compiler bug or limitation
	SetConnectors(new UnqueuedInputConnector(0, this));
	}


void VirtualNonthreadedStandardStreamingUnitCollection::ReleaseDestruct(void)
	{
	StandardStreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

STFResult VirtualNonthreadedStandardStreamingUnitCollection::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateStreamingTerminator, StreamingTerminatorUnit)


STFResult StreamingTerminatorUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamingTerminatorUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingTerminatorUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	range = num;
	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingTerminatorUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	//lint --e{613}
	DP_STREAMCMD(this, "BeginStreamingCommand", command);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			// Immediately fake the signal that enough data was received to start
			inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
			break;

		case VDR_STRMCMD_DO:
		case VDR_STRMCMD_FLUSH:
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;

		default:
			DP("*** Unhandled STRMCMD in VirtualStreamingTerminatorUnit::BeginStreamingCommand! ***\n");
		}

	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::BeginStreamingCommand(command, param));
	}


STFResult VirtualStreamingTerminatorUnit::ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	// Streaming Terminator will not care about VDRMID_STRM_GROUP_START reporting.
	// This is done by not terminated output with perfect timing, because first report is taken.

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingTerminatorUnit::ParseEndGroup(uint16 groupNumber, bool requestNotification)
	{
	//lint --e{613}
	// Streaming Terminator will immediatelly send VDRMID_STRM_GROUP_START to satisfy decoder counter in demux.
	// Not terminated output will care for perfect timing.
	if (requestNotification)
		inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_END, groupNumber, 0);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingTerminatorUnit::ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	//lint --e{613}
	if (requestNotification)
		inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_START, segmentNumber, 0);

	STFRES_RAISE_OK;
	}


STFResult VirtualStreamingTerminatorUnit::ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{
	//lint --e{613}
	if (requestNotification)
		inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_END, segmentNumber, 0);

	STFRES_RAISE_OK;
	}



#if _DEBUG
STFString VirtualStreamingTerminatorUnit::GetInformation(void)
	{
	return STFString("StreamingTerminator ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif



///////////////////////////////////////////////////////////////////////////////
// Streaming In Out Terminator Unit
///////////////////////////////////////////////////////////////////////////////


UNIT_CREATION_FUNCTION(CreateStreamingInOutTerminator, StreamingInOutTerminatorUnit)


STFResult StreamingInOutTerminatorUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamingInOutTerminatorUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


#if _DEBUG
STFString VirtualStreamingInOutTerminatorUnit::GetInformation(void)
	{
	return STFString("StreamingInOutTerminator ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif



///////////////////////////////////////////////////////////////////////////////
// StandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////

StandardInOutStreamingUnit::StandardInOutStreamingUnit(uint32 numOutPackets)
	: StandardStreamingUnit(),
	  outputConnector(numOutPackets, 0, this)
	{
	groupStartPending		= false;
	groupEndPending		= false;
	groupStartPending		= false;
	startTimePending		= false;
	endTimePending			= false;
	cutDurationPending	= false;
	skipDurationPending	= false;
	}


void StandardInOutStreamingUnit::SetConnectors(IStreamingInputConnector * inputConnector)
	{
	StandardStreamingUnit::SetConnectors(inputConnector);

	AddConnector(&outputConnector);

	// Give the output connector to the output Formatter
	outputFormatter.SetOutputConnector(&outputConnector);
	}

STFResult StandardInOutStreamingUnit::CompleteConnection(void)
	{
	STFRES_REASSERT(StandardStreamingUnit::CompleteConnection());
	STFRES_REASSERT(outputFormatter.CompleteConnection());

	STFRES_RAISE_OK;
	}

STFResult StandardInOutStreamingUnit::ResetParser(void)
	{
	if (InputPending())
		{
		pendingFlags			= 0;
		groupStartPending		= false;
		groupEndPending		= false;
		groupStartPending    = false;
		startTimePending		= false;
		endTimePending			= false;
		cutDurationPending	= false;
		skipDurationPending	= false;
		numPendingTags			= 0;
		numSentTags				= 0;
		}

	STFRES_RAISE_OK;
	}


STFResult StandardInOutStreamingUnit::ResetInputProcessing(void)
	{
	if (!InputPending())
		{
		pendingFlags			= 0;
		groupStartPending		= false;
		groupEndPending		= false;
		groupStartPending    = false;
		startTimePending		= false;
		endTimePending			= false;
		cutDurationPending	= false;
		skipDurationPending	= false;
		numPendingTags			= 0;
		numSentTags				= 0;
		tagsPending				= false;
		}

	STFRES_RAISE_OK;
	}


STFResult StandardInOutStreamingUnit::ProcessFlushing(void)
	{
	if (InputPending())
		{
		pendingFlags			= 0;
		groupEndPending		= false;
		startTimePending		= false;
		endTimePending			= false;
		cutDurationPending	= false;
		skipDurationPending	= false;
		numPendingTags			= 0;
		numSentTags				= 0;
		}

	STFRES_REASSERT(StreamingParser::Flush());
	STFRES_REASSERT(outputFormatter.Flush());

	STFRES_RAISE_OK;
	}

//
// Streaming control
//
STFResult StandardInOutStreamingUnit::ParseFlush(void)
	{
	STFRES_REASSERT(ResetInputProcessing());
	STFRES_RAISE(outputFormatter.Flush());
	}


STFResult StandardInOutStreamingUnit::ParseCommit(void)
	{
	STFRES_RAISE(outputFormatter.Commit());
	}

STFResult StandardInOutStreamingUnit::LowLatencyCommit(void)
	{
	outputFormatter.LowLatencyCommit();

	STFRES_RAISE_OK;
	}


STFResult StandardInOutStreamingUnit::ParseInterrupted(void)
	{
	STFRES_REASSERT(this->ResetParser());

	//
	// Should we inform the output ???
	//

	STFRES_RAISE_OK;
	}


STFResult StandardInOutStreamingUnit::ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.BeginGroup(groupNumber, requestNotification, singleUnitGroup));
		}
	else
		{
		groupStartPending			= true;
		this->groupNumber			= groupNumber;
		this->singleUnitGroup	= singleUnitGroup;
		this->groupStartNotification = requestNotification;

		STFRES_RAISE_OK;
		}
	}


STFResult StandardInOutStreamingUnit::ParseEndGroup(uint16 groupNumber, bool requestNotification)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.CompleteGroup(requestNotification));
		}
	else
		{
		groupEndPending = true;
		groupEndNotification = requestNotification;

		STFRES_RAISE_OK;
		}
	}


//
// Time information
//
STFResult StandardInOutStreamingUnit::ParseStartTime(const STFHiPrec64BitTime & time)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.PutStartTime(time));
		}
	else
		{
		// We cant queue more then one property of a kind
		if(startTimePending)
			STFRES_RAISE(STFRES_OBJECT_FULL);
		else
			{			
			startTimePending = true;
			startTime = time;

			// Set this flag is pending for the next frame to be parsed 
			pendingFlags |= VDR_MSMF_START_TIME_VALID; 
			}

		STFRES_RAISE_OK;
		}
	}


STFResult StandardInOutStreamingUnit::ParseEndTime(const STFHiPrec64BitTime & time)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.PutEndTime(time));
		}
	else
		{
		if(endTimePending)
			STFRES_RAISE(STFRES_OBJECT_FULL);
		else
			{
			endTimePending = true;
			endTime = time;

			// Set this flag is pending for the next frame to be parsed 
			pendingFlags |= VDR_MSMF_END_TIME_VALID; 
			}
		STFRES_RAISE_OK;
		}
	}


STFResult StandardInOutStreamingUnit::ParseCutDuration(const STFHiPrec32BitDuration & duration)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.PutCutDuration(duration));
		}
	else
		{
		// We cant queue more then one property of a kind
		if(cutDurationPending)
			STFRES_RAISE(STFRES_OBJECT_FULL);
		else
			{			
			cutDurationPending = true;
			cutDuration = duration;

			// Set this flag is pending for the next frame to be parsed 
			pendingFlags |= VDR_MSCF_CUT_AFTER; 
			}

		STFRES_RAISE_OK;
		}
	}


STFResult StandardInOutStreamingUnit::ParseSkipDuration(const STFHiPrec32BitDuration & duration)
	{
	if (!InputPending())
		{
		STFRES_RAISE(outputFormatter.PutSkipDuration(duration));
		}
	else
		{
		// We cant queue more then one property of a kind
		if(skipDurationPending)
			STFRES_RAISE(STFRES_OBJECT_FULL);
		else
			{			
			skipDurationPending = true;
			skipDuration = duration;

			// Set this flag is pending for the next frame to be parsed 
			pendingFlags |= VDR_MSCF_SKIP_UNTIL; 
			}

		STFRES_RAISE_OK;
		}
	}


STFResult StandardInOutStreamingUnit::GetStreamTagIDs(uint32 connectorID, VDRTID * & ids)
	{
	STFRES_RAISE(outputConnector.GetStreamTagIDs(ids));
	}


///////////////////////////////////////////////////////////////////////////////
// NonthreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////

NonthreadedStandardInOutStreamingUnit::NonthreadedStandardInOutStreamingUnit(uint32 numOutPackets)
	: StandardInOutStreamingUnit(numOutPackets)
	{
	// This is merely a workaround for a ST40 compiler bug or limitation
	SetConnectors(new UnqueuedInputConnector(0, this));
	}


STFResult NonthreadedStandardInOutStreamingUnit::ParseDataDiscontinuity(void)
	{
	//
	// Data shall not be contiguous from here,
	STFRES_REASSERT(this->ResetInputProcessing());

	//if you are not queuing anything in streaming unit then you 
	//should not have any pending inputs we cant queue any discontinuity 
	//we need to process pending data first

	if (this->InputPending())		 
		STFRES_RAISE(STFRES_OBJECT_FULL);

	STFRES_RAISE(outputFormatter.PutDataDiscontinuity());
	}


STFResult NonthreadedStandardInOutStreamingUnit::ParseTimeDiscontinuity(void)
	{
	STFRES_REASSERT(this->ResetInputProcessing());

	if (this->InputPending())		 
		STFRES_RAISE(STFRES_OBJECT_FULL);
	
	STFRES_RAISE(outputFormatter.PutTimeDiscontinuity());
	}


STFResult NonthreadedStandardInOutStreamingUnit::ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());

	if (this->InputPending())		 
		STFRES_RAISE(STFRES_OBJECT_FULL);
	
	STFRES_RAISE(outputFormatter.BeginSegment(segmentNumber, requestNotification));
	}


STFResult NonthreadedStandardInOutStreamingUnit::ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());

	if (this->InputPending())		 
		STFRES_RAISE(STFRES_OBJECT_FULL);

	STFRES_RAISE(outputFormatter.CompleteSegment(requestNotification));
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////


VirtualNonthreadedStandardInOutStreamingUnit::VirtualNonthreadedStandardInOutStreamingUnit (IPhysicalUnit * physical, uint32 numOutPackets)
	: VirtualUnit(physical),
     NonthreadedStandardInOutStreamingUnit(numOutPackets)
	  
	{
	}
		

STFResult VirtualNonthreadedStandardInOutStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}

void VirtualNonthreadedStandardInOutStreamingUnit::ReleaseDestruct(void)
	{
	NonthreadedStandardInOutStreamingUnit::ReleaseDestruct();
	VirtualUnit::ReleaseDestruct();
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardInOutStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


VirtualNonthreadedStandardInOutStreamingUnitCollection::VirtualNonthreadedStandardInOutStreamingUnitCollection(IPhysicalUnit * physical,
																																				   uint32 numChildren, 
																																					uint32 numOutPackets)
	: VirtualUnitCollection(physical, numChildren),
     NonthreadedStandardInOutStreamingUnit(numOutPackets)
	{
	}


STFResult VirtualNonthreadedStandardInOutStreamingUnitCollection::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}

void VirtualNonthreadedStandardInOutStreamingUnitCollection::ReleaseDestruct(void)
	{
	NonthreadedStandardInOutStreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}


///////////////////////////////////////////////////////////////////////////////
// ThreadedBaseStreamingUnit
///////////////////////////////////////////////////////////////////////////////


ThreadedStandardStreamingUnit::ThreadedStandardStreamingUnit(uint32 inQueueSize, 
																				 int32 inThreshold,
																				 STFString threadName)
	: StandardStreamingUnit(),
	  STFThread(threadName)
	{
	// This is merely a workaround for a ST40 compiler bug or limitation
	SetConnectors(new QueuedInputConnector(inQueueSize, inThreshold, 0, this));
	}


void ThreadedStandardStreamingUnit::ThreadEntry(void)
	{
	// Don't do anything until we receive data (or are requested for more data)
	// for the first time.
	WaitThreadSignal();

	while (!terminate)
		{
		// Process a possibly pending packet
		if (ProcessPendingPacket() != STFRES_OBJECT_FULL && (!pendingPacket || !packetBounced))
			{
			// Request more data at the input connector if all pending data was sent/consumed, 
			inputConnector->RequestPackets();
			}
		else
			{
			}

		if (!flushRequest)
			CompleteOutputProcessing();		


		// Wait for the next packet or packet request from further downstream to arrive
		WaitThreadSignal();
		}
	}


STFResult ThreadedStandardStreamingUnit::NotifyThreadTermination(void)
	{
	SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	// Wake up the thread in order to retrieve and process the new data
	SetThreadSignal();

	STFRES_RAISE_OK;
	}

STFResult ThreadedStandardStreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{	
	STFResult result = STFRES_OK;

	DP_STREAMCMD(this, "BeginStreamingCommand",command);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:			
			this->SignalStreamingCommandCompletion(command, result);
			break;

		case VDR_STRMCMD_FLUSH:
			// We have to let the units thread perform the actual flush
			// to avoid racing conditions
			flushRequest = true;	// Signal that a flush is requested
			SetThreadSignal();
			break;

		default:
			this->SignalStreamingCommandCompletion(command, result);
		}


	STFRES_RAISE(result);
	}

STFResult ThreadedStandardStreamingUnit::CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState)
	{
	STFRES_REASSERT(StreamingUnit::CompleteStreamingCommand(command, targetState));
	if (this->state == VDR_STRMSTATE_READY)
		{
		packetBounced = true;
		SetThreadSignal();
		}

	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardStreamingUnit::ParseDataDiscontinuity(void)
	{
	//
	// Data shall not be contiguous from here,
	//
	STFRES_REASSERT(this->ResetInputProcessing());
	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardStreamingUnit::ParseTimeDiscontinuity(void)
	{
	STFRES_REASSERT(this->ResetInputProcessing());
	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardStreamingUnit::ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());
	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardStreamingUnit::ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());
	STFRES_RAISE_OK;
	}

///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnit
///////////////////////////////////////////////////////////////////////////////


VirtualThreadedStandardStreamingUnit::VirtualThreadedStandardStreamingUnit(IPhysicalUnit * physical, 
														  uint32 inQueueSize,
														  int32 inThreshold,
														  STFString threadName)
	: VirtualUnit(physical), ThreadedStandardStreamingUnit(inQueueSize, inThreshold, threadName)
	{
	}
		
void VirtualThreadedStandardStreamingUnit::ReleaseDestruct(void)
	{
	ThreadedStandardStreamingUnit::ReleaseDestruct();
	VirtualUnit::ReleaseDestruct();
	}

STFResult VirtualThreadedStandardStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualNonthreadedStandardStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////


VirtualThreadedStandardStreamingUnitCollection::VirtualThreadedStandardStreamingUnitCollection(IPhysicalUnit * physical,
																																	  uint32 numChildren, 
														  uint32 inQueueSize,
														  int32 inThreshold,
														  STFString threadName)
	: VirtualUnitCollection(physical, numChildren),
	  ThreadedStandardStreamingUnit(inQueueSize, inThreshold, threadName)
	{
	}


void VirtualThreadedStandardStreamingUnitCollection::ReleaseDestruct(void)
	{
	ThreadedStandardStreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

STFResult VirtualThreadedStandardStreamingUnitCollection::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}



ThreadedStandardInOutStreamingUnit::ThreadedStandardInOutStreamingUnit(uint32 numOutPackets, 
																							  uint32 inQueueSize, 
																							  int32 inThreshold,
																							  STFString threadName)
	: StandardInOutStreamingUnit(numOutPackets),
	  STFThread(threadName)
	{
	// This is merely a workaround for a ST40 compiler bug or limitation
	SetConnectors(new QueuedInputConnector(inQueueSize, inThreshold, 0, this));
	}


void ThreadedStandardInOutStreamingUnit::ThreadEntry(void)
	{
	// Don't do anything until we receive data (or are requested for more data)
	// for the first time.
	WaitThreadSignal();

	while (!terminate)
		{
		// Process a possibly pending packet
		if (ProcessPendingPacket() != STFRES_OBJECT_FULL && (!pendingPacket || !packetBounced))
			{
			// Request more data at the input connector if all pending data was sent/consumed, 
			inputConnector->RequestPackets();			
			}
		else
			{
			// Do processing like delivery of produced data even if there is no more
			// input data - the thread could potentially be woken up by an MME command
			// completion callback...
			}

		if (!flushRequest)
			CompleteOutputProcessing();


		// Wait for the next packet or packet request from further downstream to arrive
		WaitThreadSignal();
		}
	}


STFResult ThreadedStandardInOutStreamingUnit::NotifyThreadTermination(void)
	{
	SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardInOutStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	// Wake up the thread in order to retrieve and process the new data
	SetThreadSignal();

	STFRES_RAISE_OK;
	}


STFResult ThreadedStandardInOutStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	//lint --e{613}
	//
	// If it is a packet request, we might be able to satisfy it immediately
	//
	if (message == VDRMID_STRM_PACKET_REQUEST || message == VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE)
		{
		// Signal the thread to kick off further processing and delivery of data.
		SetThreadSignal();
		STFRES_RAISE_OK;
		}

	//
	// Otherwise forward the request to the upstream filter via the input connector
	//
	STFRES_RAISE(inputConnector->SendUpstreamNotification(message, param1, param2));
	}

STFResult ThreadedStandardInOutStreamingUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	STFResult result = STFRES_OK;

	DP_STREAMCMD(this, "BeginStreamingCommand",command);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:			
			this->SignalStreamingCommandCompletion(command, result);
			break;

		case VDR_STRMCMD_FLUSH:
			// We have to let the units thread perform the actual flush
			// to avoid racing conditions
			flushRequest = true;	// Signal that a flush is requested
			SetThreadSignal();
			break;

		default:
			this->SignalStreamingCommandCompletion(command, result);
		}


	STFRES_RAISE(result);
	}


STFResult ThreadedStandardInOutStreamingUnit::ParseDataDiscontinuity(void)
	{
	//
	// Data shall not be contiguous from here,
	//
	STFRES_REASSERT(this->ResetInputProcessing());
	if (this->InputPending())
		STFRES_RAISE(STFRES_OBJECT_FULL);
	STFRES_RAISE(outputFormatter.PutDataDiscontinuity());
	}


STFResult ThreadedStandardInOutStreamingUnit::ParseTimeDiscontinuity(void)
	{
	STFRES_REASSERT(this->ResetInputProcessing());
	if (this->InputPending())
		STFRES_RAISE(STFRES_OBJECT_FULL);
	STFRES_RAISE(outputFormatter.PutTimeDiscontinuity());
	}


STFResult ThreadedStandardInOutStreamingUnit::ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());

	if (this->InputPending())
		STFRES_RAISE(STFRES_OBJECT_FULL);
	STFRES_RAISE(outputFormatter.BeginSegment(segmentNumber, requestNotification));
	}


STFResult ThreadedStandardInOutStreamingUnit::ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{
	STFRES_REASSERT(this->ResetInputProcessing());

	if (this->InputPending())
		STFRES_RAISE(STFRES_OBJECT_FULL);
	STFRES_RAISE(outputFormatter.CompleteSegment(requestNotification));
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardInOutStreamingUnit
///////////////////////////////////////////////////////////////////////////////

VirtualThreadedStandardInOutStreamingUnit::VirtualThreadedStandardInOutStreamingUnit(IPhysicalUnit * physical,
																												 uint32 numOutPackets,
																												 uint32 inQueueSize,
																												 int32 inThreshold,
																												 STFString threadName)
	: VirtualUnit(physical),
     ThreadedStandardInOutStreamingUnit(numOutPackets,
                                        inQueueSize,
                                        inThreshold,
                                        threadName)
	{
	}
		

STFResult VirtualThreadedStandardInOutStreamingUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}

void VirtualThreadedStandardInOutStreamingUnit::ReleaseDestruct(void)
	{
	ThreadedStandardInOutStreamingUnit::ReleaseDestruct();
	VirtualUnit::ReleaseDestruct();
	}


///////////////////////////////////////////////////////////////////////////////
// VirtualThreadedStandardInOutStreamingUnitCollection
///////////////////////////////////////////////////////////////////////////////

VirtualThreadedStandardInOutStreamingUnitCollection::VirtualThreadedStandardInOutStreamingUnitCollection(IPhysicalUnit * physical,
																																		   uint32 numChildren,
																																		   uint32 numOutPackets,
																																		   uint32 inQueueSize,
																																		   int32 inThreshold,
																																		   STFString threadName)
	: VirtualUnitCollection(physical, numChildren),
     ThreadedStandardInOutStreamingUnit(numOutPackets,
                                        inQueueSize,
                                        inThreshold,
                                        threadName)
	{
	}


STFResult VirtualThreadedStandardInOutStreamingUnitCollection::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_STREAMING_UNIT, IStreamingUnit);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}

void VirtualThreadedStandardInOutStreamingUnitCollection::ReleaseDestruct(void)
	{
	ThreadedStandardInOutStreamingUnit::ReleaseDestruct();
	VirtualUnitCollection::ReleaseDestruct();
	}

