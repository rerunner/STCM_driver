///
/// @brief      Memory startup for Linux PC User Mode
///

#include "STF/Interface/Types/STFResult.h"

STFResult InitializeKernel(void)
	{
	// Nothing needs to be done for Linux User Mode at this point so just return OK
	return STFRES_OK;
	}

