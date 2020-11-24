///
/// @file       VDR/Source/Memory/MemOverlapDetector.h
///
/// @brief      VDR memory overlap detector.
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

#ifndef MEMOVERLAPDETECTOR_H
#define MEMOVERLAPDETECTOR_H

#include "VDR/Source/Unit/PhysicalUnit.h"
#include "VDR/Source/Memory/IMemOverlapDetector.h"
#include "VDR/Source/Construction/IUnitConstruction.h"



////////////////////////////////////////////////////////////////////
//! Memory Overlap Detector implementation.
////////////////////////////////////////////////////////////////////

class MemoryOverlapDetector : virtual public IMemoryOverlapDetector,
										public SharedPhysicalUnit
	{
	protected:
		STFMutex		registerMutex;		//< protects the internal variables

		struct Range
			{
			PADDR		start;
			uint32	size;
			};
		Range		*ranges;
		int		maxRanges;		//< array size
		int		numRanges;		//< valid entries in array

	public:
		// Constructor and destructor.
		MemoryOverlapDetector (VDRUID unitID);
		virtual ~MemoryOverlapDetector (void);

	public:
		// IMemoryOverlapDetector implementation.
		virtual STFResult RegisterRange (PADDR startAddress, uint32 size);

	public:
		// Partial IVDRBase implementation.
		virtual STFResult QueryInterface (VDRIID iid, void * & ifp);

	public:
		// Partial IPhysicalUnit implementation.

		virtual STFResult CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};


#endif
