///
/// @brief OS-dependent debug-methods 
///

#ifndef OSSTFDEBUG_H
#define OSSTFDEBUG_H


#include <stdio.h>
#include <assert.h>

#if _DEBUG
		#define DPF printf

#endif

//
//  Define breakpoint
//

#ifdef _DEBUG

#define BREAKPOINT		assert(false)

#endif // _DEBUG

#endif
