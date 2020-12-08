///
/// @brief      STFTimer implementation for Linux PC User Mode
///

#include "OSSTFTimer.h"
#include "STF/Interface/STFTimer.h"

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

uint32 ClockMultiplier_SystemTo108;
uint32 ClockMultiplier_108ToSystem;                 

void OSSTFTimer::GetTime(STFHiPrec64BitTime & time)
	{
	struct timeval		timeOfDay;
	struct timezone	timeZone;

	if (gettimeofday(&timeOfDay, &timeZone) != 0)
		{
		DP("OSSTFTimer::GetTime - gettimeofday failed, errno: %d\n", errno);
		}

	time = STFHiPrec64BitTime(STFInt64((int32)timeOfDay.tv_sec) * STFInt64(1000000) + STFInt64((int32)timeOfDay.tv_usec), STFTU_HIGHSYSTEM);
	}


STFResult OSSTFTimer::WaitDuration(const STFLoPrec32BitDuration & duration)
	{
	int res;

	struct timespec nanodelayreq, nanoremaining;
	if (duration < STFLoPrec32BitDuration(0,STFTU_HIGHSYSTEM)) 
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	int32	millis = duration.Get32BitDuration();
	
	nanodelayreq.tv_sec = millis / 1000;
	nanodelayreq.tv_nsec = (millis - (millis / 1000)) * 1000 * 1000;

	do
		{
		res = nanosleep(&nanodelayreq, &nanoremaining);

		if (res == -1 && errno == EINTR)
			{
			nanodelayreq = nanoremaining;
			}
		else
			break;
						
		} while (nanoremaining.tv_sec != 0 || nanoremaining.tv_nsec != 0);

	if (res != 0)
		{
		DP("OSSTFTimer::WaitDuration nanosleep failed with %d\n", errno);
		STFRES_RAISE(STFRES_OPERATION_FAILED);
		}
  
	STFRES_RAISE_OK;
	}


STFResult OSSTFTimer::WaitIdleTime(uint32 micros)
	{
	DP("OSSTFTimer::WaitIdleTime not implemented yet!\n");
	STFRES_RAISE(STFRES_UNIMPLEMENTED);
	}



//! Helper method to calculate two floats for the 108MHZ clock emulation.
/*! This function calculates the multipliers XXX_108ToSystem which represents 
	 an 14.18 decimal and XXX_SystemTo108 wich is an 18.14 decimal. 
	 If the clock is slower than 0.5 kHz the function will fail.
*/

void Calculate108MhzMultiplier(STFInt64 SystemTicksPerSecond)
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


//! This function initializes the timer system and must be called before any timer operation is performed
/*! This function returns STFRES_TIMEOUT when the timer is not accurate enough or an error occured during
    initialization ('out of time' would be a better error value ;) )
*/

STFResult InitializeOSSTFTimer(void)
	{	
	static bool		initialized = false;

	if (!initialized)
		{
		initialized = true;
		
		Calculate108MhzMultiplier(STFInt64(1000000)); // The resolution of gettimeofday is microseconds
		}

	STFRES_RAISE_OK;
	}
