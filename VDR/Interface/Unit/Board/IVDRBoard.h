
#ifndef IVDRBOARD_H
#define IVDRBOARD_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "VDR/Interface/Unit/IVDRTags.h"

//
// THE Global Board Unit ID!
//
static const VDRUID VDRUID_VDR_BOARD = 0x00000001;


//
// Main Tag type (ex. Tag Unit) identifier for general Tags of the driver
//
static const VDRTID VDRTID_BOARD = 0x00018000;

//
// Some Tags for the Board Unit
//

// Board version info (hardware version)
MKTAG(BOARD_VERSION,		VDRTID_BOARD, 0x01,	uint32)
// Board revision info (hardware revision)
MKTAG(BOARD_REVISION,	VDRTID_BOARD, 0x02,	uint32)
// Board type info (hardware type)
MKTAG(BOARD_TYPE,			VDRTID_BOARD, 0x03,	uint32)

// Driver software version
MKTAG(DRIVER_VERSION,	VDRTID_BOARD, 0x10,	uint32)
// Driver software revision
MKTAG(DRIVER_REVISION,	VDRTID_BOARD, 0x11,	uint32)
// Driver build number
MKTAG(DRIVER_BUILD,		VDRTID_BOARD, 0x12,	uint32)




#endif	// #ifndef IVDRBOARD_H

