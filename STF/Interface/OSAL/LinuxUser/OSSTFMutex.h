///
/// @brief OS-dependent mutex-methods 
///

#ifndef OSSTFMutex_H
#define OSSTFMutex_H

#include <pthread.h>

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"

// Classes
class OSSTFMutex
	{
	private:
      static const pthread_mutex_t mutexTemplate;
      pthread_mutex_t mutex;
	public:
		OSSTFMutex(void): mutex(mutexTemplate) {}
		~OSSTFMutex(void) {}

		STFResult Enter(void)
			{
         // For this mutex type the lock/unlock calls can only fail if the
         // mutex structure is not correctly initialised. As the mutex is
         // always initialised by the constructor we are deliberately ignoring
         // the return value.
         pthread_mutex_lock(&mutex);
         STFRES_RAISE_OK;
			}

		STFResult Leave(void)
			{
         pthread_mutex_unlock(&mutex);
         STFRES_RAISE_OK;
			}
	};		

typedef OSSTFMutex OSSTFLocalMutex;

class OSSTFSharedMutex
	{
	private:
      // Use a fast, non-recursive, mutex for the lock
      static const pthread_mutex_t lockTemplate;      
      pthread_mutex_t lock;
      
		bool            shared;
		int32				 count;

      OSSTFMutex      mutex;
	public:
		OSSTFSharedMutex(void): lock(lockTemplate), mutex()
			{
			shared = false;
			count  = 0;
			}

		~OSSTFSharedMutex(void) {}

		STFResult Enter(bool exclusive)
			{
         if (exclusive)
            mutex.Enter();
			else
				{
            pthread_mutex_lock(&lock);

				if (!shared)
					{
               mutex.Enter();
					count = 1;
					shared = true;
					}
				else
					++count;

            pthread_mutex_unlock(&lock);
				}

			STFRES_RAISE_OK;
			}

		STFResult Leave(void)
			{
			if (shared)
				{
            pthread_mutex_lock(&lock);
            
				if (--count == 0)
					{
					shared = false;
               mutex.Leave();
					}
             
            pthread_mutex_unlock(&lock);
				}
			else
            mutex.Leave();
            
			STFRES_RAISE_OK;
			}
	};

#endif	// OSSTFMUTEX_H
