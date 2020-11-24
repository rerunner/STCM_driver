///
/// @brief      Clock tags
///

#ifndef ISYSTEMCLOCKTAGS_H
#define ISYSTEMCLOCKTAGS_H
 					 
#include "STF/Interface/Types/STFBasicTypes.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

static const VDRTID VDRTID_SYSCLOCK = 0x0004e000;

MKTAG(SYSCLOCK_MAIN,      VDRTID_SYSCLOCK,  0x01,  uint32)
MKTAG(SYSCLOCK_SERIAL,    VDRTID_SYSCLOCK,  0x02,  uint32)
MKTAG(SYSCLOCK_I2C,       VDRTID_SYSCLOCK,  0x03,  uint32)
MKTAG(SYSCLOCK_IRBLASTER, VDRTID_SYSCLOCK,  0x04,  uint32)
MKTAG(SYSCLOCK_STBUS,     VDRTID_SYSCLOCK,  0x05,  uint32)
MKTAG(SYSCLOCK_VES,       VDRTID_SYSCLOCK,  0x06,  uint32)

#endif // ISYSTEMCLOCKTAGS_H
