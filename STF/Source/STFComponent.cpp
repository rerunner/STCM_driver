//
// PURPOSE:		Standard Implementation of the base interface
//

#include "STF/Interface/STFComponent.h"

STFBase::STFBase(void) : refCount(1)
	{
	}

STFBase::~STFBase(void)
	{
	}

//! Standard implementation of QueryInterface
STFResult STFBase::QueryInterface(STFIID iid, void *& ifp)
	{
	STFQI_BEGIN
		STFQI_IMPLEMENT(STFIID_STF_BASE, ISTFBase);
	STFQI_END_BASE;

	STFRES_RAISE_OK;
	}

//! Standard implementation of AddRef
uint32 STFBase::AddRef()
	{
	return ++refCount;
	}

//! Standard implementation of Release
uint32 STFBase::Release()
	{
	uint32 res = --refCount;

	if (!res)
		delete this;

	return res;
	}
