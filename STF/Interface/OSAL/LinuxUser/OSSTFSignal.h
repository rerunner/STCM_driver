///
/// @brief      Linux PC User Mode specific signal methods
///

#ifndef OSSTFSIGNAL_H
#define OSSTFSIGNAL_H

// Includes
//#include <os21/event.h>

#include "STF/Interface/Types/STFTime.h"

#define OSSTFSIGNAL_OS21_EVENT_MASK	(0x1)

class OSSTFSignal
	{
	protected:
		STFSemaphore	signal;

//		event_group_t	*	event;

	public:
		OSSTFSignal(void)
			{
//			event = event_group_create(event_auto_clear);
			}

		~OSSTFSignal(void)
			{
//			event_group_delete(event);
			}

		STFResult SetSignal(void)
			{
			STFRES_RAISE(signal.Signal());

//			event_post(event, OSSTFSIGNAL_OS21_EVENT_MASK);

//			STFRES_RAISE_OK;
			}

		STFResult ResetSignal(void)
			{
			STFRES_RAISE(signal.Reset());

//			event_clear(event, OSSTFSIGNAL_OS21_EVENT_MASK);

//			STFRES_RAISE_OK;
			}

		STFResult WaitSignal(void)
			{
//			unsigned int outMask;

//			event_wait_any(event, OSSTFSIGNAL_OS21_EVENT_MASK, &outMask, TIMEOUT_INFINITY);
			STFRES_RAISE(signal.Wait());

//			STFRES_RAISE_OK;
			}

		STFResult WaitImmediateSignal(void)
			{
//			unsigned int outMask;

//			if (event_wait_any(event, OSSTFSIGNAL_OS21_EVENT_MASK, &outMask, TIMEOUT_IMMEDIATE) == OS21_FAILURE)
//				STFRES_RAISE(STFRES_TIMEOUT);

			STFRES_RAISE(signal.WaitImmediate());

//			STFRES_RAISE_OK;
			}
	};

class OSSTFTimeoutSignal : public OSSTFSignal
	{
	public:
		STFResult WaitTimeoutSignal(const STFLoPrec32BitDuration & duration)
			{
			STFRES_RAISE(signal.WaitTimeout(duration));

//			unsigned int outMask;
//			STFInt64 longDuration = duration.Get64BitDuration(STFTU_HIGHSYSTEM);
//			osclock_t clock = ((osclock_t) longDuration.Lower()) | (((osclock_t) longDuration.Upper()) << 32);
//			osclock_t	 time = time_plus(time_now(), clock);
			

//			if (event_wait_any(event, OSSTFSIGNAL_OS21_EVENT_MASK, &outMask, &time) == OS21_FAILURE)
//				STFRES_RAISE(STFRES_TIMEOUT);

//			STFRES_RAISE_OK;
			}
	};

class OSSTFArmedSignal
	{
	protected:
		OSSTFSignal				event;
		OSSTFInterlockedInt	counter;
	public:
		OSSTFArmedSignal(void)
			{
			counter = 0;
			}

		~OSSTFArmedSignal(void)
			{
			}

		STFResult ArmSignal(uint16 n)
			{
			counter += n;

			STFRES_RAISE_OK;
			}

		STFResult SetSignal(uint16 n)
			{
			if ((counter - n) <= 0)
				{
				counter = 0;
				event.SetSignal();
				}

			STFRES_RAISE_OK;
			}

		STFResult ResetSignal(void)
			{
			event.ResetSignal();

			STFRES_RAISE_OK;
			}

		STFResult WaitSignal(void)
			{
			event.WaitSignal();

			STFRES_RAISE_OK;
			}

		STFResult WaitImmediateSignal(void)
			{
			STFRES_RAISE(event.WaitImmediateSignal());
			}

	};

#endif // OSSTFSIGNAL_H
