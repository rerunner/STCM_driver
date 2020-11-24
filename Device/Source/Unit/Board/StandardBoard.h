#ifndef STANDARDBOARD_H
#define STANDARDBOARD_H

///
/// @brief      Standard Board Implementation
///

#include "VDR/Source/Unit/UnitCollection.h"
#include "VDR/Source/Unit/IUnitSet.h"
#include "VDR/Interface/Unit/IVDRUnitSetFactory.h"


/// @brief Standard Implementation of a Hardware Board with several Physical Units
///
/// While being itself a specialized Physical Unit, this class implements a
/// collection of PhysicalUnits representing the Factory References it has
/// to other Physical Units.
/// It also implements the IVDRUnitSetFactory interface for the VDR Driver
/// Component, by which it allows applications on VDR to open Unit Sets
/// with selected Virtual Units created from the Physical Units which
/// the board has references to.
class StandardBoard : public PhysicalUnit,
							 public PhysicalUnitCollection,
							 public virtual IVDRUnitSetFactory
	{
	friend class VirtualStandardBoard;

	protected:
		uint32 boardVersion;
		uint32 boardRevision;
		uint32 driverVersion;
		uint32 driverRevision;
		uint32 driverBuild;

		/// @brief This function is called when the Reference Counter of the VDRBase implementation becomes 0
		/// It can be overloaded by specific subclasses to do some system-specific cleanup.
		virtual STFResult ShutDown(void);

	public:
		StandardBoard(VDRUID unitID) : PhysicalUnit(unitID)
			{
			boardVersion	= 0xbadebade;
			boardRevision	= 0xbadebade;
			driverVersion	= 0xbadebade;
			driverRevision	= 0xbadebade;
			driverBuild		= 0xbadebade;
			}
		//
		// IVDRBase interface implementation
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult InternalConfigureTags(TAG * tags);
		virtual STFResult	InternalUpdate(void);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult GetTagIDs(VDRTID * & ids);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult ActivateAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime);
		virtual STFResult Unlock(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime);
		virtual STFResult Passivate(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime);
		virtual STFResult UnlockAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime);
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		// Called during Allocation Phase of System Construction Process
		virtual STFResult Create(uint64 * createParams);
		// Called during Connection Phase of System Construction Process
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		// Called during Initialisation Phase of System Construction Process
		virtual STFResult Initialize(uint64 * depUnitsParams);
	
		//
		// IVDRUnitSetFactory interface implementation
		//
		virtual STFResult AvailableUnits(VDRUID * availableUnits, int & num);
		virtual STFResult CreateUnitSetList(IVDRUnitSet * & units, VDRUID * requnits);

		// Commented out to workaround a gcc bug
		//virtual STFResult CreateUnitSet(IVDRUnitSet * & units, VDRUID requnits, ...);
	};



/// @brief Standard Implementation of a Virtual Board Unit representing the UnitSet
/// A VirtualStandardBoard created from the one Physical StandardBoard is the
/// object representing a UnitSet to the application.
/// When an application requests a UnitSet from IVDRUnitSet (implemented by
/// the StandardBoard Physical Unit), then a new VirtualStandardBoard is
/// created, whose VirtualUnitCollection is filled with Virtual Units of
/// exactly those units requested by the application. Activation and locking
/// of these Virtual Units is generically implemented in VirtualUnitCollection.
/// This class also implements a generic scheme for distributing Tags to the
/// Virtual Units in the UnitSet.
class VirtualStandardBoard : public VirtualUnitCollection,
									  public virtual IUnitSet
									  
	{
	friend class StandardBoard;

	private:
		StandardBoard * physicalUnit;
	
	protected:
			virtual STFResult AllocateChildUnits(void);

	public:
		VirtualStandardBoard(StandardBoard * physicalUnit, int numSubUnits) : VirtualUnitCollection(physicalUnit, numSubUnits)
			{
			this->physicalUnit = physicalUnit;
			}

		//
		// IVDRBase interface implementation
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);


		// ITagUnit interface implementation
		//
		virtual STFResult GetTagIDs(VDRTID * & ids);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult InternalConfigureTags(TAG * tags);
		virtual STFResult	InternalUpdate(void);

		//
		// IVDRUnitSet interface implementation
		//
		virtual STFResult QueryUnitInterface(VDRUID unitID, VDRIID uifID, void *& ifp);

		//
		// IUnitSet interface implementation
		//
		virtual STFResult CreateUnitSet(uint32 * reqUnits);

	};

#endif	// #ifndef STANDARDBOARD_H

