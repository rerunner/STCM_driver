//
// PURPOSE:		Unit Set Factory interface - exposed by VDR Driver Component
//

/*! \file
	\brief VDR Driver Component Interfaces
*/

#ifndef IVDRUNITSETFACTORY_H
#define IVDRUNITSETFACTORY_H

#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Unit/IVDRUnitSet.h"

#include <stdarg.h>

///////////////////////////////////////////////////////////////////////////////
// "UnitSet Factory" Interface
///////////////////////////////////////////////////////////////////////////////

// Marks end of list of requested units for CreateUnitSet... functions
#define	VDR_UNITS_DONE	0


// Interface ID definition
static const VDRIID VDRIID_VDR_UNITSET_FACTORY = 0x0000000c;



//! "UnitSet Factory" Interface
/*!
	Offers functions to create UnitSet instances and ask for available units
*/
class IVDRUnitSetFactory : public virtual IVDRBase
	{
	public:
		virtual STFResult AvailableUnits(VDRUID * availableUnits, int & num) = 0;
		virtual STFResult CreateUnitSetList(IVDRUnitSet * & units, VDRUID * requnits) = 0;

		// Modfied to workaround a compiler bug
		STFResult CreateUnitSet(IVDRUnitSet * & units, VDRUID requnits, ...)
			{
			va_list marker;
			VDRUID args[32];
			int i = 0;

			args[i] = requnits;

			va_start (marker, requnits);

			while (args[i] != VDR_UNITS_DONE && i < 32)
				{
				i++;
				args[i] = va_arg (marker, VDRUID);
				}

			va_end (marker);

			if (i >= 32)
				STFRES_RAISE(STFRES_RANGE_VIOLATION);

			return CreateUnitSetList(units, &args[0]);
			}
	};


#endif // #ifndef IVDRUNITSETFACTORY_H
