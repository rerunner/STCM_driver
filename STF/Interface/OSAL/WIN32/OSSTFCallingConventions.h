// OS : Win32

#ifndef OSSTFCALLINGCONVENTIONS_INC
#define OSSTFCALLINGCONVENTIONS_INC

//! Library call means that this function can be called from outside libary
#ifndef LIBRARYCALL
#define LIBRARYCALL __declspec(dllexport)
#endif

//! OS specific calling conventions
#ifndef OSSPECIFICCALL
#define OSSPECIFICCALL WINAPI
#endif

//! C Calling conventions
#ifndef CLANGUAGECALL
#define CLANGUAGECALL __cdecl 
#endif

//! Pascal calling conventions
#ifndef PASCALLANGUAGECALL
#define PASCALLANGUAGECALL __pascal
#endif
 
#endif // OSSTFCALLINGCONVENTIONS_INC
