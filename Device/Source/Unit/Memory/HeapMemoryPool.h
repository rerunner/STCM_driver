///
///	@brief	HeapMemoryPool physical and virtual unit definition
///

#ifndef HEAPMEMORYPOOL_H
#define HEAPMEMORYPOOL_H


#include "VDR/Source/Memory/MemoryPoolAllocator.h"
#include "STF/Interface/STFHeapMemoryManager.h"
#include "STF/Interface/Types/STFList.h"

////////////////////////////////////////////////////////////////////
// MemoryBlockNode
//
// The node to be used in list of memory blocks. The list is not
// sorted, which is why HigherPriorityThan() function is not
// overridden.
////////////////////////////////////////////////////////////////////

class MemoryBlockNode : public STFNode
	{
	public:
		MemoryPoolBlock memBlock;
	};


////////////////////////////////////////////////////////////////////
// MemoryBlockList
//
// List to hold memory blocks. It only implements Find() as a new method.
// Find() is implemented as simple list walking from the top of the list.
// If MemoryBlockList is sorted, Find() could be optimized. At the time of
// introducing this class, Find() performance is not an issue so non-optimized
// version is being used.
////////////////////////////////////////////////////////////////////

class MemoryBlockList : public STFList
	{
	public:
		MemoryBlockNode * Find (MemoryPoolBlock * memBlock);
	};


////////////////////////////////////////////////////////////////////
// HeapMemoryPoolPU
////////////////////////////////////////////////////////////////////

class HeapMemoryPoolPU : public MemoryPoolAllocator
	{
	protected:

	public:
		HeapMemoryPoolPU(VDRUID unitID) : MemoryPoolAllocator(unitID) {}

	public:
		//IPhysicalUnit implementation.
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	};


////////////////////////////////////////////////////////////////////
// HeapMemoryPoolVU.
////////////////////////////////////////////////////////////////////

class HeapMemoryPoolVU : public VirtualVariableSizeMemoryPoolAllocator
	{
	protected:
		STFMutex getMutex;
		STFFreeListHeapMemoryManager heapManager;
		MemoryBlockList memBlockList;

	public:
		HeapMemoryPoolVU(MemoryPoolAllocator* physicalUnit);
		virtual ~HeapMemoryPoolVU();

	public:
		//IVDRVariableSizeMemoryPoolAllocator implementation.
		virtual STFResult GetMemoryBlock(VDRMemoryBlock** block, uint32 size, IVDRDataHolder* holder = NULL);
		virtual STFResult ReturnMemoryBlocks(VDRMemoryBlock** blocks, uint32 number = 1);

	public:
		//IVirtualUnit implementation.
		virtual STFResult InternalPassivate(const STFHiPrec64BitTime & systemTime);
		virtual STFResult PreemptUnit(uint32 flags);
	};


#endif

//<EOF>

