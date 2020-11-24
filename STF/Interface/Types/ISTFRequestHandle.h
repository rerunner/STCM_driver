//
// PURPOSE:   STF request handle interface.
//

/*! \file
	\brief Request handles are used for asynchronous server functions. A client passes a request handle
			 to a server function, which memorizes the request and the handle and returns immediately.
			 When the server has finished the request after asynchronously processing it, the server
			 sends a message to the client, with the request handle as the second message parameter. The
			 first parameter is usually an STFResult.

			 To the asynchronous server function, the handle is only a numerical value and has no other
			 meaning than distinguishing the separate client calls to one and the same server function.

			 The client must take care to use unique request handles. It is free to create a sub-class
			 of ISTFRequestHandle to send to the server function and provide it with info that it wishes
			 to be passed back when it receives the message later.
*/

#ifndef ISTF_REQUESTHANDLE_H
#define ISTF_REQUESTHANDLE_H

#include "STF/Interface/Types/STFBasicTypes.h"



//! ISTFRequestHandle interface.
class ISTFRequestHandle
	{
	//! There's nothing in here as the interface exists only as a name anchor.
	};



//! STFRequestHandleValue.
/*! This is a convenience class for clients that want to pass a numerical value to the server
	 instead of a pointer. With this class, a client does not need to declare a sub-class.
*/
class STFRequestHandleValue : public ISTFRequestHandle
	{
	public:
		uint32	value;
	};



#endif
