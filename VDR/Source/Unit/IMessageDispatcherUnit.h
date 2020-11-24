#ifndef IMESSAGEDISPATCHERUNIT_H
#define IMESSAGEDISPATCHERUNIT_H

#include "VDR/Interface/Base/IVDRBase.h"
#include "STF/Interface/Types/STFMessage.h"

static const VDRIID VDRIID_MESSAGE_DISPATCHER = 0x8000001c;

class IMessageDispatcherUnit : public virtual IVDRBase
	{
	public:
		virtual STFResult GetDispatcher(STFMessageDispatcher * & dispatcher) = 0;
	};

#endif
