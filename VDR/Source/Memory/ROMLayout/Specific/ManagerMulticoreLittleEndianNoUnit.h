///
/// @file       VDR/Source/Memory/ROMLayout/Specific/ManagerMulticoreLittleEndianNoUnit.h
///
/// @brief      A ROM manager for the multicore little endian ROM layout, not implemented as a unit but as a singleton object.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal header file
///
/// @date       2004-06-28
///
/// &copy; 2004 ST Microelectronics. All Rights Reserved.
///

#ifndef MANAGERMULTICORELITTLEENDIANNOUNIT_H
#define MANAGERMULTICORELITTLEENDIANNOUNIT_H

#include "VDR/Source/Memory/ROMLayout/IROMManagerNoUnit.h"



////////////////////////////////////////////////////////////////////
/// WARNING: This class may be used only for special purposes before
/// the unit construction takes place. For all other purposes, the
/// unit implementation of the ROM manager MUST BE USED!
/// Otherwise, flash ROM read access is not synchronized with write
/// accesses, which leads to corrupted data.
////////////////////////////////////////////////////////////////////

class ManagerMulticoreLittleEndianNoUnit : public IROMManagerNoUnit
	{
	protected:
		uint8		*romLayoutHeader;

	public:
		// Constructor and destructor.
		ManagerMulticoreLittleEndianNoUnit (PADDR romLayoutHeader);
		virtual ~ManagerMulticoreLittleEndianNoUnit (void);

	public:
		// IROMManagerNoUnit implementation.
		virtual STFResult FindROMPartition (VDRUID unit, void *& address, uint32 & size);
	};



#endif
