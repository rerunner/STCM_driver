///
/// @brief This wrapper connects the STFProfile with the data that belongs to any of the profile data categories.
///

#ifndef PROFILEWRAPPER_H
#define PROFILEWRAPPER_H

#include "STF/Interface/STFProfile.h"
#include "STF/Interface/STFMutex.h"
//#include "VDR/Interface/Unit/Memory/IVDRNonVolatileMemory.h"
#include "VDR/Interface/Unit/IVDRUnitSetFactory.h"

class ProfileWrapper : public STFProfile
	{
	protected:
		IVDRUnitSet		*	unitSet;
		//IVDRNonVolatileMemory   *	nvMem;
		STFMutex			mutex;
		IVDRBase		*	driver;
		VDRUID				dataCategory;

	public:
		ProfileWrapper (IVDRBase * driver);
		~ProfileWrapper (void);

	public:
		STFResult Initialize (VDRUID dataCategory);

	public:
		STFResult Commit (void);
	};

#endif //PROFILEWRAPPER_H
