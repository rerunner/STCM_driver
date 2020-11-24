// Includes
#ifndef KERNELSTARTUP_H
#define KERNELSTARTUP_H

#include "STF/Interface/Types/STFResult.h"

// Initialize kernel
// This function is the first one called on system startup. Whatever initialization the kernel needs is done here
STFResult InitializeKernel(void);
 
#endif // KERNELSTARTUP_H
