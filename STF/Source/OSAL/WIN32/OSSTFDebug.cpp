//
// PURPOSE:   

#include "STF/Interface/Types/STFBasicTypes.h"
#include "OSSTFDebug.h"
#include "STF/Interface/STFThread.h"
#include "STF/Interface/STFTimer.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_LINE_LENGTH 512

// Set this to 1 if you want to prevent all debug output. This switch is only a temporary
// one, and will be removed as soon as release builds can be properly done for the 8k.
#define DISABLE_ALL_DEBUG_OUT	0


//this lets you print to the console screen, so you dont need to read debug from visual studio
HANDLE debugHandle;
DWORD bytesWritten;

void consoleDebug( const char  * szFormat, ... )
{
	char		buff[MAX_LINE_LENGTH];
	va_list	list;

	va_start(list, szFormat);

  	vsprintf(buff,szFormat,list);

	WriteFile( debugHandle, buff, strlen(buff), &bytesWritten, NULL );
}



// Debug recording enables to record near-real-time debug information into an array.
// The DPR() macro can be used like a printf() call with up to five formatted parameters
// and records the debug output. To dump the information into a file on a PC, call
// WriteDebugRecording(filename).
// Please note the following three switches.
#define DISABLE_RECORDING		0		// if one, no recording takes place
#define USE_WRAPAROUND			1		// if zero, recording stops when array is full
#define SIMPLE_WRAPAROUND		0		// if enabled, array gets flushed at wraparound

void DebugPrint(const char  * szFormat, ...)
	{
	char		buff[MAX_LINE_LENGTH];
	va_list	list;

	va_start(list, szFormat);

  	vsprintf(buff,szFormat,list);
  	::OutputDebugString(buff);
	}

void DebugPrintWithThreadName(const char  * szFormat, ...)
	{
	char			buff[MAX_LINE_LENGTH];
	char			buffWithThreadName[MAX_LINE_LENGTH + 100];
	va_list		list;
	STFThread*	currentThread = NULL;
	STFString	threadName("<unknown>");

	// Retrieve current STF thread
	GetCurrentSTFThread(currentThread);
	if (currentThread)
		threadName = currentThread->GetName();

	va_start(list, szFormat);

  	vsprintf(buff, szFormat, list);
	sprintf(buffWithThreadName, "[%s] %s", (const char*) threadName, buff);
  	::OutputDebugString(buffWithThreadName);
	}


void  MDebugPrint(const char * szFormat, ...)
	{
	char buff[MAX_LINE_LENGTH];

  	::wvsprintf(buff,szFormat,(char  *)(&szFormat+1));
  	::OutputDebugString(buff);
	}

#define ARRAY_SIZE  500000

uint8 VDRecordArray[ARRAY_SIZE];

uint8 *RecordPtr;
uint8 *RecordEndPtr;
uint8 *RecordWrapEndPtr;
bool RecordingWrappedAround;
CRITICAL_SECTION	RecordLock;

#if _DEBUG // In release builds this function has a body in the header file

void InitializeDebugRecording (void)
	{

debugHandle= GetStdHandle( STD_OUTPUT_HANDLE );


#if !DISABLE_ALL_DEBUG_OUT
	RecordPtr = VDRecordArray;
	RecordEndPtr = VDRecordArray + ARRAY_SIZE - MAX_LINE_LENGTH;
	RecordingWrappedAround = false;
	::InitializeCriticalSection(&RecordLock);
#endif // #if !DISABLE_ALL_DEBUG_OUT
	}

#endif // _DEBUG

void DebugPrintRecord (const char * szFormat, ...)
	{
#if !DISABLE_ALL_DEBUG_OUT
	char string[MAX_LINE_LENGTH];
	va_list marker;
	int32 i;

#if DISABLE_RECORDING
	return;
#endif

	va_start (marker, szFormat);

	vsprintf (string, szFormat, marker);

	i = strlen(string);
	::EnterCriticalSection(&RecordLock);

	if (RecordPtr < RecordEndPtr)
		{
		uint8 *dst = RecordPtr;
		RecordPtr += i;

		uint8 *src = (uint8*)string;
		uint8 ch = *src++;
		while (ch != 0)
			{
			*dst++ = ch;
			ch = *src++;
			}
		}
	else
		{
#if USE_WRAPAROUND
#if !SIMPLE_WRAPAROUND
		RecordingWrappedAround = true;
		RecordWrapEndPtr = RecordPtr;
#endif
		RecordPtr = VDRecordArray;
		uint8 *dst = RecordPtr;
		RecordPtr += i;

		uint8 *src = (uint8*)string;
		uint8 ch = *src++;
		while (ch != 0)
			{
			*dst++ = ch;
			ch = *src++;
			}
#endif
		}

	::LeaveCriticalSection(&RecordLock);

	va_end (marker);
#endif // #if !DISABLE_ALL_DEBUG_OUT
	}

#if _DEBUG // In release builds this function has a body in the header file

void WriteDebugRecording (char *filename)
	{
#if !DISABLE_ALL_DEBUG_OUT
	uint8 *p1, *p2;
	int32 size1, size2;

	::EnterCriticalSection(&RecordLock);

	if (RecordingWrappedAround)
		{
		p1 = RecordPtr;
		size1 = RecordWrapEndPtr - p1;
		p2 = VDRecordArray;
		size2 = RecordPtr - p2;
		}
	else
		{
		p1 = VDRecordArray;
		size1 = RecordPtr - p1;
		size2 = 0;
		}

	DPF ("WriteDebugRecording(\"%s\") size %d\n", filename, size1 + size2);
	if (size1 + size2 > 0)
		{
		FILE * file = fopen(filename, "w");
		if (file != NULL)
			{
			DPF (" writing %d bytes\n", size1 + size2);
			if (size1 > 0)
				fwrite (p1, 1, size1, file);
			if (size2 > 0)
				fwrite (p2, 1, size2, file);
			fclose (file);
			DPF (" writing done\n");
			}
		}

	::LeaveCriticalSection(&RecordLock);

#endif // #if !DISABLE_ALL_DEBUG_OUT
	}



#endif // _DEBUG
