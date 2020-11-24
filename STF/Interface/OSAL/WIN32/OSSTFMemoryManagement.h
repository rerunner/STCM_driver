//
// OPERATING SYSTEM : Win32 
// 

#include <stdlib.h>
#include <malloc.h>

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/STFMemoryManagement.h"

// set for 16 Byte alignment on ix86 machines with mmx memory


#include <stdlib.h>
#include <malloc.h>


#if MMXMEMORY 

//
// for mmx 16 bytes alignment ....
//

static inline void * __cdecl operator new(unsigned int nSize)
	{
	int * p = (int *)malloc(nSize + 16);
	int t = (int)p & 15;
	p += (16 - t) >> 2;
	p[-1] = t;
	return (void *)p;
	}

static inline void * __cdecl operator new[](unsigned int nSize)
	{
	int * p = (int *)malloc(nSize + 16);
	int t = (int)p & 15;
	p += (16 - t) >> 2;
	p[-1] = t;
	return (void *)p;
	}

static inline void * __cdecl operator new(unsigned int nSize, POOL_TYPE iType)
	{
	int * p = (int *)malloc(nSize + 16);
	int t = (int)p & 15;
	p += (16 - t) >> 2;
	p[-1] = t;
	return (void *)p;
	}

static inline void * __cdecl operator new[](unsigned int nSize, POOL_TYPE iType)
	{
	int * p = (int *)malloc(nSize + 16);
	int t = (int)p & 15;
	p += (16 - t) >> 2;
	p[-1] = t;
	return (void *)p;
	}

static inline void __cdecl operator delete(void* p, POOL_TYPE iType)
	{ 
	int * q;

	if (p)
		{
		q = (int *)p;
		int t;
		t = q[-1];
		q -= (16 - t) >> 2;
		free(q);
		}
	}

static inline void __cdecl operator delete(void* p)
	{ 
	int * q;

	if (p)
		{
		q = (int *)p;
		int t;
		t = q[-1];
		q -= (16 - t) >> 2;
		free(q);
		}
	}

static inline void __cdecl operator delete[](void* p)
	{ 
	int * q;

	if (p)
		{
		q = (int *)p;
		int t;
		t = q[-1];
		q -= (16 - t) >> 2;
		free(q);
		}
	}




#else	// MMXMEMORY



//
// Windows 32 allocations, no mmx alignment
//

static inline void * __cdecl operator new(unsigned int nSize)
	{
	return malloc(nSize);
	}

static inline void * __cdecl operator new(unsigned int nSize, POOL_TYPE iType)
	{
	return malloc(nSize);
	}

static inline void __cdecl operator delete(void* p, POOL_TYPE iType)
	{ 
	if (p)
		{
		free(p);
		}
	}


static inline void __cdecl operator delete(void* p)
	{ 
	if (p)
		{
		free(p);
		}
	}


static inline void * __cdecl operator new[](unsigned int nSize)
	{
	return malloc(nSize);
	}

static inline void * __cdecl operator new[](unsigned int nSize, POOL_TYPE iType)
	{
	return malloc(nSize);
	}

static inline void __cdecl operator delete[](void* p, POOL_TYPE iType)
	{ 
	if (p)
		{
		free(p);
		}
	}


static inline void __cdecl operator delete[](void* p)
	{ 
	if (p)
		{
		free(p);
		}
	}

#endif // MMXMEMORY