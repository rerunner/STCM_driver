///
/// @brief 		 STFTimer implementation for WIN32
///

#include "OSSTFTimer.h"
#include "STF/Interface/Types/STFInt64.h"

uint32 ClockMultiplier_SystemTo108;
uint32 ClockMultiplier_108ToSystem;                 



//! Helper method to calculate two floats for the 108MHZ clock emulation.
/*! This function calculates the multipliers XXX_108ToSystem which represents 
	 an 14.18 decimal and XXX_SystemTo108 wich is an 18.14 decimal. 
	 If the clock is slower than 0.5 kHz the function will fail.
*/

void Calculate108MhzMultiplier(STFInt64	SystemTicksPerSecond)
	{
	ASSERT(SystemTicksPerSecond > 500);
	STFInt64 emulated_108Mhz_clock = 108000000;
	STFInt64 temp,ticks;
	ticks = SystemTicksPerSecond;
	temp = 0;
	emulated_108Mhz_clock <<= 14;

	temp = emulated_108Mhz_clock / SystemTicksPerSecond;
	ASSERT(temp.Upper() == 0);

	ClockMultiplier_SystemTo108 = temp.Lower();
	
	ticks <<= 18;
	temp = ticks / 108000000;
	ClockMultiplier_108ToSystem = temp.Lower();
	ASSERT(temp.Upper() == 0);
	}

STFResult InitializeOSSTFTimer(void)
	{
	LARGE_INTEGER	frequency;
	static bool		initialized = false;

	if (!initialized)
		{
		initialized = true;

		::QueryPerformanceFrequency(&frequency);


		//We fake the real frequency and pretend we have a 108MHZ clock
		//LongTimeTicksPerSecond = 108000000;	//108Mhz
		
		Calculate108MhzMultiplier(STFInt64((uint32)frequency.LowPart, (int32)frequency.HighPart));

		}

	STFRES_RAISE_OK;
	}
