///
/// @brief      Linux PC User Mode specific semaphore (and synchronisation) methods 
///

#ifndef OSSTFSEMAPHORE_H
#define OSSTFSEMAPHORE_H

// Includes
#include <semaphore.h>

#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFTime.h"


//Globals
STFResult OSSTFGlobalLock(void);
STFResult OSSTFGlobalUnlock(void);


class OSSTFSemaphore
	{
	protected:
		sem_t sema;

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


//! Thread-safe and Interrupt-safe access to an integer variable
class OSSTFInterlockedInt
	{
	protected:
		volatile int32		value;

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
			OSSTFGlobalLock();
			int32 result = ++value;
			OSSTFGlobalUnlock();
			return result;
			}

		int32 operator++(int)
			{
			OSSTFGlobalLock();
			int32 result = value++;
			OSSTFGlobalUnlock();
			return result;
			}

		int32 operator--(void)
			{
			OSSTFGlobalLock();
			int32 result = --value;
			OSSTFGlobalUnlock();
			return result;
			}

		int32 operator--(int)
			{
			OSSTFGlobalLock();
			int32 result = value--;
			OSSTFGlobalUnlock();
			return result;
			}

		int32 operator+=(int add)	
			{
			OSSTFGlobalLock();
			int32 result = (value += add);
			OSSTFGlobalUnlock();
			return result;
			}

		int32 operator-=(int sub)
 			{
			OSSTFGlobalLock();
			int32 result = (value -= sub);
			OSSTFGlobalUnlock();
			return result;
			}

		int32 CompareExchange(int32 cmp, int32 val)
			{
			int32 p;

			// Compare value to cmp. If they match, replace value by val and return old value
			OSSTFGlobalLock();

			p = value;
			if (value == cmp)
				value = val;

			OSSTFGlobalUnlock();

			return p;
			}

		operator int32(void)
			{
			return value;
			}
		};

class OSSTFInterlockedPointer
	{
	protected:
		volatile pointer					ptr;
    
	public:
		OSSTFInterlockedPointer(pointer val = NULL)
			: ptr(val) {}

		pointer operator=(pointer val)
			{
			OSSTFGlobalLock();

			ptr = val;

			OSSTFGlobalUnlock();

			return val;
			}

		pointer CompareExchange(pointer cmp, pointer val)
			{
			pointer p;

			// Compare ptr to cmd. If they match, replace ptr by val and return old ptr value
			OSSTFGlobalLock();

			p = ptr;
			if (ptr == cmp)
				ptr = val;

			OSSTFGlobalUnlock();

			return p;
			}

		operator pointer(void)
			{
			return ptr;
			}
	};

#endif
