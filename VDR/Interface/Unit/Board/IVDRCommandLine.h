#ifndef IVDRCOMMANDLINE_H
#define IVDRCOMMANDLINE_H

#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFSTring.h"

class IVDRCommandLine : public virtual IVDRBase
	{
	public:
		STFResult GetCommandLineParameter(uint32 num, STFString & param) = 0;
		STFResult uint32 GetNumParameters() = 0;
	};
 
#endif // IVDRCOMMANDLINE_H
