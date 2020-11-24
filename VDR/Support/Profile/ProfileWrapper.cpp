///
/// @brief This wrapper connects the STFProfile with the data that belongs to any of the profile data categories.
///

#include "VDR/Support/Profile/ProfileWrapper.h"

ProfileWrapper::ProfileWrapper (IVDRBase * driver)
	{
	this->driver = driver;
	driver->AddRef();

	this->dataCategory = 0;
	}

ProfileWrapper::~ProfileWrapper (void)
	{
	delete this->dataContainer;

	//if (nvMem)
//		nvMem->Release();
	
	if(unitSet)
		{
		//unitSet->Passivate();
		unitSet->Release();
		}
		
	if (driver)
		driver->Release();
	}

STFResult ProfileWrapper::Initialize (VDRUID dataCategory)
	{
	STFAutoMutex				autoMutex(&mutex);

	IVDRUnitSetFactory	*	unitSetFactory  = NULL;

	STFResult					res;

	this->dataCategory = dataCategory;

	// Ask the driver component for the UnitSet Factory interface
	res = driver->QueryInterface(VDRIID_VDR_UNITSET_FACTORY, (void*&)unitSetFactory);
	
	if(STFRES_SUCCEEDED(res))
		{
		res = unitSetFactory->CreateUnitSet (unitSet, dataCategory, VDR_UNITS_DONE);
		}
	
	if(STFRES_SUCCEEDED(res))
		{
		// Activate UnitSet
		// Do this at realtime priority, and specify the wish for immediate activation (time = 0).
		res = unitSet->ActivateAndLock (VDRUALF_REALTIME_PRIORITY | VDRUALF_TIME_VALID | VDRUALF_WAIT, STFHiPrec64BitTime(0), STFHiPrec32BitDuration(0));
		}

	//if(STFRES_SUCCEEDED(res))
	//	{
	//	res = unitSet->QueryUnitInterface (dataCategory, VDRIID_NON_VOLATILE_MEMORY, (void*&)nvMem);
	//	}

	if(STFRES_SUCCEEDED(res))
      {
      //res = nvMem->GetMemSize(this->size);
		
		if(STFRES_SUCCEEDED(res))
			{
			this->dataContainer = new uint8[this->size];
		//	res = nvMem->InBytes(0, this->dataContainer, this->size);
			}
      }

	unitSet->Unlock();
	unitSet->Passivate();

	if(unitSetFactory)
		unitSetFactory->Release();

	STFRES_RAISE(res);
	}

STFResult ProfileWrapper::Commit (void)
	{
	STFAutoMutex	autoMutex(&mutex);

	STFResult		res = STFRES_OK;

	if (dataCategory == 0)
		{
		DP("ProfileWrapper was not initialized before commit.");
		STFRES_RAISE(STFRES_OPERATION_ABORTED);
		}

	// Activate UnitSet
	// Do this at realtime priority, and specify the wish for immediate activation (time = 0).
	res = unitSet->ActivateAndLock (VDRUALF_REALTIME_PRIORITY | VDRUALF_TIME_VALID | VDRUALF_WAIT, STFHiPrec64BitTime(0), STFHiPrec32BitDuration(0));	

	if(STFRES_SUCCEEDED(res))
		{
		//res = nvMem->OutBytes(0, this->dataContainer, this->size);

		if(STFRES_SUCCEEDED(res))
			{
		//	res = nvMem->Commit();
			}
		}

	unitSet->Unlock();
   unitSet->Passivate();

	STFRES_RAISE(res);
	}
