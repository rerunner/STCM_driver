///
/// @file       VDR/Source/Memory/MemOverlapDetector.cpp
///
/// @brief      VDR memory overlap detector.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal implementation file
///
/// @date       2003-01-17
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Memory/MemOverlapDetector.h"

#include "STF/Interface/STFDebug.h"
#include <string.h>



///////////////////////////////////////////////////////////////////////////////
// Global unit creation function.
///////////////////////////////////////////////////////////////////////////////

// We use a macro to implement the unit creation, in order to avoid mistakes.
UNIT_CREATION_FUNCTION (CreateMemoryOverlapDetector, MemoryOverlapDetector)



////////////////////////////////////////////////////////////////////
//! MemoryOverlapDetector implementation.
////////////////////////////////////////////////////////////////////

MemoryOverlapDetector::MemoryOverlapDetector (VDRUID unitID)
	: SharedPhysicalUnit (unitID)
	{
	ranges = NULL;
	maxRanges = 0;
	numRanges = 0;
	}

MemoryOverlapDetector::~MemoryOverlapDetector (void)
	{
	delete[] ranges;
	}



////////////////////////////////////////////////////////////////////
// MemoryOverlapDetector IMemoryOverlapDetector implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryOverlapDetector::RegisterRange (PADDR startAddress, uint32 size)
	{
	Range *newRanges;
	int arraySize, copySize, i, k;
	PADDR start;
	uint32 end, newEnd;
	bool newIsHigher;
	//lint --e{613}
	STFAutoMutex mutex (&registerMutex);

	// Look if the new range overlaps with the registered ranges.
	//??? This is a sub-optimal linear O(n) algorithm.
	newEnd = startAddress + size;
	i = 0;
	newIsHigher = true;
	while (i < numRanges  &&  newIsHigher)
		{
		start = ranges[i].start;
		end   = start + ranges[i].size;
		if (newEnd <= start)
			{
			// New range is at a lower address.
			if (newEnd == start)
				{
				// Extend the range with the new one.
				ranges[i].start = startAddress;
				ranges[i].size += size;
				STFRES_RAISE_OK;   // no overlap
				}
			// The loop ends now because we know that the previous ranges were at lower addresses.
			newIsHigher = false;
			}
		else if (end <= startAddress)
			{
			// New range is at a higher address somewhere.
			if (end == startAddress)
				{
				// Extend the range with the new one.
				ranges[i].size += size;
				k = i + 1;
				if (k < numRanges  &&  end + size == ranges[k].start)
					{
					// The new range exactly closes the gap between two ranges. Merge everything.
					ranges[i].size += ranges[k].size;
					copySize = (numRanges-k-1) * sizeof(Range);
					if (copySize > 0)
						memmove (&ranges[k], &ranges[k+1], copySize);
					numRanges--;
					}
				STFRES_RAISE_OK;   // no overlap
				}
			// Continue in the loop...
			i++;
			}
		else
			{
			// We now have start < newEnd and startAddress < end. Since startAdress < newEnd and start < end,
			// this means an overlap of some sort.
			DP("MemoryOverlapDetector: start address %08x, size %05x overlaps with previous range\n", startAddress, size);
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
			}
		}

	// Here, "i" is the index of the range that is right above the new range and cannot be merged with it.

	if (numRanges >= maxRanges)
		{
		// The array is too small. Allocate a new one with larger size.
		arraySize = maxRanges + 20;
		newRanges = new Range[arraySize];
		if (newRanges == NULL)
			{
			DP("MemoryOverlapDetector cannot allocate larger range array\n");
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
			}
		copySize = maxRanges * sizeof(Range);
		if (copySize > 0)
			memcpy (&newRanges[0], &ranges[0], copySize);

		delete[] ranges;
		ranges = newRanges;
		maxRanges = arraySize;
		}

	// Move up all ranges above and including index i.
	copySize = (numRanges-i) * sizeof(Range);
	if (copySize > 0)
		memmove (&ranges[i+1], &ranges[i], copySize);

	// Insert the new range at index i.
	ranges[i].start = startAddress;
	ranges[i].size  = size;
	numRanges++;

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
//! MemoryOverlapDetector partial IVDRBase implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryOverlapDetector::QueryInterface (VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT (VDRIID_MEMORYOVERLAPDETECTOR, IMemoryOverlapDetector);
	VDRQI_END(SharedPhysicalUnit);

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// MemoryPartition partial IPhysicalUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryOverlapDetector::CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	// There is no virtual memory overlap detector.
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

STFResult MemoryOverlapDetector::Create(uint64 * createParams)
	{
	PADDR startAddress;
	uint32 i, size;

	// We take parameters in pairs that represent address areas that should be reserved immediately.
	i = 0;
	while (createParams[i] != PARAMS_DONE)
		{
		STFRES_ASSERT (createParams[i] == PARAMS_DWORD  &&  createParams[i+2] == PARAMS_DWORD, STFRES_INVALID_PARAMETERS);
		startAddress = createParams[i+1];
		size = createParams[i+3];
		STFRES_REASSERT(RegisterRange (startAddress, size));
		i += 4;
		}

	STFRES_RAISE_OK;
	}

STFResult MemoryOverlapDetector::Connect(uint64 localID, IPhysicalUnit * source)
	{
	// We are not connected with another unit at unit construction time.
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}

STFResult MemoryOverlapDetector::Initialize(uint64 * depUnitsParams)
	{
	// We are not connected with another unit.
	if (depUnitsParams[0] != MAPPING_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}
