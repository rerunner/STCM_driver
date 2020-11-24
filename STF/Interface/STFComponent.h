//
// PURPOSE:		Base Component Object Interfaces
//

#ifndef STFCOMPONENT_H
#define STFCOMPONENT_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/STFSynchronisation.h"


//! VDR Interface ID type
typedef uint32 STFIID;


//! Error code returned when an interface requested with ISTFBase::QueryInterface cannot be found
static const STFResult STFRES_INTERFACE_NOT_FOUND = 0x88844005;


//! Base Interface ID
static const STFIID STFIID_STF_BASE = 0x00000006;


//! Base Interface Definition
/*!
	Access to all other interfaces a component object offers is gained
	by this interface.
*/
class ISTFBase
	{
	public:
		//! Ask for the pointer to a specific interface
		/*!
			\param ifID IN: Interface ID
			\param ifp INOUT: reference to ISTFBase pointer

			\result STFRES_OK if interface was found, otherwise STFRES_INTERFACE_NOT_FOUND
		*/
		virtual STFResult QueryInterface(STFIID iid, void *& ifp) = 0;

		//
		// Reference counting
		//
		
		//! Increase reference counter
		virtual uint32 AddRef() = 0;
		
		//! Decrease reference counter, release interface when it becomes 0
		/*!
			If reference counter becomes 0, the interface will close down. If all interfaces
			on a component instance have been closed down, the component instance will be
			released.
		*/
		virtual uint32 Release() = 0;
	};


//! Standard implementation of base interface
class STFBase : virtual public ISTFBase
	{
	protected:
		//! Reference counter to control lifetime of the object exposing the interface
		STFInterlockedInt	refCount;
	public:
		STFBase(void);
		virtual ~STFBase(void);

		virtual STFResult QueryInterface(STFIID iid, void *& ifp);

		virtual uint32 AddRef();
		virtual uint32 Release();
	};

///////////////////////////////////////////////////////////////////////////////
// Macros to simplify the implementation of QueryInterface
///////////////////////////////////////////////////////////////////////////////

//! Macro to begin checking for interfaces
#define STFQI_BEGIN	\
	switch (iid) {

//! Macro to return pointer to a specific interface if it is supported
#define STFQI_IMPLEMENT(qiid, qif)	\
	case qiid: ifp = (qif *)this; AddRef(); break

//! Macro to end checking for interfaces without calling a parent class
#define STFQI_END_BASE	\
	default: ifp = NULL; STFRES_RAISE(STFRES_INTERFACE_NOT_FOUND); }

//! Macro to end checking for interfaces on the current level, and continuing the query in the parent class
#define STFQI_END(super)	\
	default: STFRES_REASSERT(super::QueryInterface(iid, ifp)); }


#endif // STFCOMPONENT_H
