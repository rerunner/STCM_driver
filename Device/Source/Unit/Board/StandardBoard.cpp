///
/// @brief      Standard Board Implementation
///

#include "StandardBoard.h"
#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "VDR/Interface/Unit/IVDRUnitSetFactory.h"
#include "VDR/Interface/Unit/Board/IVDRBoard.h"
#include "VDR/Source/Unit/ITagUnit.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/STFDebug.h"


UNIT_CREATION_FUNCTION(CreateStandardBoard, StandardBoard)


// Standard Implementation of ShutDown function
STFResult StandardBoard::ShutDown(void)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// StandardBoard IVDRBase interface implementation
///////////////////////////////////////////////////////////////////////////////


STFResult StandardBoard::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_VDR_UNITSET_FACTORY, IVDRUnitSetFactory);
	VDRQI_END(PhysicalUnit);

	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// IVDRTagUnit interface implementation
///////////////////////////////////////////////////////////////////////////////

//
// ITagUnit interface implementation
//
STFResult StandardBoard::GetTagIDs(VDRTID * & ids)
	{
	static const VDRTID StandardBoardTagTypeIDs[] = 
		{
		VDRTID_BOARD,
		VDRTID_DONE
		};

	ids = (VDRTID*) &StandardBoardTagTypeIDs[0];

	STFRES_RAISE_OK;
	}


STFResult StandardBoard::InternalConfigureTags(TAG * tags)
	{
	DP("StandardBoard::InternalConfigureTags\n");
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::InternalUpdate(void)
	{
	DP("StandardBoard::InternalUpdate\n");
	STFRES_RAISE_OK;
	}




///////////////////////////////////////////////////////////////////////////////
// StandardBoard IPhysicalUnit interface implementation
///////////////////////////////////////////////////////////////////////////////

STFResult StandardBoard::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::ActivateAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	// Empty dummy implementation. Should never be called.
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::Unlock(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	// Empty dummy implementation. Should never be called.
	STFRES_RAISE_OK;
	}
	

STFResult StandardBoard::Passivate(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	// Empty dummy implementation. Should never be called.
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::UnlockAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	// Empty dummy implementation. Should never be called.
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::Create(uint64 * createParams)
	{
	STFRES_REASSERT(GetDWordParameter(createParams, 0, boardVersion));
	STFRES_REASSERT(GetDWordParameter(createParams, 1, boardRevision));
	STFRES_REASSERT(GetDWordParameter(createParams, 2, driverVersion));
	STFRES_REASSERT(GetDWordParameter(createParams, 3, driverRevision));
	STFRES_REASSERT(GetDWordParameter(createParams, 4, driverBuild));

	STFRES_RAISE_OK;
	}


STFResult StandardBoard::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_REASSERT(PhysicalUnitCollection::AddUnit(localID, source));
	STFRES_RAISE_OK;
	}


STFResult StandardBoard::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// StandardBoard IVDRUnitSetFactory interface implementation
///////////////////////////////////////////////////////////////////////////////
	

STFResult StandardBoard::AvailableUnits(VDRUID * availableUnits, int & num)
	{
	STFRES_RAISE(PhysicalUnitCollection::AvailableUnits(availableUnits, num));
	}


		// Commented out to workaround a gcc bug
/*STFResult StandardBoard::CreateUnitSet(IVDRUnitSet * & units, VDRUID requnits, ...)
	{
	STFRES_RAISE(CreateUnitSetList(units, &requnits));
	}*/


STFResult StandardBoard::CreateUnitSetList(IVDRUnitSet * & units, VDRUID * reqUnits)
	{
	int numReqUnits;
	STFResult err = STFRES_OK;
//	IVirtualUnit * virtualBoard;
	VirtualStandardBoard * virtualBoard;
	IUnitSet * unitSetInterface = NULL;

//	GNREASSERT(this->CreateVirtual(virtualBoard, NULL, NULL));

	// Count the number of requested units
	numReqUnits = 0;
	while (reqUnits[numReqUnits] != VDR_UNITS_DONE)
		numReqUnits++;

	virtualBoard = new VirtualStandardBoard(this, numReqUnits);

	if (virtualBoard != NULL)
		{
		// Get internal Unit Set interface by which we can add Virtual Units
		err = virtualBoard->QueryInterface(VDRIID_UNITSET, (void*&) unitSetInterface);
		if (!STFRES_IS_ERROR(err))
			{
			// Create the Unit Set from the requested unit IDs
			err = unitSetInterface->CreateUnitSet(reqUnits);

			// Get access to the IVDRUnitSet of the new Unit Set
			if (!STFRES_IS_ERROR(err))
				{
				err = virtualBoard->QueryInterface(VDRIID_VDR_UNITSET, (void*&) units);

				if (!STFRES_IS_ERROR(err))
					{
					//??? Dietmar's comment: We have a problem here. The reference counter of virtualBoard is
					// two after the QueryInterface(), though it should be one so that on the final Release(),
					// the unit set gets deleted. To force the counter down to one, we Release() here already.
					// BAD TRICK.
					virtualBoard->Release ();
					}

				// Release internal Unit Set interface
				unitSetInterface->Release();
				}
			}
		else
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		}

#if _DEBUG
	if (STFRES_IS_ERROR(err))
		DP("### Fatal error %08x in StandardBoard::CreateUnitSetList!\n", err);
#endif

	STFRES_RAISE(err);
	}





///////////////////////////////////////////////////////////////////////////////
// VirtualStandardBoard IVDRBase interface implementation
///////////////////////////////////////////////////////////////////////////////

STFResult VirtualStandardBoard::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_VDR_UNITSET,	IVDRUnitSet);
		VDRQI_IMPLEMENT(VDRIID_UNITSET,		IUnitSet);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}

//
// ITagUnit interface implementation
//
STFResult VirtualStandardBoard::GetTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE_OK;
	}


//
// ITagUnit interface implementation
//
STFResult VirtualStandardBoard::InternalConfigureTags(TAG * tags)
	{
	uint32 changeSet; // just temporary

	// Call our base class.
	STFRES_REASSERT(VirtualUnitCollection::InternalConfigureTags (tags));

//	DP("VirtualStandardBoard::InternalConfigureTags\n");

	PARSE_TAGS_START(tags, changeSet)
		GETONLY(BOARD_VERSION,		physicalUnit->boardVersion);
		GETONLY(BOARD_REVISION,		physicalUnit->boardRevision);
		GETONLY(DRIVER_VERSION,		physicalUnit->driverVersion);
		GETONLY(DRIVER_REVISION,	physicalUnit->driverRevision);
		GETONLY(DRIVER_BUILD,		physicalUnit->driverBuild);
	PARSE_TAGS_END

	STFRES_RAISE_OK;
	}

STFResult VirtualStandardBoard::InternalUpdate(void)
	{
//	DP("VirtualStandardBoard::InternalUpdate\n");
	STFRES_RAISE_OK;
	}


//
// IVDRUnitSet interface implementation
//

STFResult VirtualStandardBoard::QueryUnitInterface(VDRUID unitID, VDRIID uifID, void *& ifp)
	{
	bool found = false;
	int i = 0;

	while (!found && i < VirtualUnitCollection::numLeafUnits)
		{
		//DP("Searching for unit 0x%08x, current: 0x%08x\n", unitID, leafUnits[i]->GetUnitID());
		found = (unitID == VirtualUnitCollection::leafUnits[i]->GetUnitID());
		i++;
		}

	if (found)
		STFRES_RAISE(VirtualUnitCollection::leafUnits[i - 1]->QueryInterface(uifID, ifp));
	else
		{
		DP("Unit ID 0x%08x not found in unit collection\n", unitID);
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
		}

	STFRES_RAISE_OK;
	}


//
// IUnitSet interface implementation
//

STFResult VirtualStandardBoard::CreateUnitSet(uint32 * reqUnits)
	{
	IPhysicalUnit * tempPhysical;
	IVirtualUnit * tempVirtual;
	STFResult res = STFRES_OK;

	// Walk the list of Factory References and create Virtual Units
	// for each of these references. Enter the Virtual Units into the
	// UnitSet Virtual Unit Collection.

	VirtualUnitCollection::Connect(NULL, NULL);

	while (*reqUnits != VDR_UNITS_DONE && STFRES_SUCCEEDED(res))
		{
		res = physicalUnit->GetUnitByGlobalID(*reqUnits, tempPhysical);
		if (STFRES_SUCCEEDED(res))
			{
			//??? must it better be (tempVirtual, NULL, NULL)?
			res = tempPhysical->CreateVirtual(tempVirtual, this, this);

			if (STFRES_SUCCEEDED(res))
				{
				AddLeafUnit (tempVirtual);
				}
			else
				{
				DP("CreateVirtual err Unit #:0x%08x,  tempVirtual 0x%08x\n", *reqUnits, tempVirtual);
				}
			}
		else
			{
			DP("Did not find global unit ID in list of available public board units! Unit #: 0x%08x\n", *reqUnits);
			}

		reqUnits++;
		}

	if (STFRES_SUCCEEDED(res))
		res = VirtualUnitCollection::Initialize();

	STFRES_RAISE(res);
	}


STFResult VirtualStandardBoard::AllocateChildUnits(void)
	{
	STFRES_RAISE_OK;
	}
