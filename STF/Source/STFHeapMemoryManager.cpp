///
///	@brief	Heap management support classes implementation
///

#include "STF/Interface/STFHeapMemoryManager.h"
#include "STF/Interface/STFDebug.h"


// DEBUG_MEMORY_PARTITION symbol can be selected from the osselect.make
#if DEBUG_MEMORY_PARTITION
#define DPR_DMP	DP
#else
#define DPR_DMP	DP_EMPTY
#endif

///////////////////////////////////////////////////////////////////////////////
// Helper macros
///////////////////////////////////////////////////////////////////////////////

// All block headers for the memory managemant are aligned by this factor to facilitate memory management
#define BLOCKHEADERALIGNMENT 8

// Align the given address to the factor by increasing it until it fits
#define ALIGNTONEXT(address, factor) (address + (factor - (address % (factor))) % factor)

// Align the given address to the factor by decreasing it until it fits
#define ALIGNTOLAST(address, factor) (address - (address % (factor)))

// Calculate the offset which must be added to the address to fulfill the alignment
#define ALIGNMENTOFFSET(address, factor) ((factor - (address % (factor))) % factor)


////////////////////////////////////////////////////////////////////
// STFFreeListHeapMemoryManager implementation.
////////////////////////////////////////////////////////////////////


STFFreeListHeapMemoryManager::STFFreeListHeapMemoryManager (void)
	:physicalStartAddress(0),
	heapSize(0),
	freeList(NULL)
	{
	}


STFFreeListHeapMemoryManager::STFFreeListHeapMemoryManager (PADDR physicalStartAddressParam, uint32 heapSizeParam)
	{
	Initialize (physicalStartAddressParam, heapSizeParam);
	}


STFResult STFFreeListHeapMemoryManager::Initialize (PADDR physicalStartAddressParam, uint32 heapSizeParam)
	{
	// Initialize memory management by creating the first entry to the free block list which
	// contains the entire memory managed by this partition

	// Make sure the memory block header is aligned by BLOCKHEADERALIGNMENT
	physicalStartAddress = ALIGNTONEXT(physicalStartAddressParam, BLOCKHEADERALIGNMENT);

	// Correct the size
	heapSize = heapSizeParam;
	heapSize -= ALIGNMENTOFFSET(physicalStartAddress, BLOCKHEADERALIGNMENT);

	// Write the Blockheader
	freeList = (FreeMemoryBlockHeader*)physicalStartAddress;

	freeList->blockSize = heapSize;
	freeList->next = NULL;

	DPR_DMP("STFFreeListHeapMemoryManager 0x%8.8x Initialize(): physical=0x%08x size=%d\n", (PADDR)this, physicalStartAddress, heapSize);

	STFRES_RAISE_OK;
	}


STFResult STFFreeListHeapMemoryManager::Allocate (uint32 size, uint32 alignmentFactor, uint8 * &startAddress, PADDR &physicalAddress)
	{
	STFAutoMutex				mutex (&allocationMutex);
	FreeMemoryBlockHeader*	currentHeader;
	FreeMemoryBlockHeader*	lastHeader;
	FreeMemoryBlockHeader*	newHeader;
	uint32						currentAlignmentOffset;
	PADDR						allocationEndAddress;
	PADDR						currentBlockEndAddress;
	uint32						remainingSizeInBlock;

	// Search for a free memory block that is large enough (at first sight)
	currentHeader = freeList;
	lastHeader = NULL;

	// Alignment must not be zero
	assert(alignmentFactor != 0);

//	DPR_DMP("Allocate size: %d, align: %d\n", size, alignmentFactor);

	while (currentHeader)
		{
		// Check if this block is large enough
		//lint --e{414} suppress "possible division by zero"
		currentAlignmentOffset = ALIGNMENTOFFSET((PADDR)currentHeader, alignmentFactor);
		if (currentHeader->blockSize >= size + currentAlignmentOffset)
			{
			// Ok, this block is large enough, even if we have to increase to address due to alignment
			startAddress = (uint8*)currentHeader + currentAlignmentOffset;
			physicalAddress = (PADDR)startAddress;
			allocationEndAddress = (PADDR)startAddress + size;

			//DPR_DMP("STFFreeListHeapMemoryManager Allocate: start %08x physical %08x allocEnd %08x\n", startAddress, physicalAddress, allocationEndAddress);
			
			// Save the end address of the current block because we will need it later
			currentBlockEndAddress = (PADDR)currentHeader + currentHeader->blockSize;

			// If the alignment offset is greater than our header size we leave the header at its place and just decrease its
			// size
			if (currentAlignmentOffset >= sizeof(FreeMemoryBlockHeader))
				{
				// Now correct the size of the remaining block
				// This smaller block is now just large enough that the address that follows this block directly is 
				// the largest adress that is dividable by BLOCKHEADERALIGNMENT that is smaller than startAddress
				// We can assume that currentHeader points to an address that is aligned to BLOCKHEADERALIGNMENT so we can
				// just align currentAlignmentOffset
				currentHeader->blockSize = ALIGNTOLAST(currentAlignmentOffset, BLOCKHEADERALIGNMENT);
				lastHeader = currentHeader;
				}
			else
				{
				// The header whose memory we just allocated is no longer valid, so delete it from the free list
				if (lastHeader)
					lastHeader->next = currentHeader->next;
				else
					freeList = currentHeader->next;
				}

			// Check if we have enough space to create a new free memory block after the current allocation
			remainingSizeInBlock = currentBlockEndAddress - ALIGNTONEXT(allocationEndAddress, BLOCKHEADERALIGNMENT);
			if (remainingSizeInBlock >= sizeof(FreeMemoryBlockHeader))
				{
				// Ok, we have enough space, let's create a new block
				newHeader = (FreeMemoryBlockHeader*)ALIGNTONEXT(allocationEndAddress, BLOCKHEADERALIGNMENT);

				// Fill in data
				newHeader->blockSize = remainingSizeInBlock;

				// Link to other blocks
				if (lastHeader)
					{
					newHeader->next = lastHeader->next;
					lastHeader->next = newHeader;
					}
				else
					{
					newHeader->next = freeList;
					freeList = newHeader;
					}
				}

			// Allocation was successfull, return success
			STFRES_RAISE_OK;
			}

		// We have not yet found a suitable block, go to next element in free list while remebering the current header
		// for block insertion into the free list
		lastHeader = currentHeader;
		currentHeader = currentHeader->next;

		}

	// We haven't found a block that is large enought to satify the request
#if _DEBUG
	DP ("!!!Allocation failed for size %d from STFFreeListHeapMemoryManager(heapSize = %d)\n", size, this->heapSize);
	DP ("\tAvailable blocks:\n");
	FreeMemoryBlockHeader *tempFreeList = freeList;
	while (tempFreeList)
		{
		DP ("\tStart 0x%08x, size %d\n", (uint32)tempFreeList, tempFreeList->blockSize);
		tempFreeList = tempFreeList->next;
		}
#endif

	STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
	}


STFResult STFFreeListHeapMemoryManager::Deallocate (uint8 *startAddress, uint32 size)
	{
	STFAutoMutex				mutex(&allocationMutex);
	PADDR						realAllocationAddress;		
	uint32						realAllocationSize;
	FreeMemoryBlockHeader*	currentHeader;
	FreeMemoryBlockHeader*	lastHeader = NULL;
	FreeMemoryBlockHeader*	deallocatedBlockHeader = NULL;
	
	// First decrease the address given to the last address that is aligned to BLOCKHEADERALIGNMENT
	realAllocationAddress = ALIGNTOLAST((PADDR)startAddress, BLOCKHEADERALIGNMENT);

	// We may have given the program a larger block than it requested originally because we wanted the next
	// fre header to begin on an aligned addres. To obtain the real size, we increase the given one until we have
	// reached an alignment boundary
	realAllocationSize = ALIGNTONEXT(size, BLOCKHEADERALIGNMENT);

	// Search in the free list for adjacent blocks
	currentHeader = freeList;
	while (currentHeader && (PADDR)currentHeader < realAllocationAddress)
		{
		// Check if this block is directly before the block being released
		if ((PADDR)currentHeader + currentHeader->blockSize == realAllocationAddress)
			{
			// Yes, it is, that means we can merge the newly deallocated block to the one
			// in the free list. That means that we just have to adjust the size of the block
			// in the free list.
			currentHeader->blockSize += realAllocationSize;

			deallocatedBlockHeader = currentHeader;
			}

		// Advance to next block in free List
		lastHeader = currentHeader;
		currentHeader = currentHeader->next;
		}

	// If we have merged the deallocated block to the one directly before it we do not need to create a free block header.
	// Otherwise we do exactly this now.
	if (!deallocatedBlockHeader)
		{
		deallocatedBlockHeader = (FreeMemoryBlockHeader*)realAllocationAddress;
		deallocatedBlockHeader->blockSize = realAllocationSize;

		// Link the just created header into the chain
		if (lastHeader)
			lastHeader->next = deallocatedBlockHeader;
		else
			freeList = deallocatedBlockHeader;

		// The currentHeader pointer now either points to NULL or it points to the next free block after the just deallocated block
		deallocatedBlockHeader->next = currentHeader;
		}

	// Now we have to check if the free block after the just deaalocated one is adjacent to it and we can merge them
	if (currentHeader && ((PADDR)deallocatedBlockHeader + deallocatedBlockHeader->blockSize == (PADDR)currentHeader))
		{
		// The blocks are adjacent, so we just add the size of the following block to the just deallocated one and delete it
		deallocatedBlockHeader->blockSize += currentHeader->blockSize;
		deallocatedBlockHeader->next = currentHeader->next;
		}

	STFRES_RAISE_OK;
	}

STFResult STFFreeListHeapMemoryManager::GetStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes)
	{
	STFAutoMutex				mutex(&allocationMutex);
	FreeMemoryBlockHeader	*currentHeader = freeList;

	freeBytes = freeLargestBlockSize = 0;
	while (currentHeader)
		{
		freeBytes += currentHeader->blockSize;
		if (currentHeader->blockSize > freeLargestBlockSize)
			freeLargestBlockSize = currentHeader->blockSize;

		currentHeader = currentHeader->next;
		}

	usedBytes = heapSize - freeBytes;

	DPR_DMP("Manager=0x%8.8x Part Base=0x%08x Size=%08d(%5.5dKB) Free=%8.8d(%5.5dKB) FreeLargest=%8.8d(%5.5dKB) Used=%8.8d(%5.5dKB)\n",  
		(PADDR)this, physicalStartAddress, heapSize, heapSize/1024, freeBytes, freeBytes/1024, freeLargestBlockSize, freeLargestBlockSize/1024, usedBytes, usedBytes/1024);

	STFRES_RAISE_OK;
	}

//<EOF>
