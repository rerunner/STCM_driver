#include <windows.h>
#include <stdio.h>
#include "tchar.h"

//a handle to connect to the output console
extern HANDLE debugHandle;

#if _DEBUG
	//WIN32
	#define DPF DebugPrint

	void consoleDebug( const char  * szFormat, ... );
#endif

//
//  Define breakpoint
//

#ifdef _DEBUG

//WIN32
#define BREAKPOINT		do {__asm int 3} while (0)

#endif // _DEBUG



