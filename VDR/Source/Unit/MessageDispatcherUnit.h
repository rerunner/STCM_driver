///
/// @brief      Message Dispatcher Unit
///

#ifndef MESSAGEDISPATCHERUNIT_H
#define MESSAGEDISPATCHERUNIT_H

#include "IMessageDispatcherUnit.h"
#include "VirtualUnit.h"
#include "PhysicalUnit.h"


class PhysicalMessageDispatcherUnit : public SharedPhysicalUnit
	{
	friend class VirtualMessageDispatcherUnit;
	protected:
		STFMessageProcessorThread				*	messageThread;
		TriggeredMasterSTFMessageProcessor	*	messageProcessor;
	public:
		PhysicalMessageDispatcherUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};


class VirtualMessageDispatcherUnit : 	public virtual IMessageDispatcherUnit,
													public VirtualUnit, TriggeredQueuedSTFMessageProcessingDispatcher
	{
	protected:
		PhysicalMessageDispatcherUnit *	physical;
		bool										attached;

		//
		// From TriggeredQueuedSTFMessageProcessingDispatcher
		//
		void Trigger(void);

		//
		// From VirutalUnit
		//
		virtual STFResult PreemptUnit(uint32 flags);

		virtual STFResult	InternalUpdate(void);
	public:
		VirtualMessageDispatcherUnit(PhysicalMessageDispatcherUnit * physical);

		//
		// IMessageDispatcherUnit implementation
		//
		virtual STFResult GetDispatcher(STFMessageDispatcher * & dispatcher);
	
		// VirtualUnit::QueryInterface override
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);
	};

#endif
