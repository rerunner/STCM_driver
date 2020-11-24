// Includes
#ifndef MEMORYSTARTUP_H
#define MEMORYSTARTUP_H

#include "STF/Interface/Types/STFResult.h"

// Initialize memory management
// This function searches the global board config for the special unit ID of the HeapMemory
// and then calls the necessary OS21 function to initialize the heap and sets the global
// variable system_partition which the overloaded new and delete operators use
STFResult InitializeMemory(void);
 
#endif // MEMORYSTARTUP_H
