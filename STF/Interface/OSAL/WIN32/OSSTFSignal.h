
#include "STF/Interface/STFSemaphore.h"

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

		STFResult ArmSignal(uint16 n)
			{
			counter += n;

			STFRES_RAISE_OK;
			}

		STFResult SetSignal(uint16 n)
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
