///
/// @file       VDR/Source/Unit/MessageDispatcherUnit.cpp
///
/// @brief      Message Dispatcher Unit Implementation
///
/// @author     U. Sigmund
///
/// @par OWNER: VDR Architecture Team
///
/// @par SCOPE: INTERNAL Implementation File
///
/// @date       2003-01-24
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "MessageDispatcherUnit.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

///////////////////////////////////////////////////////////////////////////////
// Unit creation function implementation
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateMessageDispatcher, PhysicalMessageDispatcherUnit)


///////////////////////////////////////////////////////////////////////////////
// PhysicalMessageDispatcherUnit
///////////////////////////////////////////////////////////////////////////////

STFResult PhysicalMessageDispatcherUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = new VirtualMessageDispatcherUnit(this);

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult PhysicalMessageDispatcherUnit::Create(uint64 * createParams)
	{
	uint32 threadPriority;
	uint32 threadStackSize;
	char	 *threadName;
	
	STFRES_REASSERT(GetDWordParameter(createParams, 0, threadPriority));
	STFRES_REASSERT(GetDWordParameter(createParams, 1, threadStackSize));
	STFRES_REASSERT(GetStringParameter(createParams,2, threadName));

	messageProcessor = new TriggeredMasterSTFMessageProcessor();
	messageThread = new STFMessageProcessorThread(STFString(threadName) + STFString(GetUnitID(), 8, 16), 
																 threadStackSize,
																 (STFThreadPriority) threadPriority,
																 messageProcessor);

	STFRES_RAISE_OK;
	}


STFResult PhysicalMessageDispatcherUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalMessageDispatcherUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}



///////////////////////////////////////////////////////////////////////////////
// VirtualMessageDispatcherUnit
///////////////////////////////////////////////////////////////////////////////

void VirtualMessageDispatcherUnit::Trigger(void)
	{
	if (attached)
		physical->messageProcessor->TriggerMessageArrival();
	}


STFResult VirtualMessageDispatcherUnit::PreemptUnit(uint32 flags)
	{
	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		if (!attached)
			{
			physical->messageProcessor->AttachSlave(this);
			attached = true;
			}
		}
	else if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		if (attached)
			{
			attached = false;
			physical->messageProcessor->RemoveSlave(this);
			}
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualMessageDispatcherUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


VirtualMessageDispatcherUnit::VirtualMessageDispatcherUnit(PhysicalMessageDispatcherUnit * physical_)
	: VirtualUnit(physical_), 
	  TriggeredQueuedSTFMessageProcessingDispatcher(16), 
	  physical(physical_)
	{
	attached = false;
	}


STFResult VirtualMessageDispatcherUnit::GetDispatcher(STFMessageDispatcher * & dispatcher)
	{
	dispatcher = this;

	STFRES_RAISE_OK;
	}


STFResult VirtualMessageDispatcherUnit::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_MESSAGE_DISPATCHER, IMessageDispatcherUnit);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}
