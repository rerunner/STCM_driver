///
/// @brief A factory that creates and manages profiles for each available profile data category.
///

#include "VDR/Support/Profile/ProfileFactory.h"

//ProfileFactory::CategoryProfile ProfileFactory::profileList[] = { {VDRUID_NVMEM_READ_ONLY_DRIVER_PROFILE, NULL} };

STFResult ProfileFactory::CreateProfile (IVDRBase * driver, VDRUID dataCategory, ProfileWrapper *& profile)
	{
	uint32	catNum	= 0;
	bool		catFound	= false;
	//Go through profileList and see if profile for this category already exists
	for (uint32 i = 0; i < numOfCats; i++)
		{
		if (profileList[i].category == dataCategory)
			{
			catFound = true;
			catNum = i;
			}
		}
	if (!catFound) //The given dataCategory does not exist.
		STFRES_RAISE(STFRES_OBJECT_INVALID);

	if (profileList[catNum].proPointer == 0) //New profile must be created
		{
		profile = new ProfileWrapper(driver);
		STFRES_REASSERT (profile->Initialize(dataCategory));

		//Entre created profile into profileList.
		profileList[catNum].proPointer = profile;
		}
	else //existing profile is used
		{
		profile = profileList[catNum].proPointer;
		}

	STFRES_RAISE_OK;
	}
