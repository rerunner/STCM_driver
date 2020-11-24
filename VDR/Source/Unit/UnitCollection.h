//
// PURPOSE:		Virtual and Physical Unit Collections
//

#ifndef UNITCOLLECTIONS_H
#define UNITCOLLECTIONS_H

#include "VirtualUnit.h"
#include "PhysicalUnit.h"

///////////////////////////////////////////////////////////////////////////////
// Physical Unit Collection
///////////////////////////////////////////////////////////////////////////////

typedef IPhysicalUnit * IPhysicalUnitPtr;

//! Collection of PhysicalUnits
/*! Physical Unit Collections are needed by classes like the Board to store
  a reference to all physical units it has Factory References to. 
  It simply implements a dynamic array with some simple management
  functions.
*/
class PhysicalUnitCollection
   {
   protected:
   //! An array containing all subunits (Factory References)
   IPhysicalUnit ** units;

   //! The number of subunits
   int	numUnits;

   //! The size of the subunits array
   int	totalUnits;

   public:
   PhysicalUnitCollection()
         {
         numUnits = 0;
         this->totalUnits = 6;
         units = new IPhysicalUnitPtr[totalUnits];
         }
   virtual ~PhysicalUnitCollection(){}; // NHV: Added for g++ 4.1.1

   //! Get Number of units in collection
   int GetNum(void) { return numUnits; }

   //! Search the collection for a physical unit with the given ID and return it if found
   STFResult GetUnitByGlobalID(VDRUID unitID, IPhysicalUnit * & unit);

   //! Return list of unit IDs of all units in the collection
   STFResult AvailableUnits(VDRUID * unitIDs, int & numIDs);

   //! Add a Physical Unit to the collection
   virtual STFResult AddUnit(uint32 localID, IPhysicalUnit * unit);
   };

///////////////////////////////////////////////////////////////////////////////
// Virtual Unit Collection
///////////////////////////////////////////////////////////////////////////////

typedef IVirtualUnit * IVirtualUnitPtr;

//! Collection of VirtualUnits
/* This class manages collections of Virtual Sub-Units. A specific Virtual
   Unit inherits from this class which implements most of the functions
   of IVirtualUnit, which are applied to the Sub-Units. 
*/
class VirtualUnitCollection : public VirtualUnit
   {
   protected:
   //! An array containing all subunits
   IVirtualUnit				**	leafUnits;

   //! The number of subunits
   int								numLeafUnits, maxLeafUnits;
								
   //! Ordered list of physical units for locking purposes
   IPhysicalUnitSequence	*	physicalSequence;

   //! Array of Tag Unit IDs for this unit and all its leafs
   VDRUID						*	tagUnitIDs;

   //! Add a leaf unit.
   /*! Note that the CreateVirtual() that created the unit already set the ref counter
     to one, so this function DOES NOT do an additional unit->AddRef(). */
   STFResult AddLeafUnit(IVirtualUnit * unit)
         {
         if (numLeafUnits == maxLeafUnits)
            STFRES_RAISE(STFRES_OBJECT_FULL);

         leafUnits[numLeafUnits++] = unit;
         STFRES_RAISE_OK;
         }

   //! Allocate Virtual Child Units
   /*! This function is overridden by a child class to instantiate the specific
     Virtual Sub-Units that are entered into the collection.		*/
   virtual STFResult AllocateChildUnits(void) = 0;

   void ReleaseDestruct(void);

   public:
   VirtualUnitCollection(IPhysicalUnit * physicalUnit, int maxLeafUnits)
      : VirtualUnit(physicalUnit)
         {
         this->numLeafUnits = 0;
         this->maxLeafUnits = maxLeafUnits;

         this->tagUnitIDs = NULL;

         physicalSequence = NULL;

         leafUnits = new IVirtualUnitPtr[maxLeafUnits];
         }

   virtual ~VirtualUnitCollection(void)
         {
         delete[] leafUnits;
         delete[] tagUnitIDs;
         delete physicalSequence;
         }

   //
   // IVirtualUnit Interface implementation
   //

   //! Connect unit collection to its parent and root units
   /*!
     This function continues calling Connect() on the subunits of
     this virtual unit collection.	The subunits receive this unit
     collection as parent. \param root is passed further on as is.
   */
   virtual STFResult Connect(IVirtualUnit * parent, IVirtualUnit * root);

   //! Initialize unit collection
   /*!
     This function will call Initialize() on all the subunits.of
     this virtual unit collection.
   */
   virtual STFResult Initialize(void);

   virtual STFResult LockActivationMutexes(bool exclusive = true);
   virtual STFResult UnlockActivationMutexes(void);

   virtual STFResult InternalActivateAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime);
   virtual STFResult InternalUnlock(const STFHiPrec64BitTime & systemTime);
   virtual STFResult InternalPassivate(const STFHiPrec64BitTime & systemTime);
   virtual STFResult InternalUnlockAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime);
   virtual STFResult CompleteActivateAndLock(STFResult err, uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration, const STFHiPrec64BitTime & systemTime);

   virtual STFResult BuildPhysicalUnitSequence(IPhysicalUnitSequence * sequence);
   virtual STFResult ActivateAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration);
   virtual STFResult UnlockAndLock(uint32 flags, const STFHiPrec64BitTime & time, const STFHiPrec32BitDuration & duration);

   //
   // ITagUnit implementation
   //
   virtual STFResult GetTagIDs(VDRTID * & ids);
   virtual STFResult InternalBeginConfigure(void);
   virtual STFResult InternalCompleteConfigure(void);
   virtual STFResult InternalConfigureTags(TAG * tags);
   };


#endif	// #ifndef UNITCOLLECTIONS_H
