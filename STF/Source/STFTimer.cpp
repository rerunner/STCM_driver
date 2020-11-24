#include "STF/Interface/STFTimer.h"
#include "ThreadConfiguration.h"

STFTimer * SystemTimer;

STFTimer::STFTimer(void)
	: STFThread(TCTN_EVENT_SCHEDULER, TCSS_EVENT_SCHEDULER, TCTP_EVENT_SCHEDULER)
	{			
	count = 0;

	for (uint32 i = 0; i < STFTIMER_MAX_EVENTS; i++)
		pendingEvents[i].sink = NULL;

	StartThread();
	}

STFTimer::~STFTimer()
	{
	}


void STFTimer::ThreadEntry(void)
	{
	STFHiPrec64BitTime      currentTime, nextTime;
	STFLoPrec32BitDuration  duration;

	STFMessageSink *        sink;
	// uint32                  id;
	int                     nextEvent;
	uint32                  i;
	// bool                    timeout;

	while (!terminate)
		{
		nextEvent = -1;

		mutex.Enter();

		for(i=0; i<STFTIMER_MAX_EVENTS; i++)
			{
			if (pendingEvents[i].sink && (nextEvent == -1 || pendingEvents[i].time < nextTime))
				{
				nextTime = pendingEvents[i].time;
				nextEvent = i;
				}
			}

		if (nextEvent >= 0)
			{
			GetTime(currentTime);
			if (nextTime < currentTime)
				{
				sink = pendingEvents[nextEvent].sink;
				// id = pendingEvents[nextEvent].id;
				if (pendingEvents[nextEvent].recurrent)
					{
					pendingEvents[nextEvent].time += pendingEvents[nextEvent].dueCycleDuration;
					}
				else
					{
					pendingEvents[nextEvent].sink = NULL;
					}
				mutex.Leave();
				sink->SendMessage(pendingEvents[nextEvent].message, false);
				}
			else
				{
				duration = nextTime - currentTime;

				mutex.Leave();
				// timeout = signal.WaitTimeout(duration) == STFRES_TIMEOUT;
				signal.WaitTimeout(duration);
				}

			}
		else
			{
			mutex.Leave();
			signal.Wait();
			}
		}
	}

STFResult STFTimer::NotifyThreadTermination(void)
	{
	signal.SetSignal();

	STFRES_RAISE_OK;
	}		

STFResult STFTimer::ScheduleEvent(const STFHiPrec64BitTime & time, STFMessageSink * sink, const STFMessage & message, uint32 & timer)
	{
	uint32 i;
	STFResult err = STFRES_OK;

	mutex.Enter();

	i = 0;
	while (i < STFTIMER_MAX_EVENTS && pendingEvents[i].sink)
		i++;

	if (i < STFTIMER_MAX_EVENTS)
		{
		pendingEvents[i].sink = sink;
		pendingEvents[i].message = message;
		pendingEvents[i].time = time;
		pendingEvents[i].id = timer = i + count;
		pendingEvents[i].recurrent = false;
		count += STFTIMER_MAX_EVENTS;

		signal.SetSignal();
		}
	else
		err = STFRES_OBJECT_FULL;

	mutex.Leave();

	STFRES_RAISE(err);
	}

STFResult STFTimer::ScheduleRecurrentEvent(const STFHiPrec64BitTime & time, const STFHiPrec64BitDuration & dueCycleDuration, STFMessageSink * sink, const STFMessage & message, uint32 & timer)
	{
	uint32 i;
	STFResult err = STFRES_OK;

	mutex.Enter();

	i = 0;
	while (i < STFTIMER_MAX_EVENTS && pendingEvents[i].sink)
		i++;

	if (i < STFTIMER_MAX_EVENTS)
		{
		pendingEvents[i].sink = sink;
		pendingEvents[i].message = message;
		pendingEvents[i].time = time;
		pendingEvents[i].id = timer = i + count;
		pendingEvents[i].recurrent = true;
		pendingEvents[i].dueCycleDuration = dueCycleDuration;
		count += STFTIMER_MAX_EVENTS;

		signal.SetSignal();
		}
	else
		err = STFRES_OBJECT_FULL;

	mutex.Leave();

	STFRES_RAISE(err);
	}


STFResult STFTimer::CancelEvent(uint32 timer)
	{
	mutex.Enter();

	if (pendingEvents[timer & (STFTIMER_MAX_EVENTS-1)].id == timer)
		{
		pendingEvents[timer & (STFTIMER_MAX_EVENTS-1)].sink = NULL;
		}

	mutex.Leave();

	STFRES_RAISE_OK;
	}						

STFResult STFTimer::CancelEvent(STFMessageSink * sink)
	{
	uint32 i;
	if (sink == NULL)
		STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);

	mutex.Enter();

	for (i=0; i < STFTIMER_MAX_EVENTS; i++)
		{
		if (pendingEvents[i].sink == sink)
			{
			//cancel all events scheduled for this message sink
			pendingEvents[i].sink = NULL;
			}
		}

	mutex.Leave();

	STFRES_RAISE_OK;
	}		

STFResult InitializeSTFTimer(void)
	{
	STFRES_REASSERT(InitializeOSSTFTimer());

	SystemTimer = new STFTimer();
	if (SystemTimer == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}
