///
/// @file       VDR/Source/Memory/IMemOverlapDetector.h
///
/// @brief      VDR memory overlap detector interface.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR internal header file
///
/// @date       2003-01-17
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#ifndef I_MEMOVERLAPDETECTOR_H
#define I_MEMOVERLAPDETECTOR_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "VDR/Interface/Base/IVDRBase.h"



////////////////////////////////////////////////////////////////////
//! Memory Overlap Detector interface.
////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_MEMORYOVERLAPDETECTOR = 0x80000015;   // created by the ID value manager

class IMemoryOverlapDetector : virtual public IVDRBase
	{
	public:
		//! Register a memory range and return an error if it overlaps with previously registered ranges.
		/*!
			\param startAddress IN: The start address.
			\param size IN: The size of the range in bytes.

			\return Standard Error

			If there is an overlap, STFRES_RANGE_VIOLATION is returned.
		*/
		virtual STFResult RegisterRange (PADDR startAddress, uint32 size) = 0;
	};



#endif
