///
/// @file       VDR/Source/Memory/ROMLayout/IROMManagerNoUnit.h
///
/// @brief      A ROM manager that is not implemented as a unit but as a singleton object.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal header file
///
/// @date       2004-06-28
///
/// &copy; 2004 ST Microelectronics. All Rights Reserved.
///

#ifndef IROMMANAGERNOUNIT_H
#define IROMMANAGERNOUNIT_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "VDR/Interface/Unit/IVDRGlobalUnitIDs.h"



////////////////////////////////////////////////////////////////////
/// WARNING: This class may be used only for special purposes before
/// the unit construction takes place. For all other purposes, the
/// unit implementation of the ROM manager MUST BE USED!
/// Otherwise, flash ROM read access is not synchronized with write
/// accesses, which leads to corrupted data.
////////////////////////////////////////////////////////////////////

class IROMManagerNoUnit
   {
   public:
   virtual ~IROMManagerNoUnit(){}; // NHV: Added for g++ 4.1.1
   /// @brief Find a ROM partition with a specific unit ID.
   ///
   /// @param unit [in]     The unit ID to search for.
   /// @param address [out] The start address of the ROM partition.
   /// @param size [out]    The byte size of the ROM partition.
   ///
   /// @return              Result value indicating success or failure.
   /// @retval STFRES_OK
   /// @retval STFRES_OBJECT_NOT_FOUND
   ///
   virtual STFResult FindROMPartition (VDRUID unit, void *& address, uint32 & size) = 0;
   };



/// There is only a single instance of this ROM manager system-wide.

extern IROMManagerNoUnit	*	ROMManagerNoUnit;



#endif
