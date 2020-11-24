//
// PURPOSE:   VDR implementation for a linear memory pool allocator.
//

#ifndef LINEARMEMORYPOOL_H
#define LINEARMEMORYPOOL_H

#include "VDR/Source/Memory/MemoryPoolAllocator.h"



////////////////////////////////////////////////////////////////////
//! LinearMemoryPool implementation.
////////////////////////////////////////////////////////////////////

class LinearMemoryPool : public MemoryPoolAllocator
	{
	protected:

	public:
		LinearMemoryPool (VDRUID unitID) : MemoryPoolAllocator (unitID) {}

	public:
		// Partial IPhysicalUnit implementation.
		virtual STFResult CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	};



class VirtualLinearMemoryPool : public VirtualMemoryPoolAllocator
	{
	protected:
		STFSemaphore	waitSemaphore;

		STFMutex			getMutex;

		MemoryPoolBlock	*poolBlocks;
		volatile uint32	nextSearchPosition;
		volatile	bool		allocationFailed;

	public:
		VirtualLinearMemoryPool (MemoryPoolAllocator * physicalUnit);
		virtual ~VirtualLinearMemoryPool (void);

	public:
		// IVDRMemoryPoolAllocator implementation.
		virtual STFResult GetMemoryBlocks (VDRMemoryBlock ** blocks, uint32 minWait, uint32 number, uint32 &done, IVDRDataHolder * holder = NULL);
		virtual STFResult ReturnMemoryBlocks (VDRMemoryBlock ** blocks, uint32 number = 1);
		virtual void DumpMemoryBlockLog (void);

	public:
		// Partial IVirtualUnit overload implementation.
		virtual STFResult InternalPassivate (const STFHiPrec64BitTime & systemTime);
		virtual STFResult PreemptUnit (uint32 flags);
	};


#endif
