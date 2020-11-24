///
/// @brief Header for Linux User Mode specific threads
///

// STFThread.h **MUST** be included before the guarding ifdef as it
// circularly includes this file to get the class definitions in the
// right order.
#include "STF/Interface/STFThread.h"

#ifndef OSSTFTHREADS_H
#define OSSTFTHREADS_H

#include <pthread.h>

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFString.h"

#include "OSDefines.h"


///
/// @class OSSTFThread

class OSSTFThread
	{
   private:
      int GetPthreadPriority(int policy, STFThreadPriority pri);
      STFResult ChangePthreadScheduling(int newPolicy, int newPriority);
      
	protected:
		pthread_t					pthread;
		STFBaseThread			*	bthread;
		
		bool							prioritySet, stackSizeSet;
		
		STFString					name;
		uint32						stackSize;
		STFThreadPriority			priority;

		friend void OSSTFThreadEntryCall(void * p);

		void ThreadEntry(void);

	public:
		OSSTFThread(void)
			{
			prioritySet		= false;
			stackSizeSet	= false;
         bthread = 0;
         pthread = 0;
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
		STFString GetName(void) const { return name; }
	};

#endif //OSSTFTHREAD_H
