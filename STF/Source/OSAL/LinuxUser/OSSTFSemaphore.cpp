///
/// @brief      Linux PC User Mode specific semaphore (and synchronisation) methods 
///

#include <errno.h>

// Includes
#include "OSSTFSemaphore.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFMutex.h"
#include "STF/Interface/STFDebug.h"


static STFMutex GlobalLockMutex;

// Globals
STFResult OSSTFGlobalLock(void)
   {
	GlobalLockMutex.Enter();

   STFRES_RAISE_OK;
   }

STFResult OSSTFGlobalUnlock(void)
   {
	GlobalLockMutex.Leave();

   STFRES_RAISE_OK;
   }


// OSSTFSemaphore
// ----------------

OSSTFSemaphore::OSSTFSemaphore (void)
	{
	int res;

	res = sem_init(&sema, 0, 0);	// Initialize unnamed semaphore. The 2nd parameter indicates if the
											// semaphore can be shared between processes, the 3rd is the initial
											// count of the semaphore.

	ASSERT(res == 0);

	}


OSSTFSemaphore::~OSSTFSemaphore (void)
	{
	int res;

	res = sem_destroy(&sema);

	if (res == -1 && errno == EBUSY)
		DP("~OSSTFSemaphore: sem_destroy while threads are still waiting on the semaphore!\n");

	ASSERT(res == 0);		
	}


STFResult OSSTFSemaphore::Reset (void)
	{
	do {} while (WaitImmediate() == STFRES_OK);

	STFRES_RAISE_OK;
	}


STFResult OSSTFSemaphore::Signal (void)
	{
	int res;

	res = sem_post(&sema);

	if (res != 0)
		STFRES_RAISE(STFRES_OPERATION_FAILED);	

	STFRES_RAISE_OK;
	}


STFResult OSSTFSemaphore::Wait (void)
	{
	int res;

	// We have too loop on sem_wait because it could be interrupted by a signal,
	// for example while debugging in gdb or ddd.
	while((res = sem_wait(&sema)) == -1 && errno == EINTR);

   ASSERT(res == 0);
   
	STFRES_RAISE_OK;
	}


STFResult OSSTFSemaphore::WaitImmediate(void)
	{
	int res;
	
	res = sem_trywait(&sema);
	
	if (res != 0)
		{
		STFRES_RAISE(STFRES_OPERATION_FAILED);
		}

	STFRES_RAISE_OK;
	}


STFResult OSSTFSemaphore::WaitTimeout(const STFLoPrec32BitDuration & duration)
	{	
	int res;
	struct timespec time;
	STFHiPrec64BitTime targetTime;

	SystemTimer->GetTime(targetTime);
	targetTime += duration;

	STFInt64 millis = targetTime.Get64BitTime(STFTU_MILLISECS);
	STFInt64 seconds = millis / 1000;
	millis = millis - seconds * 1000;
	
	time.tv_sec = (time_t) seconds.ToInt32();
	time.tv_nsec = millis.ToInt32() * 1000;

	DP("%s : Starting\n", __FUNCTION__);
	res = sem_timedwait(&sema, &time);
	DP("%s : Finishing\n", __FUNCTION__);

	if (res != 0)
		{
		if (errno == ETIMEDOUT)
			STFRES_RAISE(STFRES_TIMEOUT);
		DP("%s : errno = %d.\n", __FUNCTION__, errno);
		STFRES_RAISE(STFRES_OPERATION_FAILED);
		}
	
	STFRES_RAISE_OK;
	}

