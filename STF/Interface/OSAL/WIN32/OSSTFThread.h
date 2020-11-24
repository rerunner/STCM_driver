
#include "STF/Interface/STFThread.h"

#ifndef OSSTFTHREADS_H
#define OSSTFTHREADS_H

#include <windows.h>

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFString.h"

#include "OSDefines.h"

///
/// @class OSSTFThread

class OSSTFThread
	{
	protected:
		HANDLE						handle;
		STFBaseThread			*	bthread;

		bool							prioritySet, stackSizeSet;
		
		STFString					name;
		uint32						stackSize;
		STFThreadPriority			priority;

		friend DWORD __stdcall OSSTFThreadEntryCall(void * p);

		void ThreadEntry(void);

	public:
		OSSTFThread(void)
			{
			prioritySet		= false;
			stackSizeSet	= false;
			}

		OSSTFThread(STFBaseThread * bthread, STFString name, uint32 stackSize, STFThreadPriority priority);
		OSSTFThread(STFBaseThread * bthread, STFString name);
		OSSTFThread(STFBaseThread * bthread);

		~OSSTFThread(void);

		STFResult SetThreadStackSize(uint32 stackSize);
		STFResult SetThreadName(STFString name);

		STFResult StartThread(void);
		STFResult StopThread(void);

		STFResult SuspendThread(void);
		STFResult ResumeThread(void);
		
		STFResult TerminateThread(void);

		STFResult Wait(void);
		STFResult WaitImmediate(void);

		STFResult SetThreadPriority(STFThreadPriority pri);

		STFResult SetThreadSignal(void);
		STFResult ResetThreadSignal(void);
		STFResult WaitThreadSignal(void);

		/// Retrieve the name of the thread
		STFString GetName(void) const;

		bool GetThreadStatusRunning(void);
	};

#endif //OSSTFTHREAD_H
