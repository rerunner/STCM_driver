///
/// @brief Implementation for Linux User Mode specific threads
///

#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>

#include "STF/Interface/STFDebug.h"
#include "OSSTFThread.h"


// The following needs to be set to SCHED_FIFO if we want realtime behaviour of the threads
// If thread priorities are to be ignored, set it to SCHED_OTHER
#define SCHEDULING_TYPE	SCHED_FIFO

static pthread_key_t tlsKey;

#if !defined(DEBUG_THREAD_CONTROL)
// Make sure this is defined for OSSTFThread::ThreadCreate
#define DEBUG_THREAD_CONTROL 0
#endif

void OSSTFThreadEntryCall(void * p)
   {
   ((OSSTFThread *)p)->ThreadEntry();
   }

extern "C" 
   {
   static void *OSSTFThreadEntryCallC(void * p)
      {
      OSSTFThreadEntryCall(p);
      return 0;
      }
   }

void OSSTFThread::ThreadEntry(void)
   {
   pthread_setspecific(::tlsKey, bthread);

   bthread->ThreadEntry();
   }

OSSTFThread::OSSTFThread(STFBaseThread * bthread, STFString name, uint32 stackSize, STFThreadPriority priority)
   {
   this->bthread   = bthread;
   this->name      = name;
   this->stackSize = max(stackSize, PTHREAD_STACK_MIN); // ASSERT Would be better than hiding bug
   this->priority  = priority;

   prioritySet	 = true;
   stackSizeSet = true;
   pthread      = 0;
   }


OSSTFThread::OSSTFThread(STFBaseThread * bthread, STFString name)
   {
   this->bthread = bthread;
   this->name    = name;

   prioritySet  = false;
   stackSizeSet = false;
   pthread      = 0;
   }

// This constructor is only used ONCE to create a representation
// of the root thread of the system, i.e. the current context.
// We have to do some extra work to turn this into a thread with the
// same scheduling properties as threads created dynamically.
OSSTFThread::OSSTFThread(STFBaseThread * bthread)
   {
   this->bthread   = bthread;
   this->name      = "root thread";
   this->stackSize = PTHREAD_STACK_MIN; // Dummy, thread exists
   this->priority  = STFTP_NORMAL;
   this->pthread   = pthread_self();
   
   pthread_setspecific(tlsKey, bthread);

   STFResult res = ChangePthreadScheduling(SCHEDULING_TYPE,
                                           GetPthreadPriority(SCHEDULING_TYPE,priority));

   ASSERT(!STFRES_IS_ERROR(res));
   
   prioritySet		= true;
   stackSizeSet	= true;
   }


OSSTFThread::~OSSTFThread(void)
   {
   TerminateThread();
   }


STFResult OSSTFThread::SetThreadStackSize(uint32 stackSize)
   {
   
   if(stackSize < PTHREAD_STACK_MIN)
#if 0
      STFRES_RAISE(STFRES_INVALID_PARAMETERS);
#else
      {
      stackSize = PTHREAD_STACK_MIN;
      }
#endif
      
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

   if (pthread != 0 && bthread->terminate)
      {
      STFRES_REASSERT(Wait());
      }

   if (pthread == 0)
      {
      
      // Do not allow starting the thread if there's no priority or stack size set.
      if (!prioritySet || !stackSizeSet)
         {
         DP("OSSTFThread::StartThread Could not start thread because no stack size or priority set!\n");
         STFRES_RAISE(STFRES_OPERATION_PROHIBITED);
         }

      bthread->terminate = false;

/*
  pthread_attr_t pthreadAttr;
  struct sched_param schedParam;
      
  schedParam.sched_priority = GetPthreadPriority(SCHEDULING_TYPE, priority);
  pthread_attr_init(&pthreadAttr);
  pthread_attr_setschedpolicy(&pthreadAttr, SCHEDULING_TYPE);
  pthread_attr_setschedparam (&pthreadAttr, &schedParam);
  pthread_attr_setstacksize  (&pthreadAttr, stackSize);
*/
      
      int ret = pthread_create(&pthread, NULL, OSSTFThreadEntryCallC, (void *)this);
      
//      pthread_attr_destroy(&pthreadAttr);
      
      if (ret != 0)
         {
         pthread = 0; // Make sure this is reset
         DP("OSSTFThread::StartThread pthread_create failed, ret = %d\n", ret);
         STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
         }

      STFRES_REASSERT(ChangePthreadScheduling(SCHEDULING_TYPE,
                                              GetPthreadPriority(SCHEDULING_TYPE, priority)));

      // Note: we use a real if here, not an ifdef to ensure the code will always
      // compile without errors. We rely on the compiler's dead code elimination
      // to get rid of it when DEBUG_THREAD_CONTROL == 0.
      if (DEBUG_THREAD_CONTROL)
         {
         char threadName[100];
         this->name.Get(threadName, 100);
         DP("OSSTFThread::StartThread of %s with priority %d, OS priority %d, stack %d, OS handle %08x\n", threadName, priority, GetPthreadPriority(SCHEDULING_TYPE, priority), stackSize, pthread);
         }

      STFRES_RAISE_OK;
      }
   else
      STFRES_RAISE(STFRES_OBJECT_EXISTS);
      
   }

STFResult OSSTFThread::StopThread(void)
   {
   if (pthread != 0)
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
   DP("OSSTFThread::SuspendThread not supported\n");
   STFRES_RAISE(STFRES_UNIMPLEMENTED);
   }

STFResult OSSTFThread::ResumeThread(void)
   {
   DP("OSSTFThread::ResumeThread not supported\n");
   STFRES_RAISE(STFRES_UNIMPLEMENTED);
   }

STFResult OSSTFThread::TerminateThread(void)
   {
   if (pthread != 0)
      {
      int ret;
      
      ASSERT(!pthread_equal(pthread_self(),pthread));
      
      ret = pthread_kill(pthread, SIGKILL);
      switch(ret)
         {
         case 0:
            ret = pthread_join(pthread, NULL);
            if(ret != 0)
               {
               DP("OSSTFThread::TerminateThread pthread_join failed, ret = %d\n",ret);
               STFRES_RAISE(STFRES_OPERATION_FAILED);
               }

            // Deliberate fall through to clean up state         
         case ESRCH:
            // Thread has already terminated
            pthread = 0;
            STFRES_RAISE_OK;
         default:
            DP("OSSTFThread::TerminateThread pthread_kill failed, ret = %d\n",ret);
            STFRES_RAISE(STFRES_OPERATION_FAILED);   
         }
      }
   else
      STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);

   }

STFResult OSSTFThread::Wait(void)
   {
   if (pthread != 0)
      {
      int ret = pthread_join(pthread, NULL);
         
      if(ret != 0)
         {
         DP("OSSTFThread::Wait pthread_join failed, ret = %d\n",ret);
         STFRES_RAISE(STFRES_OPERATION_FAILED);
         }

      pthread = 0;
      STFRES_RAISE_OK;
      }
   else
      STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
      
   }

STFResult OSSTFThread::WaitImmediate(void)
   {
   if (pthread != 0)
      {
      struct sched_param schedParam;
      int policy, ret;
   
      // Use the pthread_getschedparam call to determine if
      // the thread has exited. Is there a more official method?      
      ret = pthread_getschedparam(pthread, &policy, &schedParam);
      if(ret != ESRCH)
         STFRES_RAISE(STFRES_TIMEOUT);
            
      ret = pthread_join(pthread, NULL);
      if(ret != 0)
         {
         DP("OSSTFThread::WaitImmediate pthread_join failed, ret = %d\n",ret);
         STFRES_RAISE(STFRES_OPERATION_FAILED);
         }

      pthread = 0;
      STFRES_RAISE_OK;
      }
   else
      STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
   }

int OSSTFThread::GetPthreadPriority(int policy, STFThreadPriority stfPriority)
   {
   int pmin,pmax,range,pri;
   
   ASSERT(policy == SCHEDULING_TYPE || policy == SCHED_RR);
   
   pmin = sched_get_priority_min(policy);
   pmax = sched_get_priority_max(policy);
   
   range = pmax - pmin;
   
   // Calculate the pthread priority, note that
   // pmin will typically not be zero
   pri = pmin + ((stfPriority*range) / STFTP_CRITICAL);

   ASSERT(pri >= pmin && pri <= pmax);
   
   return pri;
   }

STFResult OSSTFThread::ChangePthreadScheduling(int newPolicy, int newPriority)
   {
   if (pthread != 0)
      {
      struct sched_param schedParam;
      int oldPolicy, ret;
         
      ret = pthread_getschedparam(pthread, &oldPolicy, &schedParam);
      if(ret != 0)
         {
         DP("OSSTFThread::SetThreadPriority pthread_getschedparam failed, ret = %d\n", ret);
         STFRES_RAISE(STFRES_OBJECT_INVALID);
         }

      schedParam.sched_priority = newPriority;
         
      ret = pthread_setschedparam(pthread, newPolicy, &schedParam);
      if(ret != 0)
         {
         DP("OSSTFThread::SetThreadPriority pthread_setschedparam failed, ret = %d. I guess you are running as User, not as Root.\n", ret);
#if 0 /* Running as user for the moment */
         switch(ret)
            {
            case EINVAL:
               STFRES_RAISE(STFRES_INVALID_PARAMETERS);
            case EPERM:
               STFRES_RAISE(STFRES_INSUFFICIENT_RIGHTS);
            default:
               STFRES_RAISE(STFRES_OBJECT_INVALID);
            }
#endif
         }         
      }
      
   STFRES_RAISE_OK;
   }

STFResult OSSTFThread::SetThreadPriority(STFThreadPriority pri)
   {
   prioritySet = true;

   if(priority	!= pri)
      STFRES_REASSERT(ChangePthreadScheduling(SCHEDULING_TYPE,
                                              GetPthreadPriority(SCHEDULING_TYPE, pri)));
      
   priority = pri;
   STFRES_RAISE_OK;
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
      int ret;
      initialized = true;

      ret = pthread_key_create(&tlsKey,NULL);
      if(ret != 0)
         {
         DP("Warning: InitializeSTFThreads() pthread_key_create failed, ret = %d\n",ret);
         assert(false);
         }

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
   thread = (STFThread *)pthread_getspecific(tlsKey);

   STFRES_RAISE_OK;
   }


