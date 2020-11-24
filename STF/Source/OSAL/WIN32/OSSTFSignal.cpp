///
/// @brief      Win32 specific implementations for STFSynchronistation  
///

#include "OSSTFSemaphore.h"
#include "OSSTFMutex.h"

//Globals

OSSTFMutex *STFGlobalLockMutex = new OSSTFMutex;

STFResult OSSTFGlobalLock(void)
   {
   STFRES_RAISE( STFGlobalLockMutex->Enter());
   }

STFResult OSSTFGlobalUnlock(void)
   {
   STFRES_RAISE( STFGlobalLockMutex->Leave());
   }
