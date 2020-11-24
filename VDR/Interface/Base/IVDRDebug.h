#ifndef IVDRDEBUG_H
#define IVDRDEBUG_H

///
/// @brief VDR Debugging Interfaces
///

#include "VDR/Interface/Base/IVDRBase.h"
#include "STF/Interface/STFDebug.h"


static const VDRIID VDRIID_VDR_GENERIC_DEBUG_INFO = 0x0000001f;

/// Interface providing access to generic Debugging Information
class IVDRGenericDebugInfo : virtual public IVDRBase
	{
#if _DEBUG
	public:
		/// Print general information about object exposing the IVDRGenericDebugInfo interface.
		/// The parameter is an ID deciding on where the output is sent to.
		virtual STFResult PrintDebugInfo (uint32 id = LOGID_ERROR_LOGGING) = 0;
#endif
	};


#endif // #ifndef IVDRDEBUG_H
