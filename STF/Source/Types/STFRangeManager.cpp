///
/// @brief      Foundation classes for range management.
///

#include "STF/Interface/Types/STFRangeManager.h"
#include "STF/Interface/STFDebug.h"
#include <stdio.h>
#include <stdarg.h>
#include "assert.h"
////////////////////////////////////////////////////////////////////
//
//  STFRangeManager Class
//
////////////////////////////////////////////////////////////////////

STFRangeManager::STFRangeManager (void)
	{
	ranges = NULL;
	maxRanges = 0;
	numRanges = 0;
	}

STFRangeManager::~STFRangeManager (void)
	{
	delete[] ranges;
	}



STFResult STFRangeManager::Add (uint32 start, uint32 size, void * userData)
	{
	Range *newRanges;
	uint32 i, k;
        //lint --e{613,668} 
	STFRES_ASSERT (size > 0, STFRES_INVALID_PARAMETERS);

	// Grow the array if it is too small.
	if (numRanges >= maxRanges)
		{
		i = maxRanges + 10;
		newRanges = new Range[i];
		if (newRanges == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		assert(ranges != NULL);
		if (maxRanges > 0)
			memcpy (newRanges, ranges, maxRanges * sizeof(Range));
		delete[] ranges;
		ranges = newRanges;
		maxRanges = i;
		}

	// Find the position to insert the new range.
	i = 0;
	while (i < numRanges  &&  start >= ranges[i].start)
		i++;

	// If it exists, we must not overlap with the previous range.
	if (i > 0  &&  ranges[i-1].start + ranges[i-1].size > start)
		STFRES_RAISE(STFRES_RANGE_VIOLATION);

	if (i < numRanges)
		{
		// We must not overlap with the next range.
		if (start + size > ranges[i].start)
			STFRES_RAISE(STFRES_RANGE_VIOLATION);

		// Make room for the new range.
		for (k = numRanges;  k > i;  k--)
			ranges[k] = ranges[k-1];
		}

	// Add the range.
	ranges[i].start = start;
	ranges[i].size = size;
	ranges[i].userData = userData;
	numRanges++;

	STFRES_RAISE_OK;
	}



STFResult STFRangeManager::Remove (uint32 start, uint32 size)
	{
	uint32 i = 0;
	//lint --e{613}
	while (i < numRanges  &&  start != ranges[i].start)   // find the range
		i++;

	if (i >= numRanges  ||  ranges[i].size != size)
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);   // range not present or size does not fit

	numRanges--;
	if (i < numRanges)
		memmove (&ranges[i], &ranges[i+1], (numRanges-i) * sizeof(Range));

	STFRES_RAISE_OK;
	}



STFResult STFRangeManager::GetRange (uint32 index, uint32 & start, uint32 & size, void *& userData) const
	{
	//lint --e{613}
	STFRES_ASSERT (index < numRanges, STFRES_INVALID_PARAMETERS);
	start = ranges[index].start;
	size = ranges[index].size;
	userData = ranges[index].userData;
	STFRES_RAISE_OK;
	}
