//
// PURPOSE:   Generic Physical Units Implementation
//

#include "PhysicalUnit.h"

#include "VDR/Source/Construction/IUnitConstruction.h"

#include "STF/Interface/STFDebug.h"

// main diagnostic control is via _DEBUG_PREEMPT, setting this to 1 and
// setting a range of UNIT IDs will produce verbose activation logging
#if _DEBUG

#define _DEBUG_PREEMPT		0
#define _DEBUG_UPDATE		0

extern char * DebugUnitNameFromID(uint32 unitID);
#define DEBUGUNITNAMEFROMID(id) DebugUnitNameFromID(id)
#define _DEBUG_LOG_FIRST_UNIT_ID		0x40002002
#define _DEBUG_LOG_LAST_UNIT_ID			0x400030FF

#else  // ! _DEBUG

#define DEBUGUNITNAMEFROMID(id) ""

#endif // _DEBUG

#if _DEBUG_PREEMPT
#define DPPREEMPT(vunit, string) \
	do { \
		if ( (vunit->GetUnitID() >= _DEBUG_LOG_FIRST_UNIT_ID) && (vunit->GetUnitID()	<= _DEBUG_LOG_LAST_UNIT_ID) ) \
			DP("%s(x%0X) "string, DebugUnitNameFromID(vunit->GetUnitID()), (int) vunit); \
		} while (0)

#else

#define DPPREEMPT(vunit, string) do {} while (0)

#endif // _DEBUG_PREEMPT


#if _DEBUG_UPDATE
#define DP_UPD(vunit, gettagidsresult, ids, string) \
	do { \
		if ( (vunit->GetUnitID() >= _DEBUG_LOG_FIRST_UNIT_ID) && (vunit->GetUnitID()	<= _DEBUG_LOG_LAST_UNIT_ID) ) \
			DP("%s GetTagIds=%d ids=%d "string, DebugUnitNameFromID(vunit->GetUnitID()), (int) gettagidsresult, (int) ids); \
		} while (0)
#else // ! _DEBUG_UPDATE
#define DP_UPD(vunit, gettagidsresult, ids, string) do {} while (0)
#endif // _DEBUG_UPDATE

///////////////////////////////////////////////////////////////////////////////
// PhysicalUnit
///////////////////////////////////////////////////////////////////////////////

STFResult PhysicalUnit::QueryInterface(VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_PHYSICAL_UNIT,		IPhysicalUnit);
		VDRQI_IMPLEMENT(VDRIID_VDR_PHYSICAL_UNIT,	IVDRPhysicalUnit);
		VDRQI_IMPLEMENT(VDRIID_VDR_DEVICE_UNIT,	IVDRDeviceUnit);
		VDRQI_IMPLEMENT(VDRIID_VDR_TAG_UNIT,		IVDRTagUnit);
	VDRQI_END(VDRBase);

	STFRES_RAISE_OK;
	}



//
// Commodities
//

uint32 PhysicalUnit::GetNumberOfParameters(uint64 * createParams)
	{
	uint32 actualNumber = 0;

	while (*createParams != PARAMS_DONE)
		{		
		createParams += 2;		
		actualNumber++;
		}

	return actualNumber;
	}


STFResult PhysicalUnit::GetDWordParameter(uint64 * createParams, uint32 index, uint32 & param)
	{
	if (createParams[2 * index] == PARAMS_DWORD)
		param = (uint32) createParams[2 * index + 1];
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::GetStringParameter(uint64 * createParams, uint32 index, char * & param)
	{
	if (createParams[2 * index] == PARAMS_STRING)
		param = (char *) createParams[2 * index + 1];
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::GetPointerParameter(uint64 * createParams, uint32 index, void * & param)
	{
	if (createParams[2 * index] == PARAMS_POINTER)
		{		
		param = (void *) createParams[2 * index + 1];
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

	STFRES_RAISE_OK;
	}


//
// These functions are called from the Virtual Unit during its
// configuration process
//

STFResult PhysicalUnit::BeginConfigure(IVirtualUnit * vUnit)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::EndConfigure(IVirtualUnit * vUnit)
	{
	VDRTID * ids;
	STFResult	res;

	res = vUnit->GetTagIDs(ids);

	DP_UPD(vUnit, res, ids, "EndConfigure@1\n");
	STFRES_REASSERT(res);
		
	// Do internal update only if the virtual unit supports tags!
	if (ids != NULL)
		STFRES_RAISE(vUnit->InternalUpdate());

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::IsUnitCurrent(IVirtualUnit * vUnit)
	{
	STFRES_RAISE_OK;
	}



//
// The following functions are used for the Tag configuration 
// of the Physical Unit itself
//

STFResult PhysicalUnit::BeginConfigure(void)
	{
	STFRES_RAISE(InternalBeginConfigure());
	}


STFResult PhysicalUnit::ConfigureTags(TAG * tags)
	{
	STFResult res;

	STFRES_REASSERT(InternalBeginConfigure());
	res = InternalConfigureTags(tags);
	InternalCompleteConfigure();

	STFRES_RAISE(res);
	}


STFResult PhysicalUnit::Configure(const TAGList & tagList)
	{
	STFRES_RAISE(ConfigureTags((TAG *)(tagList.Tags())));
	}


STFResult PhysicalUnit::CompleteConfigure(void)
	{
	STFRES_RAISE(InternalCompleteConfigure());
	}


STFResult PhysicalUnit::InternalBeginConfigure(void)
	{
	configureCounter++;

	if (configureCounter == 1)
		{
		// clear changes flag set
		changeSet = 0;
		}

	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::InternalAbortConfigure(void)
	{
	configureCounter--;
	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::InternalCompleteConfigure(void)
	{
	configureCounter--;

	if (configureCounter == 0)
		{
		STFRES_REASSERT(InternalUpdate());
		changeSet = 0;
		}

	STFRES_RAISE_OK;
	}

//
// The following three are default implementations only!
//

STFResult PhysicalUnit::GetTagIDs(VDRTID * & ids)
	{
	ids = NULL;			// No tags by default!
	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::InternalConfigureTags(TAG * tags)
	{
	STFRES_RAISE_OK;
	}


STFResult PhysicalUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// ExclusivePhysicalUnit
///////////////////////////////////////////////////////////////////////////////

ExclusivePhysicalUnit::ExclusivePhysicalUnit(VDRUID unitID)
	: PhysicalUnit(unitID)
	{
	preemptionPhase = VDRUALF_PREEMPT_NONE;
	numRegisteredUnits = 0;
	maxRegisteredUnits = 4;
	registeredUnits = new RegisteredVirtualUnit[maxRegisteredUnits];
	lockCount = 0;
	ageCount = 0;
	currentUnit = NULL;
	}


ExclusivePhysicalUnit::~ExclusivePhysicalUnit(void)
	{
	delete[] registeredUnits;
	}


STFResult ExclusivePhysicalUnit::CancelRegistration(IVirtualUnit * vunit, RegisteredVirtualUnit & runit)
	{
	int	i;

	if (numRegisteredUnits)
		{
		i = 0;
		while (i < numRegisteredUnits && registeredUnits[i].unit != vunit)
			i++;

		if (i < numRegisteredUnits)
			{
			//
			// If this was a waiting unit, we better signal a grant
			//
			if (registeredUnits[i].flags & VDRUALF_PREEMPT_REGISTERED)
				vunit->NotifyActivationRetry();

			runit = registeredUnits[i];

			numRegisteredUnits--;
			while (i < numRegisteredUnits)
				{
				registeredUnits[i] = registeredUnits[i+1];
				i++;
				}

			STFRES_RAISE_TRUE;
			}
		}

	STFRES_RAISE_FALSE;
	}


STFResult ExclusivePhysicalUnit::AddRegistration(const RegisteredVirtualUnit & runit)
	{
	int	i;
	RegisteredVirtualUnit	*	tunits;
	//lint --e{613}
	//
	// Increase size of registration array if neccessary
	//
	if (numRegisteredUnits == maxRegisteredUnits)
		{
		maxRegisteredUnits *= 2;
		tunits = new RegisteredVirtualUnit[maxRegisteredUnits];
		for(i=0; i<numRegisteredUnits; i++)
			tunits[i] = registeredUnits[i];
		delete[] registeredUnits;
		registeredUnits = tunits;
		}

	//
	// Insert into list of waiting units
	//
	i = numRegisteredUnits;
	while (i > 0 && runit < registeredUnits[i-1])
		{
		registeredUnits[i] = registeredUnits[i-1];
		i--;
		}
	registeredUnits[i] = runit;
	numRegisteredUnits++;

	//
	// Signal a registered activation
	//
	runit.unit->NotifyActivationRegistered();

	STFRES_RAISE_OK;
	}


STFResult ExclusivePhysicalUnit::SignalTopRegistration(void)
	{
	STFResult err = STFRES_OK;
	//lint --e{613}
	if (numRegisteredUnits && (registeredUnits[0].flags & VDRUALF_PREEMPT_REGISTERED))
		{
		registeredUnits[0].flags &= ~VDRUALF_PREEMPT_REGISTERED;
		err = registeredUnits[0].unit->NotifyActivationRetry();
		}

	STFRES_RAISE(err);
	}


STFResult ExclusivePhysicalUnit::ActivateAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	STFResult					err = STFRES_OK;
	RegisteredVirtualUnit	crunit;
	uint32						age;
	//lint --e{613}
	//
	// Separate recover flags from preemption flags.  
	// If recorvery and preemption are specified, we are only to recover if
	// the preemption fails.
	//
	uint32	rflags = flags & VDRUALF_PREEMPT_RECOVER;
	if (flags & VDRUALF_PREEMPT_DIRECT) flags &= ~VDRUALF_PREEMPT_RECOVER;

	//
	// If this is an already registered unit, remove it from the registration queue
	//
	if (CancelRegistration(vunit, crunit) == STFRES_TRUE)
		age = crunit.age;
	else
		age = ageCount++;

	if (flags & VDRUALF_PREEMPT_CHECK)
		{
		RegisteredVirtualUnit	runit(vunit, flags | VDRUALF_PREEMPT_REGISTERED, age, time, duration);

		//
		// Check if a different virtual unit locks this physical or the next in line
		// has a higher priority than what we have
		//
		if (lockCount > 0 && currentUnit != vunit || numRegisteredUnits > 0 && registeredUnits[0] < runit)
			{
			//
			// If so, we can not preempt
			//
			if (flags & VDRUALF_PREEMPT_REGISTER)
				{
				//
				// Now register for preemption
				//
				AddRegistration(runit);
				//
				// Return with the info, that the activation has been deferred
				//
				STFRES_RAISE(STFRES_OPERATION_PENDING);
				}
			else
				{
				DP("# ExclusivePhysicalUnit in use %s=%08x\n", DEBUGUNITNAMEFROMID(unitID), unitID);
				STFRES_RAISE(STFRES_OBJECT_IN_USE);
				}
			}
		}

	//
	// First phase of preemption, stop previously active unit
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_STOP_PREVIOUS))
		{
		//
		// Check for current phase
		//
		DPPREEMPT( vunit, "ActivateAndLock -  VDRUALF_PREEMPT_STOP_PREVIOUS\n");

		if (preemptionPhase == VDRUALF_PREEMPT_NONE)
			{
			//
			// Stop previous unit, if there is one and it is not the same as the current
			//
			if (currentUnit && currentUnit != vunit)
				err = currentUnit->PreemptUnit(VDRUALF_PREEMPT_STOP_PREVIOUS);

			//
			// Advance phase if we succeeded
			//
			if (!STFRES_IS_ERROR(err)) preemptionPhase = VDRUALF_PREEMPT_STOP_PREVIOUS;
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// Second phase of preemption, change parameters to new unit
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_CHANGE))
		{
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_CHANGE\n");

		if (preemptionPhase == VDRUALF_PREEMPT_STOP_PREVIOUS)
			{
			//
			// Change the parameters if the new unit is not already the current one
			//
			if (currentUnit != vunit)
				{	  
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE)\n");						

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE);				
				}
			else 
				DPPREEMPT(vunit, "Already current. not calling vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE)\n");
			
			if (!STFRES_IS_ERROR(err)) preemptionPhase = VDRUALF_PREEMPT_CHANGE;
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// Third phase of preemption, start the new unit
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_START_NEW))
		{
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_START_NEW\n");				

		if (preemptionPhase == VDRUALF_PREEMPT_CHANGE)
			{
			//
			// Start the new unit, if it is not already the current one
			//
			if (currentUnit != vunit)
				{	 
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW)\n");						

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW);
				}
			else
				DPPREEMPT(vunit, "Already current. Not calling vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW)\n");					

			if (!STFRES_IS_ERROR(err)) preemptionPhase = VDRUALF_PREEMPT_START_NEW;
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// Final phase of preemption, signal success
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_COMPLETE))
		{
		DPPREEMPT(vunit, "ActivateAndLock(%s) VDRUALF_PREEMPT_COMPLETE\n");				

		if (preemptionPhase == VDRUALF_PREEMPT_START_NEW)
			{
			if (currentUnit != vunit)
				{
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE)\n");						

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE);
				}
			else 
				DPPREEMPT(vunit, "Already current. Not calling vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE)\n");

			//
			// If we succeeded, mark this unit as the new current one
			// or just increment the lockCount
			//
			if (!STFRES_IS_ERROR(err)) 
				{
				preemptionPhase = VDRUALF_PREEMPT_NONE;
				currentUnit = vunit;
				lockCount++;
				}
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// If this activation failed, and we are to recover or if this is
	// a pure recovery operation, do it
	//
	if (STFRES_IS_ERROR(err) || (flags & VDRUALF_PREEMPT_RECOVER))
		{
		if (STFRES_IS_ERROR(err))
			DP("# Error %x caused by unit %s=%x, preemptionPhase: %x, flags: %x #\n", err, DEBUGUNITNAMEFROMID(unitID), unitID, preemptionPhase, flags);
		else if (preemptionPhase)
			DP("### VDRUALF_PREEMPT_RECOVER - unit %s=%x, preemptionPhase: %x\n", DEBUGUNITNAMEFROMID(unitID), unitID, preemptionPhase);
		//
		// Now we need to recover from a previous activation failure.  We only
		// recover the phases that succeeded before.
		//
		// First phase of recover is to stop the new unit, this might be
		// necessary if we are in a multi unit activation, and some other unit fails.
		// In a single unit scenario, this would not happen.
		//
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_RECOVER\n");				

		if (preemptionPhase == VDRUALF_PREEMPT_START_NEW && (rflags & VDRUALF_PREEMPT_STOP_NEW))
			{
			if (vunit != currentUnit)				
				{
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_STOP_NEW)\n");						

				vunit->PreemptUnit(VDRUALF_PREEMPT_STOP_NEW);
				}
			else 
				DPPREEMPT(vunit, " Already current - not calling vunit->PreemptUnit(VDRUALF_PREEMPT_STOP_NEW)\n");					

			preemptionPhase = VDRUALF_PREEMPT_CHANGE;
			}

		//
		//	Second phase of recovery, restore the parameters of the previously active
		// unit.
		//
		if (preemptionPhase == VDRUALF_PREEMPT_CHANGE && (rflags & VDRUALF_PREEMPT_RESTORE))
			{
			if (currentUnit && vunit != currentUnit)
				{
				DPPREEMPT(currentUnit," Calling currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTORE)\n");						
				currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTORE);
				}
			else 
				DPPREEMPT(currentUnit, "Already current - not calling currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTORE)\n");					

			preemptionPhase = VDRUALF_PREEMPT_STOP_PREVIOUS;
			}

		//
		// Final phase of recovery, restart the previous unit
		//
		if (preemptionPhase == VDRUALF_PREEMPT_STOP_PREVIOUS && (rflags & VDRUALF_PREEMPT_RESTART_PREVIOUS))
			{
			if (currentUnit && vunit != currentUnit)
				{
				DPPREEMPT(currentUnit, "Calling currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTART_PREVIOUS)\n");						

				currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTART_PREVIOUS);
				}
			else 
				DPPREEMPT(currentUnit, "Already current - not calling currentUnit->PreemptUnit(VDRUALF_PREEMPT_RESTART_PREVIOUS)\n");					

			preemptionPhase = VDRUALF_PREEMPT_NONE;
			}
		}

	STFRES_RAISE(err);
	}


STFResult ExclusivePhysicalUnit::Unlock(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	STFResult	err = STFRES_OK;
	//lint --e{613}
	if (vunit == currentUnit && lockCount > 0)
		{
		lockCount--;
		if (lockCount == 0)
			{
			if (numRegisteredUnits && (registeredUnits[0].flags & VDRUALF_PREEMPT_REGISTERED))
				{
				registeredUnits[0].flags &= ~VDRUALF_PREEMPT_REGISTERED;
				err = registeredUnits[0].unit->NotifyActivationRetry();
				}
			}
		}
	else
		err = STFRES_INVALID_PARAMETERS;

	STFRES_RAISE(err);
	}


STFResult ExclusivePhysicalUnit::Passivate(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	STFResult					err = STFRES_OK;
	RegisteredVirtualUnit	runit;

	err = CancelRegistration(vunit, runit);

	if (vunit == currentUnit)
		{
		currentUnit->PreemptUnit(VDRUALF_PREEMPT_STOP_PREVIOUS);
		currentUnit->PreemptUnit(VDRUALF_PREEMPT_PASSIVATED);
		currentUnit = NULL;

		if (lockCount > 0)
			{
			lockCount = 0;
			
			SignalTopRegistration();
			}
		}

	STFRES_RAISE(err);
	}


STFResult ExclusivePhysicalUnit::UnlockAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	STFResult	err = STFRES_OK;
	//lint --e{613}
	if (vunit == currentUnit)
		{
		if (flags & VDRUALF_PREEMPT_UNLOCK)
			{
			if (lockCount > 0)
				{
				lockCount--;
				if (lockCount == 0 && numRegisteredUnits)
					{
					RegisteredVirtualUnit	runit(vunit, flags | VDRUALF_PREEMPT_REGISTERED, ageCount++, time, duration);

					if (registeredUnits[0] < runit)
						{
						lockCount = 0;
						SignalTopRegistration();
						
						if (flags & VDRUALF_PREEMPT_REGISTER)
							{
							//
							// Now register for preemption
							//
							AddRegistration(runit);
							//
							// Return with the info, that the activation has been deferred
							//
							err = STFRES_OPERATION_PENDING;
							}
						else
							err = STFRES_OBJECT_IN_USE;		
						}
					}
				}
			else
				err = STFRES_INVALID_PARAMETERS;
			}

		if (err == STFRES_OK && (flags & VDRUALF_PREEMPT_LOCK))
			{
			lockCount++;
			}
		}
	else
		err = STFRES_INVALID_PARAMETERS;

	STFRES_RAISE(err);
	}


STFResult ExclusivePhysicalUnit::BeginConfigure(IVirtualUnit * vUnit)
	{
	// Configuration only possible if Virtual Unit is either "current and locked" or "passive"
	if ((vUnit == currentUnit && lockCount > 0) || (vUnit != currentUnit))
		STFRES_RAISE_OK;
	else
		STFRES_RAISE(STFRES_OBJECT_IN_USE);
//		STFRES_RAISE(STFRES_UNIT_NOT_IN_CONFIGURE_STATE);
	}


STFResult ExclusivePhysicalUnit::EndConfigure(IVirtualUnit * vUnit)
	{
	VDRTID * ids;
	//lint --e{613}
	if (vUnit == currentUnit && lockCount > 0)
		{
		STFResult	res;

		res = vUnit->GetTagIDs(ids);
		DP_UPD(vUnit, res, ids, "EndConfigure current@2\n");
		STFRES_REASSERT(res);
		
		// Do internal update only if the virtual unit supports tags!
		if (ids != NULL)
			STFRES_REASSERT(vUnit->InternalUpdate());
		}
	else
		{
		DP_UPD(vUnit, ((vUnit == currentUnit) ? 1 : 0), lockCount, "EndConfigure !current || lockCount==0 @3\n");
		}

	STFRES_RAISE_OK;
	}


STFResult ExclusivePhysicalUnit::IsUnitCurrent(IVirtualUnit * vUnit)
	{
	if (vUnit == currentUnit)
		STFRES_RAISE(STFRES_TRUE);
	else 
		STFRES_RAISE(STFRES_FALSE);
	}



///////////////////////////////////////////////////////////////////////////////
// Shared Physical Unit
///////////////////////////////////////////////////////////////////////////////

SharedPhysicalUnit::SharedPhysicalUnit(VDRUID unitID)
	: PhysicalUnit(unitID)
	{
	numCurrentUnits = 0;
	maxCurrentUnits = 4;
	currentUnits = new CurrentVirtualUnitShare[maxCurrentUnits];
	}


SharedPhysicalUnit::~SharedPhysicalUnit(void)
	{
	delete[] currentUnits;
	}


STFResult SharedPhysicalUnit::ActivateAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	int	i;
	CurrentVirtualUnitShare	*	nunits;
	STFResult						err = STFRES_OK;

	uint32	rflags = flags & VDRUALF_PREEMPT_RECOVER;
	if (flags & VDRUALF_PREEMPT_DIRECT) flags &= ~VDRUALF_PREEMPT_RECOVER;

	i = 0;
	while (i < numCurrentUnits && currentUnits[i].unit != vunit)
		i++;

	if (i == numCurrentUnits)
		{
		//
		//	If this is a new active unit gain some space in the array
		// of active units.
		//
		if (numCurrentUnits == maxCurrentUnits)
			{
			maxCurrentUnits *= 2;
			nunits = new CurrentVirtualUnitShare[maxCurrentUnits];
			for(i=0; i<numCurrentUnits; i++)
				nunits[i] = currentUnits[i];
			delete[] currentUnits;
			currentUnits = nunits;
			}

		//
		// Enter the unit into the array of active units
		//
		currentUnits[numCurrentUnits].unit = vunit;
		currentUnits[numCurrentUnits].lockCount = 0;
		currentUnits[numCurrentUnits].preemptionPhase = VDRUALF_PREEMPT_NONE;
		currentUnits[numCurrentUnits].active = false;
		numCurrentUnits++;
		}

	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_CHANGE))
		{
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_CHANGE\n");				

		if (currentUnits[i].preemptionPhase == VDRUALF_PREEMPT_NONE)
			{
			//
			// Change the parameters if the new unit is not already a current one
			//
			if (!(currentUnits[i].active))
				{
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE)\n");						

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE);
				}
			else 
				DPPREEMPT(vunit, "Already active - not calling vunit->PreemptUnit(VDRUALF_PREEMPT_CHANGE)\n");					
			
			if (!STFRES_IS_ERROR(err)) currentUnits[i].preemptionPhase = VDRUALF_PREEMPT_CHANGE;
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// Third phase of preemption, start the new unit
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_START_NEW))
		{
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_START_NEW\n");				

		if (currentUnits[i].preemptionPhase == VDRUALF_PREEMPT_CHANGE)
			{
			//
			// Start the new unit, if it is not already the current one
			//
			if (!(currentUnits[i].active))
				{
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW)\n");						

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW);
				}
			else 
				DPPREEMPT(vunit, "Already active - not calling vunit->PreemptUnit(VDRUALF_PREEMPT_START_NEW)\n");					

			if (!STFRES_IS_ERROR(err)) currentUnits[i].preemptionPhase = VDRUALF_PREEMPT_START_NEW;
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// Final phase of preemption, signal success
	//
	if (!STFRES_IS_ERROR(err) && (flags & VDRUALF_PREEMPT_COMPLETE))
		{
		DPPREEMPT(vunit, "ActivateAndLock - VDRUALF_PREEMPT_COMPLETE\n");

		if (currentUnits[i].preemptionPhase == VDRUALF_PREEMPT_START_NEW)
			{
			if (!(currentUnits[i].active))
				{
				DPPREEMPT(vunit, "Calling vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE)\n");

				err = vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE);
				}
			else 
				DPPREEMPT(vunit, "Already active - not calling vunit->PreemptUnit(VDRUALF_PREEMPT_COMPLETE)\n");					

			//
			// If we succeeded, mark this unit as a new current one
			// or just increment the lockCount
			//
			if (!STFRES_IS_ERROR(err)) 
				{
				currentUnits[i].preemptionPhase = VDRUALF_PREEMPT_NONE;
				currentUnits[i].active = true;
				currentUnits[i].lockCount++;
				}
			}
		else
			err = STFRES_INVALID_PARAMETERS;
		}

	//
	// If this activation failed, and we are to recover or if this is
	// a pure recovery operation, do it
	//
	if (STFRES_IS_ERROR(err) || (flags & VDRUALF_PREEMPT_RECOVER))
		{
		if (STFRES_IS_ERROR(err))
			DP("# Error %x caused by unit %s=%x, preemptionPhase: %x, flags: %x #\n", err, DEBUGUNITNAMEFROMID(unitID), unitID, currentUnits[i].preemptionPhase, flags);
		else if (currentUnits[i].preemptionPhase)
			DP("### VDRUALF_PREEMPT_RECOVER - unit %s=%x, preemptionPhase: %x\n", DEBUGUNITNAMEFROMID(unitID), unitID, currentUnits[i].preemptionPhase);
		//BREAKPOINT;

		//
		// Now we need to recover from a previous activation failure.  We only
		// recover the phases that succeeded before.
		//
		// First phase of recover is to stop the new unit, this might be
		// necessary if we are in a multi unit activation, and some other unit fails.
		// In a single unit scenario, this would not happen.
		//
		if (currentUnits[i].preemptionPhase == VDRUALF_PREEMPT_START_NEW && (rflags & VDRUALF_PREEMPT_STOP_NEW))
			{
			if (!(currentUnits[i].active))
				vunit->PreemptUnit(VDRUALF_PREEMPT_STOP_NEW);
			currentUnits[i].preemptionPhase = VDRUALF_PREEMPT_CHANGE;
			}

		//
		//	Second phase of recovery, restore the parameters of the previously active
		// unit.
		//
		if (currentUnits[i].preemptionPhase == VDRUALF_PREEMPT_CHANGE && (rflags & VDRUALF_PREEMPT_RESTORE))
			{
			if (!(currentUnits[i].active))
				vunit->PreemptUnit(VDRUALF_PREEMPT_RESTORE);
			currentUnits[i].preemptionPhase = VDRUALF_PREEMPT_NONE;
			}
				 
		//
		// Remove unit from set of active or activating
		//
		if ((rflags & VDRUALF_PREEMPT_FAILED) && !(currentUnits[i].lockCount))
			{
			numCurrentUnits--;
			if (numCurrentUnits)
				currentUnits[i] = currentUnits[numCurrentUnits];
			}
		}

	STFRES_RAISE(err);
	}


STFResult SharedPhysicalUnit::Unlock(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	int	i;

	i = 0;
	while (i < numCurrentUnits && currentUnits[i].unit != vunit)
		i++;

	if (i < numCurrentUnits)
		{
		if (currentUnits[i].lockCount > 0)
			currentUnits[i].lockCount--;
		}

	STFRES_RAISE_OK;
	}


STFResult SharedPhysicalUnit::Passivate(IVirtualUnit * vunit, const STFHiPrec64BitTime & systemTime)
	{
	int	i;

	i = 0;
	while (i < numCurrentUnits && currentUnits[i].unit != vunit)
		i++;

	if (i < numCurrentUnits)
		{
		vunit->PreemptUnit(VDRUALF_PREEMPT_STOP_PREVIOUS);
		vunit->PreemptUnit(VDRUALF_PREEMPT_PASSIVATED);

		numCurrentUnits--;
		if (numCurrentUnits)
			currentUnits[i] = currentUnits[numCurrentUnits];
		}

	STFRES_RAISE_OK;
	}


STFResult SharedPhysicalUnit::UnlockAndLock(IVirtualUnit * vunit, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
	{
	int	i;

	i = 0;
	while (i < numCurrentUnits && currentUnits[i].unit != vunit)
		i++;

	if (i < numCurrentUnits)
		{
		if (flags & VDRUALF_PREEMPT_UNLOCK)
			{
			if (currentUnits[i].lockCount > 0)
				currentUnits[i].lockCount--;
			}

		if (flags & VDRUALF_PREEMPT_LOCK)
			{
			currentUnits[i].lockCount++;
			}
		}

	STFRES_RAISE_OK;
	}


STFResult SharedPhysicalUnit::BeginConfigure(IVirtualUnit * vUnit)
	{
	STFRES_RAISE_OK;
	}


STFResult SharedPhysicalUnit::EndConfigure(IVirtualUnit * vUnit)
	{
	VDRTID * ids;

	if (IsUnitCurrent(vUnit))
		{
		STFResult	res;

		res = vUnit->GetTagIDs(ids);

		DP_UPD(vUnit, res, ids, "EndConfigure current@4\n");
		STFRES_REASSERT(res);

		// Do internal update only if the virtual unit supports tags!
		if (ids != NULL)
			STFRES_REASSERT(vUnit->InternalUpdate());
		}
	else
		{
		DP_UPD(vUnit, 0, 0, "EndConfigure !current@5\n");
		}
	
	STFRES_RAISE_OK;
	}


STFResult SharedPhysicalUnit::IsUnitCurrent(IVirtualUnit * vUnit)		
	{
	int i; 

	for (i=0; i<numCurrentUnits; i++)
		{
		if (currentUnits[i].unit == vUnit)
			STFRES_RAISE(STFRES_TRUE);
		}

#if 0
// This is useful for detecting problems with specific driver units
// and the wrong unit set being used by the application.
// VDRUID VDRUID_AC3_FRAME_STREAM_DECODER         = 0x40003001;
// VDRUID VDRUID_AC3_STREAM_DECODER               = 0x40003002;
// VDRUID VDRUID_MME_AC3_FRAME_DECODER            = 0x40003007;
	if ((vUnit->GetUnitID() == 0x40003007) || 
		 (vUnit->GetUnitID() == 0x40003001) ||
		 (vUnit->GetUnitID() == 0x40003002))
		{
		DP("%s says %s not current, numCurrentUnits=%d\n", DebugUnitNameFromID(this->GetUnitID()), DebugUnitNameFromID(vUnit->GetUnitID()), numCurrentUnits);
		for (i=0; i<numCurrentUnits; i++)
			DP(" %s=0x%08X != %s=0x%08X\n", 
				DebugUnitNameFromID(currentUnits[i].unit->GetUnitID()),
				(int) currentUnits[i].unit,
				DebugUnitNameFromID(vUnit->GetUnitID()),
				(int) vUnit);
		}
#endif

	STFRES_RAISE(STFRES_FALSE);
	}

SharedPhysicalAutoMutex::SharedPhysicalAutoMutex(SharedPhysicalUnit* pU, STFMutex* mutex)
	{
	acquireMutex = (pU->GetNumCurrentUnits() > 1);
	if (acquireMutex)
		{
		this->mutex = mutex;
		mutex->Enter();
		}
	}

SharedPhysicalAutoMutex::~SharedPhysicalAutoMutex(void)
	{
	if (acquireMutex)
		mutex->Leave();
	}

