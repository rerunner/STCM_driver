//
// PURPOSE:		VDR Layer Main API
//

#ifndef IVDRLAYER_H
#define IVDRLAYER_H

#include "IVDRBase.h"


//! Create VDR Driver Instance
/*! 
	Create an instance of the "VDR Driver" Component and gain access to its Base Interface.
	
	\param name IN: identifier of driver instance

	\param hwInstanceID IN: which physical HW instance to create the driver for
									(if more than one of the same HW (e.g. PCI board) exists)

	\param driver OUT: pointer to the IVDRBase interface of the driver instance created.
							 NULL if the driver instance cannot be created

	\return Standard Error

	A driver instance is shut down when the reference counts of all interfaces that 
	have been queried from it become 0.

*/

STFResult VDRCreateDriverInstance(char * name, uint32 hwInstanceID, IVDRBase* & driver);

#endif // #ifndef IVDRLAYER_H
