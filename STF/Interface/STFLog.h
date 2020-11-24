///
/// @brief 
///

#ifndef STFLOG_H
#define STFLOG_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFTime.h"
#include "STF/Interface/Types/STFTree.h"
#include "STF/Interface/STFSynchronisation.h"

#define STFLOG_DEBUG(logger,msg) do { if(logger->IsDebugEnabled(STFLL_DEBUG)) { logger->LogDebug(msg); }}while(0)


class STFRootLogger;
class STFLogEventRepository;
class STFLogger;
class STFLogAppender;

extern STFRootLogger RootLogger;

///#################
/// defines the severity-level.
/// debug is for the debug-messages, these messages should be removed in production-code
/// fatal is for the last message.
///
enum STFLogLevel
	{
	STFLL_NA,
	STFLL_DEBUG,
	STFLL_INFO,
	STFLL_NOTICE,
	STFLL_WARN,
	STFLL_FATAL
	};

enum STFLogType
	{
	STFLT_STRING
	};

class STFLogEvent
	{
	friend STFLogEventRepository;
	
	protected:
		STFLogType type;
		uint32  ref;
		STFLogEventRepository * repository;

		//STFLogEvent(STFLogEventRepository * rep,);

		STFLogEvent(void);
		~STFLogEvent(void);
		
	public:
		//STF32BitLowPrecTime timeStamp;
		STFString     threadName;
		STFString     category;
		STFLogLevel   level;
//		uint32        custom1;

		STFLogType GetType();

		void AddRef(void);

		uint32 GetRefCnt(void);

		void Release(void);
	};

//###############################
/// @class STFStringLogEvent
///
class STFStringLogEvent : public STFLogEvent
	{
	public:
		STFString data;
		
		STFStringLogEvent(); //## only temporary
		STFStringLogEvent(STFLogEventRepository * rep); //# set protected
	};

//###############################
/// @class STFLogger
///
class STFLogger : protected STFTreeNode 
	{
	protected:
		STFPointerArray appenders;
		STFString name;
		STFLogLevel currentLevel;


	public:
		STFLogger(STFLogger * parent);
		STFLogger(STFLogger * par, STFString n);
		~STFLogger();

		/// @brief returns true if this level is enabled.
		///
		/// if it is disabled, a msg would not be sent, instead instantly discarded
		///
		bool IsDebugEnabled(STFLogLevel level);

		/// @brief used to log a message, containing a String
		void Log(STFLogLevel level, STFString content);
		//void LogWarn(STFString content){Log(STFLL_WARN,content);}
		void LogDebug(STFString content){Log(STFLL_DEBUG,content);}

		//void LogWarn(const char * szFormat, ...);
		//void Log(STFLogLevel level, const char * szFormat, ...);

		void Log(STFLogEvent * event);

		void AddAppender(STFLogAppender * app);
		//void Log(STFLogLevel level, STFLogType datatype, uint8 * data);

		void SetName(STFString n){ name = n;} 
		STFString GetName() { return name;}

		STFLogger * GetSubLoggerByPath(STFString path);

		STFLogger * GetSubLoggerByName(STFString name);

		void SetLevel(STFLogLevel level) {currentLevel = level;}
	};

//###############################
/// @class STFRootLogger
///
class STFRootLogger : public STFLogger
	{

	public:
		STFRootLogger();
		~STFRootLogger();

		STFLogger GetLogger(STFString name);

	};

//###############################
/// @class STFLogEventRepository
///
class STFLogEventRepository 
	{
	protected:
		STFStringLogEvent * freeStringEvents;
		uint32 stringEventsReadIndex;
		uint32 stringEventsWriteIndex;
		uint32 stringEventsBM;
		uint32 stringEventsSize;
	
		STFMutex mutex;
		
		void PrepEvent(STFStringLogEvent * event);

	public:
		STFLogEventRepository();
		~STFLogEventRepository();

      STFResult GetLogEvent(STFStringLogEvent * event);
		
		void ReturnLogEvent(STFLogEvent * msg);
	};

//###############################
/// @class STFLogAppender
///
class STFLogAppender
	{
	public:
		virtual void Append(STFLogEvent * event) = 0;
	};

//###############################
/// @class STFLogConsoleAppender
///
class STFLogConsoleAppender : public STFLogAppender
	{
	public:
		virtual void Append(STFLogEvent * event);
	};
	
#endif
