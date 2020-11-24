
#ifndef OSSTFSEMAPHORE_H
#define OSSTFSEMAPHORE_H

 // Includes
#include <windows.h>
#include <assert.h>
#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFTime.h"

// Globals
STFResult OSSTFGlobalLock(void);
STFResult OSSTFGlobalUnlock(void);


// Classes
class OSSTFInterlockedInt
	{
	protected:
		int32		value;
	public:
		OSSTFInterlockedInt(int32 init = 0)
			: value(init) {}

		int32 operator=(int32 val)
			{
			value = val;
			return val;
			}
		
		int32 operator++(void)
			{
			return ::InterlockedIncrement((long *)&value);
			}

		int32 operator++(int)
			{
			return ::InterlockedIncrement((long *)&value) - 1;
			}

		int32 operator--(void)
			{
			return ::InterlockedDecrement((long *)&value);
			}

		int32 operator--(int)
			{
			return ::InterlockedDecrement((long *)&value) + 1;
			}

		int32 operator+=(int add)	
			{
			return ::InterlockedExchangeAdd((long *)&value, add) + add;
			}

		int32 operator-=(int sub)
 			{
			return ::InterlockedExchangeAdd((long *)&value, -sub) -sub;
			}

		int32 CompareExchange(int32 cmp, int32 val)
			{
			return ::InterlockedCompareExchange((long *)(&value), val, cmp);
			}

		operator int32(void)
			{
			return value;
			}
		};

class OSSTFInterlockedPointer
	{
	protected:
		pointer	ptr;
	public:
		OSSTFInterlockedPointer(pointer val = NULL)
			: ptr(val) {}

		pointer operator=(pointer val)
			{
			ptr = val;
			return val;
			}

		pointer CompareExchange(pointer cmp, pointer val)
			{
#ifndef InterlockedCompareExchangePointer
			return ::InterlockedCompareExchange(&ptr, val, cmp);
#else
			return ::InterlockedCompareExchangePointer(&ptr, val, cmp);
#endif
			}

		operator pointer(void)
			{
			return ptr;
			}
	};

class OSSTFSemaphore
	{
	protected:
		HANDLE	sema;
	public:
		OSSTFSemaphore (void);
		virtual ~OSSTFSemaphore (void);

		STFResult Reset(void);
		STFResult Signal(void);
		STFResult Wait(void);
		STFResult WaitImmediate(void);
		STFResult WaitTimeout(const STFLoPrec32BitDuration & duration);
	};

class OSSTFTimeoutSemaphore : public OSSTFSemaphore
	{
	public:
		OSSTFTimeoutSemaphore (void) : OSSTFSemaphore() { }
		virtual ~OSSTFTimeoutSemaphore (void) { }
	};


// *********************************************
// Inlines
// *********************************************


inline OSSTFSemaphore::OSSTFSemaphore (void)
	{
	sema = ::CreateSemaphore(NULL, NULL, 1000000, NULL);

	assert(sema != NULL);
	}

inline OSSTFSemaphore::~OSSTFSemaphore (void)
	{
	if (sema)
		CloseHandle(sema);
	}

inline STFResult OSSTFSemaphore::Reset (void)
	{
	do {} while (::WaitForSingleObject(sema, 0) != WAIT_TIMEOUT);

	STFRES_RAISE_OK;
	}

inline STFResult OSSTFSemaphore::Signal (void)
	{
	if (::ReleaseSemaphore(sema, 1, NULL) == 0)
		STFRES_RAISE(STFRES_OPERATION_FAILED);

	STFRES_RAISE_OK;
	}

inline STFResult OSSTFSemaphore::Wait(void)
	{
	::WaitForSingleObject(sema, INFINITE);
	STFRES_RAISE_OK;
	}

inline STFResult OSSTFSemaphore::WaitImmediate(void)
	{
	DWORD error;
	
	error = ::WaitForSingleObject(sema, 0);
	if (error == WAIT_TIMEOUT)
		STFRES_RAISE(STFRES_TIMEOUT);

	STFRES_RAISE_OK;
	}

inline STFResult OSSTFSemaphore::WaitTimeout(const STFLoPrec32BitDuration & duration)
	{
	DWORD error;
	
	error = ::WaitForSingleObject(sema, duration.Get32BitDuration(STFTU_MILLISECS));
	if (error == WAIT_TIMEOUT)
		STFRES_RAISE(STFRES_TIMEOUT);

	STFRES_RAISE_OK;
	}

#endif //OSSTFSEMAPHORE_H
