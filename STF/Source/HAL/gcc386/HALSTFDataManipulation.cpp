//
// Platform: 386 Processor, using the GCC compilers
//

// These are just simple C routines 

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/STFDataManipulationMacros.h"

void MUL32x32 (uint32 op1, uint32 op2, uint32 & upper, uint32 & lower)
	{
//	unsigned long long result = (unsigned long long)op1 * op2;
	uint64 result = (uint64)op1 * op2;

	upper = HIDWORD(result);
	lower = LODWORD(result);
	}

uint32 DIV64x32 (uint32 upper, uint32 lower, uint32 op)
	{
//	unsigned long long result = (unsigned long long)upper;
	uint64 result = (uint64)(upper);
	result <<= 32;
	result |= lower;

	result /= op;

	return LODWORD(result);
	}

uint32 ScaleDWord (uint32 op, uint32 from, uint32 to)
	{
//	unsigned long long result = (unsigned long long)op;
	uint64 result = (uint64)op;

	result = (result * to) / from;

	return LODWORD(result);
	}

int32 ScaleLong (int32 op, int32 from, int32 to)
	{
	uint32 result;
	int signCount = 0;
	if (op < 0)
		{
		op = -op;
		signCount++;
		}
	if (from < 0)
		{
		from = -from;
		signCount++;
		}
	if (to < 0)
		{
		to = -to;
		signCount++;
		}

	result = ScaleDWord((uint32)op, (uint32)from, (uint32)to);
	return (signCount & 1) ? -result : result;
	}

