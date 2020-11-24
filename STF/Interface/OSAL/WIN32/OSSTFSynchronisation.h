
// Platform: Win32

#ifndef OSSTFSYNCHRONISATION_H
#define OSSTFSYNCHRONISATION_H

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

class OSSTFMutex
	{
	protected:
		CRITICAL_SECTION	lock;
	public:
		OSSTFMutex(void);
		~OSSTFMutex(void);

		STFResult Enter(void);
		STFResult Leave(void);
	};

class OSSTFSemaphore
	{
	protected:
		HANDLE	sema;
	public:
		OSSTFSemaphore (void);
		~OSSTFSemaphore (void);

		STFResult Reset(void);
		STFResult Signal(void);
		STFResult Wait(void);
		STFResult WaitImmediate(void);
		STFResult WaitTimeout(const STFLoPrec32BitDuration & duration);
	};

class OSSTFSharedMutex
	{
	protected:
		HANDLE	owner;
		bool		shared;
		int32		count;
		CRITICAL_SECTION	lock, wlock;
	public:
		OSSTFSharedMutex(void)
			{
			owner = NULL;
			shared = false;
			count = 0;
			::InitializeCriticalSection(&lock);
			::InitializeCriticalSection(&wlock);
			}

		~OSSTFSharedMutex(void)
			{
			::DeleteCriticalSection(&lock);
			::DeleteCriticalSection(&wlock);
			}

		STFResult Enter(bool exclusive)
			{
			HANDLE	me = ::GetCurrentThread();

			if (me == owner)
				{
				++count;
				}
			else if (exclusive)
				{
				::EnterCriticalSection(&wlock);
				count = 1;
				owner = me;
				}
			else
				{
				::EnterCriticalSection(&lock);

				if (!shared)
					{
					::EnterCriticalSection(&wlock);
					count = 1;
					shared = true;
					}
				else
					++count;

				::LeaveCriticalSection(&lock);
				}

			STFRES_RAISE_OK;
			}

		STFResult Leave(void)
			{
			if (shared)
				{
				EnterCriticalSection(&lock);
				if (--count == 0)
					{
					shared = false;
					LeaveCriticalSection (&wlock);
					}
				LeaveCriticalSection(&lock);
				}
			else
				{
				if (--count == 0)
					{
					owner = NULL;
					LeaveCriticalSection (&wlock);
					}
				}

			STFRES_RAISE_OK;
			}

	};

class OSSTFSignal
	{
	protected:
		HANDLE		event;
	public:
		OSSTFSignal(void)
			{
			event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			}

		~OSSTFSignal(void)
			{
			::CloseHandle(event);
			}

		STFResult SetSignal(void)
			{
			::SetEvent(event);

			STFRES_RAISE_OK;
			}

		STFResult ResetSignal(void)
			{
			::ResetEvent(event);

			STFRES_RAISE_OK;
			}

		STFResult WaitSignal(void)
			{
			::WaitForSingleObject(event, INFINITE);

			STFRES_RAISE_OK;
			}

		STFResult WaitImmediateSignal(void)
			{
			HRESULT	hr;

			hr = ::WaitForSingleObject(event, 0);
			if (hr == WAIT_TIMEOUT)
				STFRES_RAISE(STFRES_TIMEOUT);

			STFRES_RAISE_OK;
			}
	};


class OSSTFTimeoutSignal : public OSSTFSignal
	{
	public:
		STFResult WaitTimeoutSignal(const STFLoPrec32BitDuration & duration)
			{
			HRESULT	hr;

			hr = ::WaitForSingleObject(event, duration.Get32BitDuration(STFTU_MILLISECS));

			if (hr == WAIT_TIMEOUT)
				STFRES_RAISE(STFRES_TIMEOUT);

			STFRES_RAISE_OK;
			}
	};


class OSSTFArmedSignal
	{
	protected:
		HANDLE					event;
		OSSTFInterlockedInt	counter;
	public:
		OSSTFArmedSignal(void)
			{
			event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			counter = 0;
			}

		~OSSTFArmedSignal(void)
			{
			::CloseHandle(event);
			}

		STFResult ArmSignal(int n)
			{
			counter += n;

			STFRES_RAISE_OK;
			}

		STFResult SetSignal(int n)
			{
			counter -= n;

			if (counter <= 0)
				{
				::SetEvent(event);
				counter = 0;
				}

			STFRES_RAISE_OK;
			}

		STFResult ResetSignal(void)
			{
			counter = 0;
			::ResetEvent(event);

			STFRES_RAISE_OK;
			}

		STFResult WaitSignal(void)
			{
			::WaitForSingleObject(event, INFINITE);

			STFRES_RAISE_OK;
			}

		STFResult WaitImmediateSignal(void)
			{
			HRESULT	hr;

			hr = ::WaitForSingleObject(event, 0);
			if (hr == WAIT_TIMEOUT)
				STFRES_RAISE(STFRES_TIMEOUT);

			STFRES_RAISE_OK;
			}

	};


// *********************************************
// Inlines
// *********************************************

inline OSSTFMutex::OSSTFMutex(void)
	{
	::InitializeCriticalSection(&lock);
	}

inline OSSTFMutex::~OSSTFMutex(void)
	{
	::DeleteCriticalSection(&lock);
	}

inline STFResult OSSTFMutex::Enter()
	{
	::EnterCriticalSection(&lock);

	STFRES_RAISE_OK;
	}

inline STFResult OSSTFMutex::Leave(void)
	{
	::LeaveCriticalSection(&lock);

	STFRES_RAISE_OK;
	}


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


#endif // OSSTFSYNCHRONISATION_H
