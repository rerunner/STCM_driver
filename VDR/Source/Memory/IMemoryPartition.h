///
/// @file       VDR/Source/Memory/IMemoryPartition.h
///
/// @brief      VDR internal interface for memory partitions.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal header file
///
/// @date       2003-01-13
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#ifndef I_MEMORYPARTITION_H
#define I_MEMORYPARTITION_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Unit/IVDRTags.h"



////////////////////////////////////////////////////////////////////
// Tag type definition.
////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_MEMORYPARTITION = 0x0001a000;   // created by the ID value manager



////////////////////////////////////////////////////////////////////
//! Memory Partition internal interface.
////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_MEMORYPARTITION = 0x80000006;   // created by the ID value manager

class IMemoryPartition : virtual public IVDRBase
	{
	public:
		//! Set the allocation parameters.
		/*!
			\param size IN: The total size in bytes.

			\param alignmentFactor IN: In bytes, e.g. "8" for 64 bit alignment.

			\param ownerName IN: An optional string for owner identification (debugging).

			\return Standard STFResult
		*/
		virtual STFResult SetAllocationParameters (uint32 size, uint32 alignmentFactor, char * ownerName) = 0;

		//! Get the start address (in Kernel mode; valid only if this virtual unit is active).
		virtual uint8 *GetStartAddress (void) = 0;

		//! Get the physical address (valid only if this virtual unit is active).
		virtual PADDR GetPhysicalAddress (void) = 0;

		//! Get the status of the physical memory partition.
		virtual STFResult GetPhysicalUnitStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes) = 0;
	};



#endif
