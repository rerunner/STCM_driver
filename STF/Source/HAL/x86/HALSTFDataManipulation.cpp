//
// Platform: Win32
//

#include "STF/Interface/Types/STFBasicTypes.h"


void MUL32x32(uint32 op1, uint32 op2, uint32 & upper, uint32 & lower)
	{
	uint32 u, l;

	__asm {
			mov	eax, op1
			mov	edx, op2
			mul	edx
			mov	u, edx
			mov	l, eax
			}

	upper = u;
	lower = l;
	}


uint32 DIV64x32(uint32 upper, uint32 lower, uint32 op)
	{
	uint32 res;

	if (op)
		{
		__asm {
				mov	edx, upper
				mov	eax, lower
				mov	ecx, op
				div	ecx
				mov	res, eax
				};
		return res;
		}
	else
		return 0;
	}


uint32 ScaleDWord(uint32 op, uint32 from, uint32 to)
	{
	uint32	res;         
	
	if (to && op)
		{
		__asm {                        
				mov	eax, op
				mov	edx, to
				mul	edx
				mov	ecx, from
				cmp	edx, ecx
				jge	done
				div	ecx
				mov	res, eax
			done:
				};

		return res;
		}
	else
		return 0;
	}

int32 ScaleLong(int32 op, int32 from, int32 to)
	{
	int32	res;         
	
	if (to && op)
		{
		__asm {                        
				mov	eax, op
				mov	edx, to
				imul	edx
				mov	ecx, from
				cmp	edx, ecx
				jge	done
				idiv	ecx
				mov	res, eax
			done:
				};                                                                                     

		return res;
		}
	else
		return 0;
	}

