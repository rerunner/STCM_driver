#include "VDR/Source/Construction/UnitConstruction.h"
#include "VDR/Source/Startup/IStartup.h"
#include "STF/Interface/STFThread.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Startup/MemoryStartup.h"
#include "VDR/Source/Startup/KernelStartup.h"

#include "InitTerm.h"


static const char * STCMEncoderDriver_Revision = "STCM80x0-REL_0.0.1";



STFResult STCMEncoderDriver_Init(IVDRBase *& localBoard, const STCMEncoderDriver_InitParams & param)
	{		
	InitializeDebugRecording ();

	// System specific call to perform whatever initialization the Kernel needs
	if (param.initializeKernel)
		InitializeKernel();	// optionnal

	// System specific implementation of the next function can be found in MemoryStartup.cpp
	// in VDR/Source/Construction/Specific/XXXXX
	InitializeMemory();	// Already got in GlobalBoardConfig

	InitializeSTFDebugLog ();

	InitializeSTFThreads();
	InitializeSTFTimer();

	PreBoardConstructionCallback();

	return VDRCreateBoard(GlobalBoardConfig, localBoard);
	}

STFResult STCMEncoderDriver_Term(IVDRBase * localBoard, const STCMEncoderDriver_TermParams & param)
	{
	int exitParam = param.startupStatus;
	if (!STFRES_IS_ERROR(param.initStatus))
		{
		// Release localBoard interface here, to free up resources
		localBoard->Release();
		}
	else
		{
		BoardCreationFailureCallback(param.initStatus);

		// Do something ...
		exitParam = STFRES_OPERATION_FAILED;
		}

	PreExitCallback(exitParam);

	return param.startupStatus;
	}

STFResult VDRCreateDriverInstance(IVDRBase * localBoard, char * name, uint32 hwInstanceID, IVDRBase* & driver)
	{
	localBoard->AddRef();
	driver = static_cast<IVDRBase*>(localBoard);
	STFRES_RAISE_OK;
	}

//
// Few additional calls
//

const char * STCMEncoderDriver_GetRevision(void)
	{
	return STCMEncoderDriver_Revision;
	}

// STFResult STCMEncoderDriver_GetCapability(IVDRBase * localBoard, STCMEncoderDriver_Capability *Capability)
// 	{
// 	}
