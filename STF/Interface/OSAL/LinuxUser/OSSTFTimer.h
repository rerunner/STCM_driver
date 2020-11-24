///
/// @brief      STFTimer implementation for Linux PC User Mode
///

#ifndef OSSTFTIMER_H
#define OSSTFTIMER_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFInt64.h"
#include "STF/Interface/Types/STFTime.h"


// must change in OSSTFTimer.cpp and PCMPlayer.cpp
#define DEBUG_SYSTEM_WITH_SLOWED_OUTPUT	0 // 1 = slow down the output for debugging purposes, 0 system at normal speed
#define NUM_SLOW_SHIFTS							2 // 1->50% of normal speed,  2->25% of normal speed, others ???


void Calculate108MhzMultiplier(STFInt64 SystemTicksPerSecond);

class OSSTFTimer
	{
	public:		
		void GetTime(STFHiPrec64BitTime & time);

		STFResult WaitDuration(const STFLoPrec32BitDuration & duration);		

		STFResult WaitIdleTime(uint32 micros);
	};


STFResult InitializeOSSTFTimer(void);

#endif //OSSTFTIMER_H
