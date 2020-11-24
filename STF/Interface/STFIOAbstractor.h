///
/// @brief  A generic class which provides basic console I/O.
///

#ifndef STFIOABSTRACTOR_H
#define STFIOABSTRACTOR_H

#include <stdarg.h>
#include <stdio.h>
#include "STF/Interface/Types/STFString.h"
#include "STF/Interface/STFSynchronisation.h"

#include "OSSTFIOAbstractor.h"



 
class STFIOSynchronizer
	{
	private:
		STFMutex	   ioLock;

	public:
      STFIOSynchronizer() { }
		void Aquire(void) { ioLock.Enter(); }
		void Release(void) { ioLock.Leave(); }
	};


class STFIOAbstractor
	{
   friend class OSSTFIOAbstractor;
	
   protected:
		OSSTFIOAbstractor osioabs;
		STFIOSynchronizer	*synchronizer;
	
	public:
		STFIOAbstractor(STFIOSynchronizer *synchronizer = NULL)
			{
			this->synchronizer = synchronizer;
			}

		void Printf(const char* format, ...);
		STFString GetEnvironmentVariable(const char *var, const char* defaultString = NULL);
	};

//--------------------------------------------- INLINES ----------------------------------------------

inline void STFIOAbstractor::Printf(const char* format, ...)
   {
   va_list list;
   va_start(list, format);

   osioabs.Printf(format, list);
   }

inline STFString STFIOAbstractor::GetEnvironmentVariable(const char *var, const char* defaultString)
   {
   return osioabs.GetEnvironmentVariable(var, defaultString);
   }


#endif // of STFIOABSTRACTOR_H
