// Includes
#ifndef CACHETARTUP_H
#define CACHETARTUP_H

#include "STF/Interface/Types/STFResult.h"

// Temporary define for InitializeCache();
#define		CFG_CACHE_NOCACHES		0
#define		CFG_CACHE_ALLCACHES		1
#define		CFG_CACHE_DCACHE_ONLY	2

STFResult InitializeCache(int cacheControl);
 
#endif // CACHETARTUP_H
