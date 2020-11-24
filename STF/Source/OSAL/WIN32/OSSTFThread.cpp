///
/// @brief Implementation for Win32 specific threads
///

// Set to see more debug messages (default of 0 only prints sever warnings)
#define DEBUG_THREAD_CONTROL (_DEBUG && 0)


#include "OSSTFThread.h"

// The Windows priority scheme:
// IDLE          -15
// LOWEST        -2
// BELOW_NORMAL  -1
// NORMAL         0
// ABOVE_NORMAL   1
// HIGHEST        2
// TIME_CRITICAL 15

// ??? @TODO: Evaluate if more intermediate steps can be used, to create a unique mapping
//            of STF to Windows priorities.

static uint32 PriorityList[17] = 
	{
	THREAD_PRIORITY_IDLE, 
	THREAD_PRIORITY_IDLE, 
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_TIME_CRITICAL,
	THREAD_PRIORITY_TIME_CRITICAL
	};

static uint32	tlsIndex;

DWORD __stdcall OSSTFThreadEntryCall(void * p)
	{
	((OSSTFThread *)p)->ThreadEntry();

	return 0;
	}

void OSSTFThread::ThreadEntry(void)
	{
	::TlsSetValue(tlsIndex, (LPVOID)bthread);

	bthread->ThreadEntry();
	}

OSSTFThread::OSSTFThread(STFBaseThread * bthread, STFString name, uint32 stackSize, STFThreadPriority priority)
	{
	this->bthread = bthread;
	this->name = name;
	this->stackSize = max (stackSize, OSAL_MIN_STACK_SIZE);
	this->priority = priority;
	handle = INVALID_HANDLE_VALUE;

	prioritySet		= true;
	stackSizeSet	= true;
	}


OSSTFThread::OSSTFThread(STFBaseThread * bthread, STFString name)
	{
	this->bthread = bthread;
	this->name = name;
	handle = INVALID_HANDLE_VALUE;

	prioritySet		= false;
	stackSizeSet	= false;
	}


OSSTFThread::OSSTFThread(STFBaseThread * bthread)
	{
	this->bthread = bthread;
	this->name = "root thread";
   this->stackSize = OSAL_MIN_STACK_SIZE; //Dummy value. Thread already exists.
	this->priority = STFTP_NORMAL;
	::DuplicateHandle(::GetCurrentProcess(), ::GetCurrentThread(), GetCurrentProcess(), &(this->handle), 0, 0, DUPLICATE_SAME_ACCESS);

	::TlsSetValue(tlsIndex, (LPVOID)bthread);

	prioritySet		= true;
	stackSizeSet	= true;
	}


OSSTFThread::~OSSTFThread(void)
	{
	TerminateThread();
	}


STFResult OSSTFThread::SetThreadStackSize(uint32 stackSize)
	{
	this->stackSize = stackSize;
	stackSizeSet = true;
	STFRES_RAISE_OK;
	}


STFResult OSSTFThread::SetThreadName(STFString name)
	{
	this->name = name;
	STFRES_RAISE_OK;
	}


STFResult OSSTFThread::StartThread(void)
	{
	DWORD id;

	if (handle != INVALID_HANDLE_VALUE && bthread->terminate)
		{
		STFRES_REASSERT(Wait());
		}

	if (handle == INVALID_HANDLE_VALUE)
		{
		// Do not allow starting the thread if there's no priority or stack size set.
		if (!prioritySet || !stackSizeSet)
			{
			DP("WARNING: Could not start thread because no stack size or priority set!\n");
			STFRES_RAISE(STFRES_OPERATION_PROHIBITED);
			}

		bthread->terminate = false;

		handle = ::CreateThread(NULL, stackSize, OSSTFThreadEntryCall, this, 0, (LPDWORD)&id);
		if (handle == INVALID_HANDLE_VALUE)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

#if DEBUG_THREAD_CONTROL
		char threadName[100];
		this->name.Get(threadName, 100);
		DP("StartThread of %s with priority %d, OS priority %d, stack %d, OS handle %08x\n", threadName, priority, PriorityList[priority * 16 / STFTP_CRITICAL], stackSize, handle);
#endif

		::SetThreadPriority(handle, PriorityList[priority * 16 / STFTP_CRITICAL]);

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EXISTS);
	}

STFResult OSSTFThread::StopThread(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		bthread->terminate = true;

		STFRES_REASSERT(bthread->NotifyThreadTermination());

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult OSSTFThread::SuspendThread(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		::SuspendThread(handle);

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult OSSTFThread::ResumeThread(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		::ResumeThread(handle);

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult OSSTFThread::TerminateThread(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		::TerminateThread(handle, 0);
		::CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		}

	STFRES_RAISE_OK;
	}

STFResult OSSTFThread::Wait(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		::WaitForSingleObject(handle, INFINITE);

		::CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult OSSTFThread::WaitImmediate(void)
	{
	if (handle != INVALID_HANDLE_VALUE)
		{
		if (::WaitForSingleObject(handle, 0) == WAIT_TIMEOUT)
			STFRES_RAISE(STFRES_TIMEOUT);

		::CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;

		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}

STFResult OSSTFThread::SetThreadPriority(STFThreadPriority pri)
	{
	prioritySet = true;

	if (handle != INVALID_HANDLE_VALUE)
		{
		if (priority != pri)
			{
			::SetThreadPriority(handle, PriorityList[priority * 16 / STFTP_CRITICAL]);
			};
		}

	priority = pri;

	STFRES_RAISE_OK;
	}

/// Retrieve the name of the thread
STFString OSSTFThread::GetName(void) const
	{
	return this->name;
	}

class STFRootThread : public STFThread
	{
	protected:
		void ThreadEntry(void) {}
		STFResult NotifyThreadTermination(void) {STFRES_RAISE_OK;}

	};


STFResult InitializeSTFThreads(void)
	{
	static bool initialized = false;

	if (!initialized)
		{
		initialized = true;

		tlsIndex = TlsAlloc();

		new STFRootThread();
		}
	else
		{
		DP("Warning: InitializeSTFThreads() has been called more than once!!\n");
		assert(false);
		}

	STFRES_RAISE_OK;
	}

STFResult GetCurrentSTFThread(STFThread * & thread)
	{
	thread = (STFThread *)::TlsGetValue(tlsIndex);

	STFRES_RAISE_OK;
	}


