///
///	@brief	HeapMemoryPool physical and virtual unit implementation
///

#include "Device/Source/Unit/Memory/HeapMemoryPool.h"


UNIT_CREATION_FUNCTION(CreateHeapMemoryPool, HeapMemoryPoolPU)


////////////////////////////////////////////////////////////////////
// MemoryBlockList implementation.
////////////////////////////////////////////////////////////////////

MemoryBlockNode * MemoryBlockList::Find (MemoryPoolBlock* memBlock)
	{
	MemoryBlockNode *currMemBlockNode = (MemoryBlockNode*)First();

	while (currMemBlockNode  &&  &currMemBlockNode->memBlock != memBlock)
		currMemBlockNode = (MemoryBlockNode*)currMemBlockNode->Succ();

	return currMemBlockNode;
	}



////////////////////////////////////////////////////////////////////
// HeapMemoryPoolPU implementation.
////////////////////////////////////////////////////////////////////

STFResult HeapMemoryPoolPU::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	STFResult res;

	// Create our virtual unit.
	unit = new HeapMemoryPoolVU(this);
	if (unit == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	if (STFRES_FAILED(res = unit->Connect(parent, root)))
		{
		delete unit;
		unit = NULL;
		}

	STFRES_RAISE(res);
	}



////////////////////////////////////////////////////////////////////
// HeapMemoryPoolVU implementation.
////////////////////////////////////////////////////////////////////

HeapMemoryPoolVU::HeapMemoryPoolVU(MemoryPoolAllocator * physicalUnit)
	: VirtualVariableSizeMemoryPoolAllocator(physicalUnit)
	{
	}


HeapMemoryPoolVU::~HeapMemoryPoolVU()
	{
	MemoryBlockNode *block, *next;

	//Clear the list of allocated blocks if it by some chance is not empty
	if (! memBlockList.IsEmpty())
		{
		block = (MemoryBlockNode*)memBlockList.First();
		while (block != NULL)
			{
			next = (MemoryBlockNode*)block->Succ();
			delete block;
			block = next;
			}
		}
	}


/////////////////////////////////////////////////////////////////////////
// HeapMemoryPoolVU - IVDRVariableSizeMemoryPoolAllocator implementation.
/////////////////////////////////////////////////////////////////////////

STFResult HeapMemoryPoolVU::GetMemoryBlock(VDRMemoryBlock** block, uint32 size, IVDRDataHolder* holder /*= NULL*/)
	{
	STFResult res;
	uint8* startAddress;
	PADDR physicalAddress;

	getMutex.Enter();

	if (STFRES_SUCCEEDED(res = heapManager.Allocate(size, alignmentFactor, startAddress, physicalAddress)))
		{
		MemoryBlockNode* mbn = new MemoryBlockNode;
		if (mbn == NULL)
			res = STFRES_NOT_ENOUGH_MEMORY;
		else
			{
			memBlockList.InsertLast(mbn);

			// block->userBase cannot be filled in as there is no user context available.
			//??? does this need to be solved here or will an instance at the VDR border take care of this?
			mbn->memBlock.kernelBase = mbn->memBlock.userBase = startAddress;
			mbn->memBlock.physicalAddress = physicalAddress;
			mbn->memBlock.size = size;
			mbn->memBlock.pool = (IVDRMemoryPoolDeallocator*)this;
			mbn->memBlock.SetInitialCounter (holder);

			*block = &(mbn->memBlock);
			}
		}

	getMutex.Leave();

	STFRES_RAISE(res);
	}



// It must be possible to call this function from an interrupt!

STFResult HeapMemoryPoolVU::ReturnMemoryBlocks(VDRMemoryBlock ** blocks, uint32 number)
	{
	MemoryPoolBlock **b = (MemoryPoolBlock **)blocks;

	for (uint32 i = 0;  i < number;  i++)
		{
		//Remove the block from the list of allocated blocks
		MemoryBlockNode* mbn = memBlockList.Find(b[i]);

		//Return blocks to memory pool heap
		heapManager.Deallocate(b[i]->GetStart(), b[i]->GetSize());

		if (mbn)
			{
			memBlockList.Remove(mbn);
			delete mbn;
			}
		}

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// HeapMemoryPoolVU - IVirtualUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult HeapMemoryPoolVU::InternalPassivate(const STFHiPrec64BitTime & systemTime)
	{
	//If list of memory block is not empty, some blocks are not released, so
	//there may be someone still using them, and we can't passivate.
	//The base class must not be passivated, either.
	if (!memBlockList.IsEmpty())
		STFRES_RAISE(STFRES_OBJECT_IN_USE);

	//Passivate base class.
	STFRES_RAISE(VirtualVariableSizeMemoryPoolAllocator::InternalPassivate(systemTime));
	}

STFResult HeapMemoryPoolVU::PreemptUnit(uint32 flags)
	{
	MemoryBlockNode *block, *next;

	// First, the base class needs to do its stuff.
	STFRES_REASSERT(VirtualVariableSizeMemoryPoolAllocator::PreemptUnit(flags));

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		uint8 *startAddress;
		PADDR physicalStartAddress;

		// The single memory partition has just been allocated.

		//Clear the list of allocated blocks if it by some chance is not empty
		if (! memBlockList.IsEmpty())
			{
			block = (MemoryBlockNode*)memBlockList.First();
			while (block != NULL)
				{
				next = (MemoryBlockNode*)block->Succ();
				delete block;
				block = next;
				}

			memBlockList.Clear();
			}

		// Initialize the block structures and partition the single memory partition.
		startAddress = memPartInterface->GetStartAddress();
		physicalStartAddress = memPartInterface->GetPhysicalAddress();

		// Initialize memory management
		STFRES_RAISE(heapManager.Initialize(physicalStartAddress, totalSize));
		}

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		// Deallocate the blocks from the pool and clear the list
		if (! memBlockList.IsEmpty())
			{
			block = (MemoryBlockNode*)memBlockList.First();
			while (block != NULL)
				{
				heapManager.Deallocate (block->memBlock.GetStart(), block->memBlock.GetSize());
				next = (MemoryBlockNode*)block->Succ();
				delete block;
				block = next;
				}

			memBlockList.Clear();
			}

		// The single memory partition will now be deallocated in its own class...
		}

	STFRES_RAISE_OK;
	}

//<EOF>

