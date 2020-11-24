//
// PURPOSE:   VDR implementation for a linear memory pool allocator.
//

#include "Device/Source/Unit/Memory/LinearMemoryPool.h"
#include "STF/Interface/STFDebug.h"
#include <assert.h>


#ifndef CONFIG_DEBUG_REFERENCE_COUNTING
#define CONFIG_DEBUG_REFERENCE_COUNTING 0
#endif

#define DEBUG_REFERENCE_COUNTING (_DEBUG && CONFIG_DEBUG_REFERENCE_COUNTING)


///////////////////////////////////////////////////////////////////////////////
// Global unit creation function.
///////////////////////////////////////////////////////////////////////////////

// We use a macro to implement the unit creation, in order to avoid mistakes.
UNIT_CREATION_FUNCTION (CreateLinearMemoryPool, LinearMemoryPool)



////////////////////////////////////////////////////////////////////
// LinearMemoryPool partial IPhysicalUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult LinearMemoryPool::CreateVirtual (IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	STFResult res;

	// Create our virtual unit.
	unit = new VirtualLinearMemoryPool (this);
	if (unit == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	if (STFRES_FAILED(res = unit->Connect (parent, root)))
		{
		delete unit;
		unit = NULL;
		}

	STFRES_RAISE(res);
	}



////////////////////////////////////////////////////////////////////
//! VirtualLinearMemoryPool implementation.
////////////////////////////////////////////////////////////////////

VirtualLinearMemoryPool::VirtualLinearMemoryPool (MemoryPoolAllocator * physicalUnit)
	: VirtualMemoryPoolAllocator (physicalUnit)
	{
	poolBlocks = NULL;
	allocationFailed = false;
	}

VirtualLinearMemoryPool::~VirtualLinearMemoryPool (void)
	{
	delete[] poolBlocks;
	}



////////////////////////////////////////////////////////////////////
// VirtualLinearMemoryPool IVDRMemoryPoolAllocator implementation.
////////////////////////////////////////////////////////////////////

void VirtualLinearMemoryPool::DumpMemoryBlockLog (void)
	{
#if DEBUG_REFERENCE_COUNTING
#if 1
	uint32				freeCount, totalCount, userCount;
	IVDRDataHolder	*	holders[64];
	uint32				holderCounts[64];
	uint32				i, j, k;

	freeCount = totalCount = userCount = 0;
	STFGlobalLock();
	for(i=0; i<numBlocks; i++)
		{
		if (poolBlocks[i].counter != -1)
			{
			for(k=0; k<poolBlocks[i].numLoggingEntries; k++)
				{
				j = 0;
				while (j < userCount && poolBlocks[i].loggingArray[k] != holders[j])
					j++;
				if (j == userCount)
					{
					holders[userCount++] = poolBlocks[i].loggingArray[k];				
					holderCounts[j] = 0;
					}
				holderCounts[j]++;
				}
			}
		else
			freeCount++;

		totalCount++;
		}
	STFGlobalUnlock();

	DEBUGLOG(LOGID_BLOCK_LOGGING, "TOTAL %3d FREE %3d USERS %2d\n", totalCount, freeCount, userCount);
	for(i=0; i<userCount; i++)
		{
		DEBUGLOG(LOGID_BLOCK_LOGGING, "LOCKS %4d USER", holderCounts[i]);
		if (holders[i])
			holders[i]->PrintDebugInfo(LOGID_BLOCK_LOGGING);
		else
			DEBUGLOG(LOGID_BLOCK_LOGGING, "NULL");
		DEBUGLOG(LOGID_BLOCK_LOGGING, "\n");
		}

#else
	uint32	i;

	for(i = 0; i < numBlocks; i++)
		{
		if (poolBlocks[i].counter != -1)
			{
			DEBUGLOG(LOGID_BLOCK_LOGGING, "BLOCK [%3d] ", i);
			poolBlocks[i].DumpLogging();	
			}
		}
#endif
#endif
	}



STFResult VirtualLinearMemoryPool::GetMemoryBlocks (VDRMemoryBlock ** blocks, uint32 minWait, uint32 number, uint32 &done, IVDRDataHolder * holder)
	{
	MemoryPoolBlock *	b;
	uint32				i, j;
	uint32				blocksObtained;
	uint32				clusterSize = this->clusterSize;

	assert (poolBlocks != NULL);   // the virtual unit must be active

	if (minWait > number)
		minWait = number;

	//??? This implementation is not optimal, but only a first approach. We sequentially select
	// free blocks, starting from the last end position of the previous search.
	while (number > 0)
		{
		getMutex.Enter ();

		blocksObtained = 0;
		i = nextSearchPosition;

		if (clusterSize == 1)
			{
			// Non-clustered mode. First pass: Find the available blocks.
			while (blocksObtained < number  &&  i < numBlocks)
				{
				b = &poolBlocks[i];
				if (b->counter == -1)
					blocks[blocksObtained++] = b;   // free block available
				i++;
				}
			if (blocksObtained < number)
				{
				// End of array reached, but not enough blocks yet. Wrap-around to the beginning of the array.
				i = 0;
				while (blocksObtained < number  &&  i < nextSearchPosition)
					{
					b = &poolBlocks[i];
					if (b->counter == -1)
						blocks[blocksObtained++] = b;   // free block available
					i++;
					}
				}

			if (blocksObtained >= minWait)
				{
				// We have all the blocks or we do not wait for them to become available.
				nextSearchPosition = i;

				done = blocksObtained;

				// Second pass: Set the reference counters to one.
				MemoryPoolBlock **p = (MemoryPoolBlock **)blocks;
				for (i = 0;  i < blocksObtained;  i++)
					{
					p[i]->size = blockSize;
					p[i]->clusters = 1;
					p[i]->SetInitialCounter (holder);
					}

				if (done == 0) allocationFailed = true;

				getMutex.Leave ();
				STFRES_RAISE_OK;   // done
				}
			}
		else
			{
			// Clustered pool allocator mode. First pass: Find the available blocks.
			// Ask Ulrich Sigmund what this is doing...
			while (blocksObtained < number && i + clusterSize <= numBlocks)
				{
				b = &poolBlocks[i];
				
				j = clusterSize;
				while (j > 0 && b[j-1].counter == -1)
					j--;

				if (j == 0)
					{
					blocks[blocksObtained++] = b;
					i += clusterSize;
					}
				else
					i += j;
				}

			if (blocksObtained < number)
				{
				i = 0;
				while (blocksObtained < number && i + clusterSize <= nextSearchPosition)
					{
					b = &poolBlocks[i];
					
					j = clusterSize;
					while (j > 0 && b[j-1].counter == -1)
						j--;

					if (j == 0)
						{
						blocks[blocksObtained++] = b;
						i += clusterSize;
						}
					else
						i += j;
					}
				}

			if (blocksObtained >= minWait)
				{
				// We have all the blocks or we do not wait for them to become available.
				nextSearchPosition = i;

				done = blocksObtained;

				// Second pass: Set the reference counters to one.
				MemoryPoolBlock **p = (MemoryPoolBlock **)blocks;
				for (i = 0;  i < blocksObtained;  i++)
					{
					p[i]->clusters = clusterSize;
					p[i]->size = blockSize * clusterSize;
					for (j = 0; j < clusterSize; j++)
						{
						p[i][j].counter = 0;
						}
					p[i]->SetInitialCounter (holder);
					}

				if (done == 0) allocationFailed = true;

				getMutex.Leave ();
				STFRES_RAISE_OK;   // done
				}
			}

		// When we get here, we have searched the array once, but not enough blocks were available.
		getMutex.Leave ();

		// Wait until enough blocks are available.
		waitSemaphore.Wait ();
		}

	STFRES_RAISE_OK;
	}



// Remember that it must be possible to call this function from an interrupt!

STFResult VirtualLinearMemoryPool::ReturnMemoryBlocks (VDRMemoryBlock ** blocks, uint32 number)
	{
	MemoryPoolBlock **b = (MemoryPoolBlock **)blocks;
	uint32	i, j;

	// To forcefully return memory blocks, we just need to force their counters to -1.
	// The search process will later find the free blocks.
	// Note that zero cannot be used because there would be a racing condition if the
	// Release() call reached zero but did not call ReturnMemoryBlocks() yet.
	for (i = 0;  i < number;  i++)
		{
		for (j = 0; j < b[i]->clusters; j++)
			b[i][j].counter = -1;
		}

	// Now trigger any waiting GetMemoryBlocks().
	waitSemaphore.Signal ();
	if (allocationFailed)
		{
		allocationFailed = false;
		this->SendMessage(STFMessage(VDRMID_MEMORY_POOL_ALLOCATOR_BLOCKS_AVAILABLE, 0, 0), false);
		}

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// VirtualLinearMemoryPool partial IVirtualUnit overload implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualLinearMemoryPool::InternalPassivate (const STFHiPrec64BitTime & systemTime)
	{
	MemoryPoolBlock *b;
	uint32 i;

	if (poolBlocks != NULL)
		{
		// If any block counter is not -1, somebody is still using the block and
		// we must not passivate, but return an error. The base class must also
		// not be called.
		for (b = poolBlocks, i = 0;  i < numBlocks;  i++, b++)
			{
			if (b->counter != -1)
				{
#if DEBUG_REFERENCE_COUNTING
				DP("MemoryPool passivation with blocks in use\n");
				DEBUGLOG(LOGID_BLOCK_LOGGING, "MemoryPool passivation with blocks in use\n");
				DumpMemoryBlockLog();
#endif
				STFRES_RAISE(STFRES_OBJECT_IN_USE);
				}
			}
		}

	// Call base class.
	STFRES_RAISE(VirtualMemoryPoolAllocator::InternalPassivate (systemTime));
	}

STFResult VirtualLinearMemoryPool::PreemptUnit (uint32 flags)
	{
	uint8 *startAddress;
	PADDR physicalAddress;
	MemoryPoolBlock *b;
	uint32 i;

	// First, the base class needs to do its stuff.
	STFRES_REASSERT(VirtualMemoryPoolAllocator::PreemptUnit (flags));

	if (flags & VDRUALF_PREEMPT_START_NEW)
		{
		// The single memory partition has just been allocated. Now create the block structures.

		// If this assertion fails, the caller should check if MEMPOOL_TOTALSIZE includes
		// MEMPOOL_ALIGNMENT_FACTOR in addition to MEMPOOL_BLOCKSIZE.
		assert (numBlocks > 0);

		delete[] poolBlocks;
		poolBlocks = new MemoryPoolBlock[numBlocks];
		if (poolBlocks == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

		// Initialize the block structures and partition the single memory partition.
		startAddress = memPartInterface->GetStartAddress ();
		physicalAddress = memPartInterface->GetPhysicalAddress ();
		for (b = poolBlocks, i = 0;  i < numBlocks;  i++, b++)
			{
			// b->userBase cannot be filled in as there is no user context available.
			//??? does this need to be solved here or will an instance at the VDR border take care of this?
			b->kernelBase = b->userBase = startAddress;
			b->physicalAddress = physicalAddress;
			b->size = blockSize;
			b->pool = (IVDRMemoryPoolDeallocator*)this;
			b->counter = -1;
			startAddress += blockSize;
			physicalAddress += blockSize;
			}
		nextSearchPosition = 0;
		}

	if (flags & VDRUALF_PREEMPT_STOP_PREVIOUS)
		{
		// Deallocate the block structures.
		delete[] poolBlocks;
		poolBlocks = NULL;

		// The single memory partition will now be deallocated in its own class...
		}

	STFRES_RAISE_OK;
	}
