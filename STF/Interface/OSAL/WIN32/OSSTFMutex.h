
#include "STF/Interface/STFSemaphore.h"


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

class OSSTFSharedMutex
	{
	protected:
		DWORD		owner;
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
			DWORD	me = ::GetCurrentThreadId();

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

