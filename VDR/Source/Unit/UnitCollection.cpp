///
/// @brief      Virtual and Physical Unit Collections Implementation
///

#include "UnitCollection.h"
#include "Tags.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"

///////////////////////////////////////////////////////////////////////////////
// PhysicalUnitCollection
///////////////////////////////////////////////////////////////////////////////

//! Search the collection for a physical unit with the given ID and return it if found
STFResult PhysicalUnitCollection::GetUnitByGlobalID(uint32 unitID, IPhysicalUnit * & unit)
   {
   int i = 0;
   bool found = false;

   while (i < numUnits && !found)
      {
      found = (unitID == units[i]->GetUnitID());
      i++;
      }

   if (found)
      unit = units[i - 1];
   else
      {
      unit = NULL;
      STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
      }

   STFRES_RAISE_OK;
   }


//! Return list of unit IDs of all units in the collection
STFResult PhysicalUnitCollection::AvailableUnits(uint32 * unitIDs, int & numIDs)
   {
   int i;

   if (numIDs < numUnits)
      {
      numIDs = numUnits;
      STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
      }
	
   for (i = 0; i < numUnits; i++)
      {
      *unitIDs = units[i]->GetUnitID();
      unitIDs++;
      }

   numIDs = numUnits;

   STFRES_RAISE_OK;
   }


//! Add a Physical Unit to the list
STFResult PhysicalUnitCollection::AddUnit(uint32 localID, IPhysicalUnit * unit)
   {
   IPhysicalUnit **newUnits;
   int newSize;
   int i;

   if (numUnits == totalUnits)
      {
      // The array is too small. Allocate a new one with doubled size.
      newSize = totalUnits * 2;
      newUnits = new IPhysicalUnitPtr[newSize];
      if (newUnits == NULL)
         STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

      // Copy the array content.
      for (i = 0; i < numUnits; i++)
         newUnits[i] = units[i];

      // Replace the old array with the new one.
      delete[] units;
      units = newUnits;
      totalUnits = newSize;
      }

   // Add unit at the end.
   units[numUnits] = unit;
   numUnits++;

   STFRES_RAISE_OK;
   }





///////////////////////////////////////////////////////////////////////////////
// PhysicalUnitSequence 
///////////////////////////////////////////////////////////////////////////////

class PhysicalUnitSequence : public IPhysicalUnitSequence
   {
protected:
   IPhysicalUnit	**	units;
   int					numUnits, maxUnits;
public:
   PhysicalUnitSequence(int maxInitialUnits = 16);

   virtual ~PhysicalUnitSequence(void);

   STFResult InsertUnit(IPhysicalUnit * unit);

   STFResult LockActivationMutexes(bool exclusive);

   STFResult UnlockActivationMutexes(void);
   };


typedef IPhysicalUnit * IPhysicalUnitPtr;


PhysicalUnitSequence::PhysicalUnitSequence(int maxInitialUnits)
   {
   numUnits = 0;
   maxUnits = maxInitialUnits;			 
   units = new IPhysicalUnitPtr[maxUnits];
   //??? What if units == NULL?
   }


PhysicalUnitSequence::~PhysicalUnitSequence(void)
   {
   delete[] units;
   }


STFResult PhysicalUnitSequence::InsertUnit(IPhysicalUnit * unit)
   {
   IPhysicalUnit **newUnits;
   int newSize;
   int	i, j;

   // Look if the unit is already present.
   i = 0;
   while (i < numUnits && unit < units[i])
      i++;

   if (i == numUnits || unit != units[i])
      {
      // The unit is not present yet.
      if (numUnits == maxUnits)
         {
         // The array is too small. Allocate a new one with doubled size.
         newSize = maxUnits * 2;
         newUnits = new IPhysicalUnitPtr[newSize];
         if (newUnits == NULL)
            STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

         // Copy the array content, inserting the unit at index i.
         for(j = 0; j < i; j++)
            newUnits[j] = units[j];

         newUnits[j] = unit;

         for(; j < numUnits; j++)
            newUnits[j+1] = units[j];

         numUnits++;

         // Replace the old array with the new one.
         delete[] units;
         units = newUnits;
         maxUnits = newSize;
         }
      else
         {
         // Insert the unit at index i.
         for(j = numUnits; j > i; j--)
            units[j] = units[j-1];

         units[j] = unit;
         numUnits++;
         }
      }

   STFRES_RAISE_OK;
   }


STFResult PhysicalUnitSequence::LockActivationMutexes(bool exclusive)
   {
   int	i;
   STFResult	res = STFRES_OK;

   // Lock in increasing order.
   i = 0;
   while (i < numUnits && STFRES_SUCCEEDED(res = units[i]->LockActivationMutex(exclusive)))
      i++;

   if (STFRES_FAILED(res))
      {
      // Undo the locking in decreasing order.
      while (--i >= 0)
         units[i]->UnlockActivationMutex();
      }

   STFRES_RAISE(res);
   }


STFResult PhysicalUnitSequence::UnlockActivationMutexes(void)
   {
   int	i;

   // Unlock in decreasing order.
   for(i=numUnits-1; i>=0; i--)
      units[i]->UnlockActivationMutex();

   STFRES_RAISE_OK;
   }



///////////////////////////////////////////////////////////////////////////////
// VirtualUnitCollection
///////////////////////////////////////////////////////////////////////////////


STFResult VirtualUnitCollection::Connect(IVirtualUnit * parent, IVirtualUnit * root)
   {
   STFResult res;
	
   res = VirtualUnit::Connect(parent, root);

   if (STFRES_SUCCEEDED(res))
      {
      res = AllocateChildUnits();

#if _DEBUG
      if (STFRES_FAILED(res))
         DP("### SEVERE PROBLEM in VirtualUnitCollection::Connect() \n - AllocateChildUnits() failed with result %08x - collection Unit ID: %08x\n", res, GetUnitID());
#endif
      }
   else
      {
#if _DEBUG
      DP("### SEVERE PROBLEM in VirtualUnitCollection::Connect() \n - Connect() failed with result %08x - collection Unit ID: %08x\n", res, GetUnitID());
#endif
      }

   STFRES_RAISE(res);
   }


STFResult VirtualUnitCollection::Initialize(void)
   {
   int	i;
   STFResult res;

   for(i = 0; i < numLeafUnits; i++)
      {
      res = leafUnits[i]->Initialize();

#if _DEBUG
      if (STFRES_FAILED(res))
         DP("### SEVERE PROBLEM in VirtualUnitCollection::Initialize() \n - Initialize() in collection %08x failed with result %08x for subunit ID: %08x\n", GetUnitID(), res, leafUnits[i]->GetUnitID());
#endif
      if (STFRES_FAILED(res))
         STFRES_RAISE(res);
      }

   STFRES_RAISE_OK;
   }

void VirtualUnitCollection::ReleaseDestruct(void)
   {
   for (int i = 0;  i < numLeafUnits;  i++)
      {
      leafUnits[i]->Release();   // inversion of CreateVirtual()
      leafUnits[i] = NULL;
      }

   VirtualUnit::ReleaseDestruct();
   }

//
// Lock all activation mutexes that might be used, in a fixed order
//
STFResult VirtualUnitCollection::LockActivationMutexes(bool exclusive)
   {
   //
   // Build the ordered physical sequence if we have not done so
   // already.
   //
   if (!physicalSequence)
      {
      physicalSequence = new PhysicalUnitSequence();

      //
      // Add all physical units to it
      //
      STFRES_REASSERT(BuildPhysicalUnitSequence(physicalSequence));
      }

   //
   // Lock the physical units according to sequence order
   //
   return physicalSequence->LockActivationMutexes(exclusive);
   }

//
// Unlock all activation mutexes that might be used, in a fixed order
//
STFResult VirtualUnitCollection::UnlockActivationMutexes(void)
   {
   //lint --e{613}
   assert(physicalSequence != NULL);
   return physicalSequence->UnlockActivationMutexes();
   }


STFResult VirtualUnitCollection::InternalActivateAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
   {
   STFResult	err = STFRES_OK, err2 = STFRES_OK;
   int	i;

   if (flags & VDRUALF_PREEMPT_RECOVER)
      {
      // In a recover situation, we first need to process ourself, then process our leafs.

      // Process ourself.
      if (STFRES_SUCCEEDED(err = physical->ActivateAndLock(this, flags, time, duration, systemTime)))
         {
         // Now process our leafs.
         for (i = 0;  i < numLeafUnits  &&  STFRES_SUCCEEDED(err2 = leafUnits[i]->InternalActivateAndLock (flags, time, duration, systemTime));  i++)
            {
            // Remember a potential warning, if none has been received so far.
            if (err == STFRES_OK)
               err = err2;
            }

         // An error overrides any warnings.
         if (STFRES_IS_ERROR(err2))
            err = err2;
         }
      }
   else
      {
      // In a normal situation (i.e. non-recover), we first need to process our leafs, then process ourself.

      // Process our leafs.
      err = STFRES_OK;
      for (i = 0;  i < numLeafUnits  &&  STFRES_SUCCEEDED(err2 = leafUnits[i]->InternalActivateAndLock (flags, time, duration, systemTime));  i++)
         {
         if (err == STFRES_OK)
            err = err2;   // remember potential warning
         }

      // An error overrides any warnings.
      if (STFRES_IS_ERROR(err2))
         err = err2;
      else
         {
         // Now process ourself.
         if (STFRES_FAILED(err2 = physical->ActivateAndLock(this, flags, time, duration, systemTime)))
            err = err2;
         else if (err == STFRES_OK)
            err = err2;   // remember potential warning
         }
      }

   return err;
   }


STFResult VirtualUnitCollection::InternalUnlock(const STFHiPrec64BitTime & systemTime)
   {
   STFResult	err, terr;
   int	i;

   err = physical->Unlock(this, systemTime);
   for(i=0; i<numLeafUnits; i++)
      {
      terr = leafUnits[i]->InternalUnlock(systemTime);
      if (!STFRES_IS_ERROR(err) && STFRES_IS_ERROR(terr)) err = terr;
      }

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::InternalPassivate(const STFHiPrec64BitTime & systemTime)
   {
   STFResult	err, terr;
   int	i;

   // First passivate ourself, then the leaf units.
   err = physical->Passivate(this, systemTime);
   for(i=0; i < numLeafUnits; i++)
      {
      terr = leafUnits[i]->InternalPassivate(systemTime);
      if (!STFRES_IS_ERROR(err) && STFRES_IS_ERROR(terr)) err = terr;
      }

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::InternalUnlockAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime)
   {
   STFResult	err, terr;
   int	i;

   terr = STFRES_OK;

   //
   // First try to activate our own physical unit
   //
   if (!STFRES_IS_ERROR(err = physical->UnlockAndLock(this, flags, time, duration, systemTime)))
      {
      //
      // If this did not fail, try all the others
      //
      i = 0;
      while (i < numLeafUnits && !STFRES_IS_ERROR(terr = leafUnits[i]->InternalUnlockAndLock(flags, time, duration, systemTime)))
         {
         //
         // Remember a potential warning, if none has been received so far
         //
         if (err == STFRES_OK) err = terr;
         i++;
         }

      //
      // Remember a potential error, overrides any warnings
      //
      if (STFRES_IS_ERROR(terr)) err = terr;
      }
	
   return err;
   }


STFResult VirtualUnitCollection::BuildPhysicalUnitSequence(IPhysicalUnitSequence * sequence)
   {
   int	i;

   STFRES_REASSERT(sequence->InsertUnit(physical));

   for(i=0; i<numLeafUnits; i++)
      {
      STFRES_REASSERT(leafUnits[i]->BuildPhysicalUnitSequence(sequence));
      }

   STFRES_RAISE_OK;
   }


STFResult VirtualUnitCollection::CompleteActivateAndLock(STFResult err, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & sT)
   {
   STFHiPrec64BitTime systemTime(sT);
		
   //
   // While we are registered
   //
   while (err == STFRES_OPERATION_PENDING)
      {
      //
      // Unlock the mutexes, wait for _all_ grant signals and take the mutexes again
      //
      if (!STFRES_IS_ERROR(err = WaitUnlockedForActivation()))
         {
         //
         // Retry the check until we fail or succeed
         //
         SystemTimer->GetTime(systemTime);
         err = InternalActivateAndLock(flags | VDRUALF_PREEMPT_CHECK, time, duration, systemTime);
         }
      }

   //
   // If the check succeeded
   //
   if (!STFRES_IS_ERROR(err))
      {
      //
      // Perform the four preemption phases in order for all affected virtual units in waves.
      // If any phase fails, perform the recoveries for the potentialy partially successfull
      // phases.
      //
      if (STFRES_IS_ERROR(err = InternalActivateAndLock(flags | VDRUALF_PREEMPT_STOP_PREVIOUS, time, duration, systemTime)))
         {
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_RESTART_PREVIOUS, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_FAILED, time, duration, systemTime);
         }
      else if (STFRES_IS_ERROR(err = InternalActivateAndLock(flags | VDRUALF_PREEMPT_CHANGE, time, duration, systemTime)))
         {
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_RESTORE, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_RESTART_PREVIOUS, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_FAILED, time, duration, systemTime);
         }
      else if (STFRES_IS_ERROR(err = InternalActivateAndLock(flags | VDRUALF_PREEMPT_START_NEW, time, duration, systemTime)))
         {
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_STOP_NEW, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_RESTORE, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_RESTART_PREVIOUS, time, duration, systemTime);
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_FAILED, time, duration, systemTime);
         }
      else
         InternalActivateAndLock(flags | VDRUALF_PREEMPT_COMPLETE, time, duration, systemTime); // This one may not fail !!!
      }
   else
      {
      //
      // In case of failure, signal a failed anyway, to remove any potential registration
      //
      InternalActivateAndLock(flags | VDRUALF_PREEMPT_FAILED, time, duration, systemTime);
      }

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::ActivateAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration)
   {
   STFResult				err = STFRES_OK;
   STFHiPrec64BitTime	systemTime;

   //
   // Lock the activation mutexes of all affected physical units in a
   // fixed order
   //
   if (!STFRES_IS_ERROR(err = LockActivationMutexes()))
      {
      //
      // If we are to wait, set the registration flag
      //
      if (flags & VDRUALF_WAIT)
         flags |= VDRUALF_PREEMPT_REGISTER;

      //
      // First try of activation check
      //
      SystemTimer->GetTime(systemTime);
      err = InternalActivateAndLock(flags | VDRUALF_PREEMPT_CHECK, time, duration, systemTime);

      err = CompleteActivateAndLock(err, flags, time, duration, systemTime);

      //
      // Unlock the activation mutexes
      //
      UnlockActivationMutexes();
      }

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::UnlockAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration)
   {
   STFResult				err = STFRES_OK;
   STFHiPrec64BitTime	systemTime;

   //
   // Lock the activation mutexes of all affected physical units in a
   // fixed order
   //
   if (!STFRES_IS_ERROR(err = LockActivationMutexes()))
      {
      //
      // If we are to wait, set the registration flag
      //
      if (flags & VDRUALF_WAIT)
         flags |= VDRUALF_PREEMPT_REGISTER;

      //
      // First unlock all the units, but register for activation in case
      //
      SystemTimer->GetTime(systemTime);
      err = InternalUnlockAndLock(flags | VDRUALF_PREEMPT_UNLOCK, time, duration, systemTime);
		
      if (err == STFRES_OK)
         {
         //
         // We did not loose the current status during unlock, so we can just
         // lock the units again immediately
         //
         err = InternalUnlockAndLock(flags | VDRUALF_PREEMPT_LOCK , time, duration, systemTime);
         }
      else
         {
         //
         // We lost current status, so we are not locked anymore.  We will have to
         // run through the activate and lock scheme to complete the operation.
         //
         err = CompleteActivateAndLock(err, flags, time, duration, systemTime);
         }

      //
      // Unlock the activation mutexes
      //
      UnlockActivationMutexes();
      }

   STFRES_RAISE(err);
   }


//
// ITagUnit implementation
//

/*! The array of Tag Unit IDs is only built the first time GetTagIDs is called.
  It is then cached.
*/

STFResult VirtualUnitCollection::GetTagIDs(VDRTID * & ids)
   {
   STFResult  err = STFRES_OK, terr = STFRES_OK;
   int        i;
   VDRTID    **	tempIDs;

   // Build a cached combined list, if none exists
   if (tagUnitIDs == NULL)
      {
      tempIDs = new VDRTID*[numLeafUnits + 1];

      // Add child lists
      for (i=0; i<numLeafUnits; i++)
         {
         terr = leafUnits[i]->GetTagIDs(tempIDs[i]);
         STFRES_HIGHER_SEVERITY(err, terr);
         }

      // Add own list
      terr = physical->GetTagIDs(tempIDs[numLeafUnits]);
      STFRES_HIGHER_SEVERITY(err, terr);

      if (STFRES_SUCCEEDED(err))
         err = MergeTagTypeIDLists(tempIDs, numLeafUnits + 1, tagUnitIDs);

      delete[] tempIDs;
      }

   ids = tagUnitIDs;

   STFRES_RAISE(err);
   }	


STFResult VirtualUnitCollection::InternalBeginConfigure(void)
   {
   int i, failPos;
   STFResult err = STFRES_OK, terr = STFRES_OK;

   STFRES_REASSERT(VirtualUnit::InternalBeginConfigure());

   //
   // Start configuration of all other subunits
   //
   i = 0;
   while (i < numLeafUnits && !STFRES_IS_ERROR(terr = leafUnits[i]->BeginConfigure()))
      {
      // Remember a potential warning, if none has been received so far
      if (err == STFRES_OK) err = terr;
      i++;
      }

   // Remember a potential error, overrides any warnings
   if (STFRES_IS_ERROR(terr))
      {
      failPos = i;
      // Undo the BeginConfigure for the subunits we could process so far
      while (i > 0)
         {
         // Undo beginning of configuration
         leafUnits[failPos - i]->InternalAbortConfigure();
         i--;
         }

      err = terr;
      }

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::InternalCompleteConfigure(void)
   {
   STFResult	err = STFRES_OK, terr = STFRES_OK;
   int	i;

   //
   // Complete configuration for all other subunits
   //
   i = 0;

   while (i < numLeafUnits)
      {
      terr = leafUnits[i]->CompleteConfigure();
      // Remember a potential warning, if none has been received so far
      if (err == STFRES_OK) err = terr;
      i++;
      }

   // Remember a potential error, overrides any warnings
   STFRES_HIGHER_SEVERITY(err, terr);

   // Complete Configuration for this Virtual Unit
   terr = VirtualUnit::InternalCompleteConfigure();

   STFRES_HIGHER_SEVERITY(err, terr);

   STFRES_RAISE(err);
   }


STFResult VirtualUnitCollection::InternalConfigureTags(TAG * tags)
   {
   int i;
   STFResult err = STFRES_OK, terr = STFRES_OK;
   uint32 * tuIDs;
   TagSplitter	splitter(tags);
   TAG	*	splitList;

   // Parent processes Tags first. We only process those Tags that our physical unit wants.
   err = physical->GetTagIDs(tuIDs);
   if (tuIDs && !STFRES_IS_ERROR(err))
      {
      // Build a tag list that only contains Tags for this collection
      splitList = splitter.Split(tuIDs);
	
      if (splitList)
         err = VirtualUnit::InternalConfigureTags(splitList);
      }

   if (!STFRES_IS_ERROR(err))
      {
      terr = STFRES_OK;

      // Now configure all leaf units

      i = 0;
      while (i < numLeafUnits && !STFRES_IS_ERROR(terr))
         {
         // Get the Tag Type IDs the leaf unit is accepting
         terr = leafUnits[i]->GetTagIDs(tuIDs);
         if (tuIDs && !STFRES_IS_ERROR(terr))
            {
            // Build tag list containing only those Tags whose type the leaf unit is
            // asking for
            splitList = splitter.Split(tuIDs);

            if (splitList)
               terr = leafUnits[i]->ConfigureTags(splitList);
            }

         // Remember a potential warning, if none has been received so far
         if (err == STFRES_OK) err = terr;
         i++;
         }
      }

   STFRES_RAISE(err);
   }
