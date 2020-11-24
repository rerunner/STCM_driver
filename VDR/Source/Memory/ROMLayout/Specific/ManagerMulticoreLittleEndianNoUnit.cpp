///
/// @file       VDR/Source/Memory/ROMLayout/Specific/ManagerMulticoreLittleEndianNoUnit.cpp
///
/// @brief      A ROM manager for the multicore little endian ROM layout, not implemented as a unit but as a singleton object.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal implementation file
///
/// @date       2004-06-28
///
/// &copy; 2004 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Memory/ROMLayout/Specific/ManagerMulticoreLittleEndianNoUnit.h"
#include "STF/Interface/STFDataManipulationMacros.h"

#include "STF/Interface/STFDebug.h"
#include <string.h>



/// There is only a single instance of this ROM manager system-wide.

IROMManagerNoUnit	*	ROMManagerNoUnit = NULL;



////////////////////////////////////////////////////////////////////
/// ManagerMulticoreLittleEndianNoUnit implementation.
////////////////////////////////////////////////////////////////////

ManagerMulticoreLittleEndianNoUnit::ManagerMulticoreLittleEndianNoUnit (PADDR romLayoutHeader)
	{
	this->romLayoutHeader = (uint8 *)romLayoutHeader;
	}

ManagerMulticoreLittleEndianNoUnit::~ManagerMulticoreLittleEndianNoUnit (void)
	{
	}



////////////////////////////////////////////////////////////////////
// ManagerMulticoreLittleEndianNoUnit IROMManagerNoUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult ManagerMulticoreLittleEndianNoUnit::FindROMPartition (VDRUID unit, void *& address, uint32 & size)
	{
	char searchName[14];
	uint8 *p;
	uint32 i, dataPtr;
	
	// Start at the image control directory.
	p = (uint8 *)romLayoutHeader + 0x800;

	sprintf (searchName, "ROMP%08x", unit);
	i = 0;
	do {
		dataPtr = MAKELONG4(p[0], p[1], p[2], p[3]);

		//if (dataPtr != NULL)
		if (dataPtr != 0)  // Fixed Nico
			{
			if (0 == strncmp ((const char *)dataPtr, searchName, 12))
				{
				// Found the ROM partition. Get the address and size.
				p = (uint8 *)dataPtr;
				i = MAKELONG4(p[0x2c], p[0x2d], p[0x2e], p[0x2f]);
				address = (void *)MAKELONG4(p[0x34], p[0x35], p[0x36], p[0x37]);
				if (i == 0  ||  address != (void *)MAKELONG4(p[0x38], p[0x39], p[0x3a], p[0x3b]))
					STFRES_RAISE(STFRES_OBJECT_INVALID);   // no section at all or different source and destination

				// Success.
				size = MAKELONG4(p[0x3c], p[0x3d], p[0x3e], p[0x3f]);
				STFRES_RAISE_OK;
				}
			}

		p += 4;
		i++;
		} while (i < 64);

	STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}
