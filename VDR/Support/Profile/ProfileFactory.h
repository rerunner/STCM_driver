///
/// @brief A factory that creates and manages profiles for each available profile data category.
///

#ifndef PROFILEFACTORY_H
#define PROFILEFACTORY_H

#include "VDR/Support/Profile/ProfileWrapper.h"
#include "STF/Interface/Types/STFResult.h"
//#include "VDR/Interface/Unit/Memory/IVDRNonVolatileMemoryUnits.h"



class ProfileFactory
	{
	protected:
		static const uint32 numOfCats = 1;

		static struct CategoryProfile
			{
			VDRUID				category;
			ProfileWrapper	*	proPointer;
			} profileList[numOfCats];

	public:
		static STFResult CreateProfile (IVDRBase * driver, VDRUID dataCategory, ProfileWrapper *& profile);
	};

#endif //PROFILEFACTORY_H
