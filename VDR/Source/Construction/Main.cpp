#include "VDR/Source/Construction/UnitConstruction.h"
#include "VDR/Source/Startup/IStartup.h"
#include "STF/Interface/STFThread.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Startup/MemoryStartup.h"
#include "VDR/Source/Startup/KernelStartup.h"

extern "C"
{
#include <stdio.h>
#include <unistd.h>
#include "SDL2/SDL.h"
}
//#include <directfb.h>

IVDRBase * Board;

int main (int argc, char ** argv)
	{		
	STFResult res;
	int	exitCode = 0;

	InitializeDebugRecording ();

	// System specific call to perform whatever initialization the Kernel needs
	InitializeKernel();

	// System specific implementation of the next function can be found in MemoryStartup.cpp
	// in VDR/Source/Construction/Specific/XXXXX
	InitializeMemory();

	InitializeSTFDebugLog ();

	InitializeSTFThreads();
	InitializeSTFTimer();

	// SDL2 Init, this needs a better place...
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
		{
		DP("SDL2 global init failed.\n");
		}
	else
		{
		DP("SDL2 global init OK.\n");
		}

	PreBoardConstructionCallback();

	res = VDRCreateBoard(GlobalBoardConfig, Board);

	// Create Driver
	if (!STFRES_IS_ERROR(res))
		{
		// Call Customer's main to continue system startup
		exitCode = ContinueStartupCallback(Board, argc, argv);

		// Release Board interface here, to free up resources
		Board->Release();
		}
	else
		{
		BoardCreationFailureCallback(res);

		// Do something ...
		exitCode = (int)STFRES_OPERATION_FAILED;
		}

	PreExitCallback(exitCode);

	return exitCode;
	}


STFResult VDRCreateDriverInstance(char * name, uint32 hwInstanceID, IVDRBase* & driver)
	{
	Board->AddRef();
	driver = static_cast<IVDRBase*>(Board);
	STFRES_RAISE_OK;
	}
