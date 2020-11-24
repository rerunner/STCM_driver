//
// VDR/Source/Construction/InitTerm.h
//

#ifndef INITTERM_H
#define INITTERM_H


#include "VDR/Interface/Base/IVDRBase.h"
#include "STF/Interface/Types/STFResult.h"


// struct STCMEncoderDriver_Capability		// As this is to provide audio and video characteristics this should be moved nearer from DDPQualityMode.h
// 	{
// 	;
// 	};


struct STCMEncoderDriver_InitParams
	{
	bool initializeKernel;
	};

struct STCMEncoderDriver_TermParams
	{
	STFResult initStatus;
	STFResult startupStatus;
	};


STFResult STCMEncoderDriver_Init(IVDRBase *& Board, const STCMEncoderDriver_InitParams & param);
STFResult STCMEncoderDriver_Term(IVDRBase * Board, const STCMEncoderDriver_TermParams & param);
STFResult VDRCreateDriverInstance(IVDRBase * Board, char * name, uint32 hwInstanceID, IVDRBase* & driver);


#endif
