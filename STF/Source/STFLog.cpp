///
/// @brief 
///

#include "STF/Interface/STFLog.h"
#include <stdio.h>

STFRootLogger RootLogger;


//###########################################
//           STFLogEvent
//###########################################

STFLogEvent::STFLogEvent()
	{
	ref        = 0; 
	repository = NULL;
	
	//timeStamp();
	threadName = "";
	category   = "";
	level      = STFLL_DEBUG;
	//custom1    = 0;
	}

STFLogEvent::~STFLogEvent()
	{
	}

STFLogType STFLogEvent::GetType()
	{
	return type;
	}

void STFLogEvent::AddRef()
	{
	ref++;
	}

void STFLogEvent::Release()
	{
	if (ref <= 1)
		{
		ref = 0;
		repository->ReturnLogEvent(this);
		//erase data
		}
	else
		ref--;
	}

uint32 STFLogEvent::GetRefCnt()
	{
	return ref;
	}


//###########################################
//           STFStringLogEvent
//###########################################

STFStringLogEvent::STFStringLogEvent(STFLogEventRepository * rep)
	{
	ref = 0;
	repository = rep;
	type = STFLT_STRING;
	//timeStamp();
	threadName = "";
	category   = "";
	level      = STFLL_DEBUG;
//	custom1    = 0;
	}

STFStringLogEvent::STFStringLogEvent()
	{
	ref = 0;
	repository = NULL;
	type = STFLT_STRING;
	threadName = "";
	category   = "";
	level      = STFLL_DEBUG;
	}

//###########################################
//           STFLogger
//###########################################
STFLogger::STFLogger(STFLogger * par) : appenders(1)
	{
	SetParent(par);
	currentLevel = STFLL_NA;
	}

STFLogger::STFLogger(STFLogger * par,STFString n) : appenders(1), name(n)
	{
	SetParent(par);
	currentLevel = STFLL_NA;
	}

STFLogger::~STFLogger()
	{
	//delete all appenders and children? and remove from parent's child-list
	}

bool STFLogger::IsDebugEnabled(STFLogLevel level)
	{
	return currentLevel >= level;
	}

void STFLogger::Log(STFLogEvent * content)
	{
	if (IsDebugEnabled(content->level))
		{
		uint32 appenderCount;
		uint32 i;
		STFLogAppender * currentAppender;
		pointer p;
		i = 0;
		appenderCount = appenders.Size();
		for (; i < appenderCount; i++)
			{
			appenders.ElementAt(i,p);
			currentAppender = (STFLogAppender*)p;
			currentAppender->Append(content);
			}
		if (!IsRoot())
			{
			((STFLogger*)GetParent())->Log(content);
			}
		}// if(IsDebugEnabled)
		//### cleanup and consuming the event missing
	}

void STFLogger::Log(STFLogLevel level, STFString content)
	{
	if (IsDebugEnabled(level))
		{
		STFStringLogEvent * event;
		event = new STFStringLogEvent();
		event->level = level;
		event->data = content;
		Log(event);
		}
	}

void STFLogger::AddAppender(STFLogAppender * app)
	{
	appenders.Add(app);
	}

STFLogger * STFLogger::GetSubLoggerByPath(STFString path)
	{
	STFString firstChild;
	int32 index;
	index = path.First('.');
	if (index == -1)
		{
		return GetSubLoggerByName(path);
		}
	else
		{
		STFLogger * subLogger;
		subLogger = GetSubLoggerByName(path.Head(index -1));
		return subLogger->GetSubLoggerByPath(path.Tail(path.Length() - index));	
		}
	}

STFLogger * STFLogger::GetSubLoggerByName(STFString name)
	{
	STFLogger * currentLogger;
	uint32 index = 0;
	for(;index < GetChildCount() ; index++)
		{
		currentLogger = (STFLogger*)GetChild(index);
		if (currentLogger->GetName() == name)
			{
			return currentLogger;
			}
		}
	// no Logger found
	currentLogger = new STFLogger(this,name);
	AddChild(currentLogger);
	return currentLogger;
	//###
	}

//###########################################
//           STFRootLogger
//###########################################

STFRootLogger::STFRootLogger()  : STFLogger(NULL) 
	{
	name = "root";
	}

STFRootLogger::~STFRootLogger()
	{
	}

//###########################################
//           STFLogEventRepository
//###########################################

STFLogEventRepository::STFLogEventRepository()
	{
	stringEventsSize = 64;
	freeStringEvents = new STFStringLogEvent[stringEventsSize];
	stringEventsReadIndex = 0;
	stringEventsWriteIndex = 0;
	stringEventsBM = 0x0000003F;
	}

STFLogEventRepository::~STFLogEventRepository()
	{
	//### delete free StringEvents!
	//## for ...
	delete[] freeStringEvents;
	}

STFResult STFLogEventRepository::GetLogEvent(STFStringLogEvent * event)
	{
	STFAutoMutex autoMutex(&mutex);
	if (stringEventsWriteIndex - stringEventsReadIndex > 0)
		{
		event = &freeStringEvents[stringEventsReadIndex++];
		PrepEvent(event);
		}
	else
		{
		event = new STFStringLogEvent(this);
		PrepEvent(event);
		}
	STFRES_RAISE_OK;
	}

void STFLogEventRepository::PrepEvent(STFStringLogEvent * event)
	{
	event->ref = 1;
	event->data = "";
	//#
	}

void STFLogEventRepository::ReturnLogEvent(STFLogEvent * msg)
	{
	
	}

//###########################################
//           STFLogConsoleAppender
//###########################################

void STFLogConsoleAppender::Append(STFLogEvent * e)
	{
	STFStringLogEvent * strEvent;
	const char * buffer;

	switch(e->GetType())
		{
		case STFLT_STRING:
			strEvent = (STFStringLogEvent*)e;
			//printf("im appender\n");
			buffer = (const char *)strEvent->data;
			printf(buffer);
			printf("\n");
			break;

		default:
			printf("no debug output defined\n");
		}
	//###
	}

