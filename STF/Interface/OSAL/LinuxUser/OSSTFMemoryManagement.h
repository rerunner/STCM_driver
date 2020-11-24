///
/// @brief      Linux specific memory management
///

#include <stdlib.h>
#include <malloc.h>

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/STFMemoryManagement.h"

#include <stdlib.h>
#include <malloc.h>

#define __cdecl

// set for 16 Byte alignment on ix86 machines with mmx memory
//
// Windows 32 allocations, no mmx alignment
//

//static inline void * __cdecl operator new(unsigned int nSize)
//	{
//	return malloc(nSize);
//	}

inline void * __cdecl operator new(unsigned int nSize, POOL_TYPE iType)
	{
        return ::operator new(nSize);
	}


inline void __cdecl operator delete(void* p, POOL_TYPE iType)
	{ 
	if (p)
		{
                ::operator delete(p);
		}
	}


//static inline void __cdecl operator delete(void* p)
//	{ 
//	if (p)
//		{
//		free(p);
//		}
//	}

//static inline void * __cdecl operator new[](unsigned int nSize)
//	{
//	return malloc(nSize);
//	}

inline void * __cdecl operator new[](unsigned int nSize, POOL_TYPE iType)
	{
	return ::operator new[](nSize);
	}

inline void __cdecl operator delete[](void* p, POOL_TYPE iType)
	{ 
	if (p)
		{
                ::operator delete[](p);
		}
	}


//static inline void __cdecl operator delete[](void* p)
//	{ 
//	if (p)
//		{
//		free(p);
//		}
//	}

