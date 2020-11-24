//
// PURPOSE:   Function prototypes of System Startup Callbacks
//

#ifndef ISTARTUP_H
#define ISTARTUP_H

#include "STF/Interface/Types/STFResult.h"
#include "VDR/Interface/Base/IVDRBase.h"

//! Called before Unit Construction Process is started
STFResult PreBoardConstructionCallback(void);

//! Called after the Board was successfully created and initialized
/*! This is the starting point for all applications running on top
    of the VDR driver
*/
STFResult ContinueStartupCallback(IVDRBase * boardInterface, int argc, char**argv);

//! Called if there was an error during the Unit Construction Process
/*! Being called back by this function enables Customer code to
    do specific recovery steps or debug outputs if the board could not
	 be initialized.
*/
void BoardCreationFailureCallback(STFResult res);

//! Called back shortly before shutting down the VDR driver
/*  This happens when the last reference to the VDR driver component
    has been released.
*/
void PreExitCallback(int & exitCode);

#endif	// ISTARTUP_H
