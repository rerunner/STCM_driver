///
/// @brief  UnitSet Object Interface
///

#ifndef IVDRUNITSET_H
#define IVDRUNITSET_H

#include "VDR/Interface/Unit/IVDRUnit.h"

///////////////////////////////////////////////////////////////////////////////
// UnitSet Interface
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_VDR_UNITSET = 0x0000000b;

/// @brief "UnitSet" Interface
/// 
/// Access to this interface is gained by creating a unit set using 
/// IVDRUnitSetFactory.CreateUnitSet().
class IVDRUnitSet : public virtual IVDRVirtualUnit
	{
	public:
		/// @brief Query for the interface of a unit
		/// 
		/// With this function, we can query for interfaces a particular unit (specified by unitID)
		/// of the UnitSet offers.
		/// UnitSet interfaces are based on IVDRBase, so they can be released using that interface.
		/// 
		/// Using QueryUnitInterface on a IVDRUnitSet instance, we can retrieve access 
		/// to interfaces which a specific unit within a Unit Set offers. This way, we 
		/// can allow several units of the same type within a UnitSet.
		///
		/// @param unitID  Public unit ID of unit from which to query the interface
		/// @param uifID   Public ID of requested interface
		/// @param ifp     Reference to pointer receiving the interface
		/// @returns       STFRES_INTERFACE_NOT_FOUND or STFRES_OK if successful
		virtual STFResult QueryUnitInterface(VDRUID unitID, VDRIID uifID, void *& ifp) = 0;
	};

#endif // #ifndef IVDRUNITSET_H

