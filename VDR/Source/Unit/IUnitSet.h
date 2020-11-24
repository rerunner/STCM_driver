//
// PURPOSE:		Internal UnitSet Interface
//

#ifndef IUNITSET_H
#define IUNITSET_H

#include "VDR/Interface/Unit/IVDRUnitSet.h"

//! Internal Unit Set Interface ID
static const VDRIID VDRIID_UNITSET = 0x80000004;

//! Internal Unit Set Interface
class IUnitSet : public virtual IVDRUnitSet
	{
	public:
		//! Initialize a Unit Set, leading to creation of requested Virtual Units
		virtual STFResult CreateUnitSet(uint32 * reqUnits) = 0;
	};

#endif	// #ifndef IUNITSET_H

