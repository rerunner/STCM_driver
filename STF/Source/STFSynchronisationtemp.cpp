
// Includes
#include "STF/Interface/STFSynchronisation.h"
#include "OSSTFSynchronisation.h"


///////////////////////////////////////////////////////////////
// STFArmedSignal
///////////////////////////////////////////////////////////////

// Implementation of virtual member functions
STFResult STFArmedSignal::Set(void)
	{
	STFRES_RAISE(oss.SetSignal(1));
	}

STFResult STFArmedSignal::Reset(void)
	{
	STFRES_RAISE(oss.ResetSignal());
	}

STFResult STFArmedSignal::Wait(void)
	{
	STFRES_RAISE(oss.WaitSignal());
	}

STFResult STFArmedSignal::WaitImmediate(void)
	{
	STFRES_RAISE(oss.WaitImmediateSignal());
	}

