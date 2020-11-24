///
/// @file       VDR/Source/Memory/MemoryPartition.h
///
/// @brief      Memory partition implementation.
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

#ifndef MEMORYPARTITION_H
#define MEMORYPARTITION_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/STFHeapMemoryManager.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "VDR/Source/Unit/VirtualUnit.h"
#include "VDR/Source/Memory/IMemoryPartition.h"
#include "VDR/Source/Construction/IUnitConstruction.h"



class VirtualMemoryPartition;   // forward declaration



////////////////////////////////////////////////////////////////////
//! Memory partition implementation.
////////////////////////////////////////////////////////////////////

class MemoryPartition : public SharedPhysicalUnit
	{
	friend class VirtualMemoryPartition;


	private:
		bool		overlapDetectionDone;

	protected:
		PADDR							physicalStartAddress;
		uint32						partitionSize;
		STFHeapMemoryManager		*heapManager;

		virtual STFResult Allocate (uint32 size, uint32 alignmentFactor, uint8 *& startAddress, PADDR & physicalAddress);
		virtual STFResult Deallocate (uint8 * startAddress, uint32 size);
		virtual STFResult GetPhysicalUnitStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes);

	public:
		MemoryPartition (VDRUID unitID) : SharedPhysicalUnit (unitID)
			{
			overlapDetectionDone = false;
			heapManager = NULL;
			}
		
		virtual ~MemoryPartition (void)
			{
			delete heapManager;
			}

	public:
		// Partial ITagUnit implementation.

		virtual STFResult GetTagIDs (VDRTID * & ids);

		virtual STFResult InternalConfigureTags (TAG * tags);
		virtual STFResult InternalUpdate (void);

	public:
		// Partial IPhysicalUnit implementation.

		virtual STFResult CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);		// reads two createParams: start address and size
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};



class VirtualMemoryPartition : virtual public IMemoryPartition,
										 public VirtualUnit
	{
	protected:
		MemoryPartition	*physicalUnit;

		// The values of the active allocation
		uint8		*startAddress;		// Kernel mode logical address
		PADDR		physicalAddress;
		uint32	allocatedSize;
		uint32	allocatedAlignmentFactor;

		// Variables configurable by tags.
		uint32	size;
		uint32	alignmentFactor;
		char		*ownerName;

	public:
		VirtualMemoryPartition (MemoryPartition * physical);
		virtual ~VirtualMemoryPartition (void);

		// Partial IVDRBase implementation.
		virtual STFResult QueryInterface (VDRIID iid, void * & ifp);

	public:
		// IMemoryPartition implementation.
		virtual STFResult SetAllocationParameters (uint32 size, uint32 alignmentFactor, char * ownerName);
		virtual uint8 *GetStartAddress (void);
		virtual PADDR GetPhysicalAddress (void);
		virtual STFResult GetPhysicalUnitStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes);

	public:
		// Partial ITagUnit implementation.
		virtual STFResult InternalConfigureTags (TAG * tags);
		virtual STFResult InternalUpdate (void);

	public:
		// Partial IVirtualUnit overload implementation.
		virtual STFResult PreemptUnit (uint32 flags);
	};


#endif
