///
/// @file       VDR/Source/Memory/MemoryPartition.cpp
///
/// @brief      Memory partition implementation.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal implementation file
///
/// @date       2003-01-13
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Memory/MemoryPartition.h"
#include "VDR/Source/Memory/IMemOverlapDetector.h"
#include "STF/Interface/STFDebug.h"

#include <assert.h>
#include <stdlib.h>
// DEBUG_MEMORY_PARTITION symbol can be selected from the osselect.make
#if DEBUG_MEMORY_PARTITION
#define DPR_DMP	DP
#else
#define DPR_DMP	DP_EMPTY
#endif

///////////////////////////////////////////////////////////////////////////////
// Global unit creation function.
///////////////////////////////////////////////////////////////////////////////

// We use a macro to implement the unit creation, in order to avoid mistakes.
UNIT_CREATION_FUNCTION (CreateMemoryPartition, MemoryPartition)

////////////////////////////////////////////////////////////////////
// MemoryPartition partial ITagUnit interface implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryPartition::GetTagIDs (VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_MEMORYPARTITION,
		VDRTID_DONE
		};

	ids = (VDRTID *)supportedTagTypes;
	STFRES_RAISE_OK;
	}

STFResult MemoryPartition::InternalConfigureTags (TAG * tags)
	{
	STFRES_RAISE_OK;
	}


STFResult MemoryPartition::InternalUpdate (void)
	{
	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// MemoryPartition partial IPhysicalUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryPartition::CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	STFResult res;

	unit = new VirtualMemoryPartition (this);
	if (unit == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	if (STFRES_FAILED(res = unit->Connect (parent, root)))
		{
		delete unit;
		unit = NULL;
		}

	STFRES_RAISE(res);
	}


STFResult MemoryPartition::Create(uint64 * createParams)
	{
	// Retrieve the start address and partition size from the Global Board Configuration.
	// The memory attributes are implicitly determined by the start address.
	if (createParams[0] != PARAMS_DWORD  ||  createParams[2] != PARAMS_DWORD  ||  createParams[4] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	physicalStartAddress = (PADDR)createParams[1];
	partitionSize = (uint32)createParams[3];

	// A phyiscal start address 0 and partitionSize 0 denotes cached system memory
	// if (physicalStartAddress != NULL || partitionSize != 0)
	if (physicalStartAddress != 0 || partitionSize != 0) // Fixed Nico
		{
		// Create heap manager.
		heapManager = (STFHeapMemoryManager*)new STFFreeListHeapMemoryManager;
		if (heapManager == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		}
	else
 		overlapDetectionDone = true;

	DPR_DMP("MemoryPartition::Create(): Manager=0x%08x Unit=0x%08x physical=0x%08x size=%07d\n", 
		(PADDR)heapManager, this->GetUnitID(), physicalStartAddress, partitionSize);

#if WIN32
	// Actual physical addresses do not work too well in a virtual memory
	// system, so we fake it a little here.  This memory will never be deallocated
	// as long as the driver lives.
	physicalStartAddress = (PADDR)::VirtualAlloc(NULL, partitionSize, MEM_COMMIT, PAGE_READWRITE);
#elif __linux__
	physicalStartAddress = (PADDR)malloc(partitionSize);
#endif

	STFRES_RAISE_OK;
	}

STFResult MemoryPartition::Connect(uint64 localID, IPhysicalUnit * source)
	{
	IMemoryOverlapDetector *overlapDetector;

	if (heapManager)
		{
		// We are only connected to the Memory Overlap Detector.
		if (localID != 0)
			STFRES_RAISE(STFRES_INVALID_PARAMETERS);

		// Let the overlap detector check that our memory range doesn't collide with others.
		STFRES_REASSERT(source->QueryInterface (VDRIID_MEMORYOVERLAPDETECTOR, (void *&)overlapDetector));
		STFRES_REASSERT(overlapDetector->RegisterRange (physicalStartAddress, partitionSize));
		overlapDetector->Release ();
		overlapDetectionDone = true;
		}
	else
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}

STFResult MemoryPartition::Initialize(uint64 * depUnitsParams)
	{
	if (heapManager)
		{
		// For safety reasons, the overlap detection must have been executed.
		if (! overlapDetectionDone)
			{
			delete heapManager;
			heapManager = NULL;
			STFRES_RAISE(STFRES_OPERATION_ABORTED);
			}

		// Initialize memory management
		STFRES_RAISE(heapManager->Initialize(physicalStartAddress, partitionSize));
		}
	else
		STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// MemoryPartition specific implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryPartition::Allocate (uint32 size, uint32 alignmentFactor, uint8 * &startAddress, PADDR &physicalAddress)
	{
	if (heapManager != NULL)
		STFRES_RAISE(heapManager->Allocate (size, alignmentFactor, startAddress, physicalAddress));
	else
		{
		uint8 *p = (uint8 *)new uint32[(size + 3) / 4];
		startAddress = p;
		physicalAddress = (PADDR)p;

		if (!startAddress)
			STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);
		else
			STFRES_RAISE_OK;
		}
	}


STFResult MemoryPartition::Deallocate (uint8 *startAddress, uint32 size)
	{
	if (heapManager != NULL)
		STFRES_RAISE(heapManager->Deallocate (startAddress,size));
	else
		{
		delete[] (uint32 *)startAddress;

		STFRES_RAISE_OK;
		}
	}


STFResult MemoryPartition::GetPhysicalUnitStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes)
	{
	if (heapManager != NULL)
		STFRES_RAISE(heapManager->GetStatus (freeBytes, freeLargestBlockSize, usedBytes));
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);
	}

////////////////////////////////////////////////////////////////////
// The virtual memory partition unit.
////////////////////////////////////////////////////////////////////

VirtualMemoryPartition::VirtualMemoryPartition (MemoryPartition * physical)
	: VirtualUnit (physical)
	{
	physicalUnit = physical;

	startAddress = NULL;
	physicalAddress = (PADDR) NULL;
	allocatedSize = 0;
	allocatedAlignmentFactor = 1;

	size = 0;
	alignmentFactor = 0;
	}

VirtualMemoryPartition::~VirtualMemoryPartition (void)
	{
	// Nothing to do.
	}



STFResult VirtualMemoryPartition::QueryInterface (VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT (VDRIID_MEMORYPARTITION, IMemoryPartition);
	VDRQI_END(VirtualUnit);

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// VirtualMemoryPartition IMemoryPartition implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPartition::SetAllocationParameters (uint32 size, uint32 alignmentFactor, char * ownerName)
	{
	this->size = size;
	this->alignmentFactor = alignmentFactor;
	this->ownerName = ownerName;

	//DP("VirtualMemoryPartition Allocation Params: size %d alignmentFactor: %d\n");

	STFRES_RAISE_OK;
	}

uint8 *VirtualMemoryPartition::GetStartAddress (void)
	{
	return startAddress;
	}

PADDR VirtualMemoryPartition::GetPhysicalAddress (void)
	{
	return physicalAddress;
	}

STFResult VirtualMemoryPartition::GetPhysicalUnitStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes)
	{
	return physicalUnit->GetPhysicalUnitStatus (freeBytes, freeLargestBlockSize, usedBytes);
	}


////////////////////////////////////////////////////////////////////
// VirtualMemoryPartition partial ITagUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPartition::InternalConfigureTags (TAG * tags)
	{
	STFRES_REASSERT(VirtualUnit::InternalConfigureTags (tags));

/*	PARSE_TAGS_START(tags)
		GETSETC(MEMPART_SIZE,					size, 0);
		GETSETC(MEMPART_ALIGNMENT_FACTOR,	alignmentFactor, 0);
	PARSE_TAGS_END
*/
	STFRES_RAISE_OK;
	}

STFResult VirtualMemoryPartition::InternalUpdate (void)
	{
	//???
	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// Partial IVirtualUnit overload implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPartition::PreemptUnit (uint32 flags)
	{
	STFResult res;

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		// Allocate our memory.
		allocatedSize = size;
		allocatedAlignmentFactor = alignmentFactor;

		assert(allocatedSize != 0 || allocatedAlignmentFactor != 0);
		if (STFRES_IS_ERROR(res = physicalUnit->Allocate (allocatedSize, allocatedAlignmentFactor, startAddress, physicalAddress)))
			{
			DP("VirtualMemoryPartition::PreemptUnit(VDRUALF_PREEMPT_START_NEW) Could not allocate memory: \
				0x%08x bytes with alignment %i requested,  name %s. (Unit id:0x%08x)\n",
				allocatedSize, allocatedAlignmentFactor, ownerName, this->physicalUnit->unitID);
			STFRES_RAISE(res);
			}

		DPR_DMP("Manager=0x%08x Alloc.   Partition base=0x%08x size=%07d name=%s unit=0x%08x\n",  
			(PADDR)physicalUnit->heapManager, physicalAddress, allocatedSize, ownerName, this->physicalUnit->GetUnitID());
		}

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		// Deallocate our memory.
		STFRES_REASSERT(physicalUnit->Deallocate (startAddress, allocatedSize));

		DPR_DMP("Manager=0x%08x DeAlloc. Partition base=0x%08x size=%07d name=%s unit=0x%08x\n",
			(PADDR)physicalUnit->heapManager, physicalAddress, allocatedSize, ownerName, this->physicalUnit->GetUnitID());

		startAddress = NULL;
		physicalAddress = (PADDR) NULL;
		}

	STFRES_RAISE_OK;
	}
