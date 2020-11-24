
// Includes
#ifndef STFBASICTYPES_H
#define STFBASICTYPES_H

// Include the OS-specific header file
#include "OSSTFBasicTypes.h"

typedef unsigned char		uint8;
typedef signed char			int8;
typedef unsigned short		uint16;
typedef signed short			int16;

typedef unsigned short		wchar;
typedef void	*				pointer;
typedef bool					bit;

#define HIGH					true
#define LOW						false   

#ifndef NULL
#define	NULL					0
#endif


#endif //STFTYPES_H
