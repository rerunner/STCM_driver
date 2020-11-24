///
///	@brief	Heap management support classes definitions
///

#ifndef STFHEAPMEMORYMANAGER_H
#define STFHEAPMEMORYMANAGER_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/STFSynchronisation.h"

/*!
  @brief STFHeapMemoryManager class definition

  STFHeapMemoryManager provides generic interfacing to heap memory management.
  The memory for the heap is allocated outside of STFHeapMemoryManager. The
  inputs given to STFHeapMemoryManager are physical base address and size of
  memory available for heap. Inside this memory area, STFHeapMemoryManager
  provides allocation and de-allocation of variable-sized memory blocks.

*/

class STFHeapMemoryManager
   {
   public:
   virtual ~STFHeapMemoryManager() {}; // NHV: Added for g++ 4.1.1
   /*!
     Initializes heap manager. Must be called before calling Allocate() and Deallocate().
     @param physicalStartAddressParam: Physical start address of memory blcok to be used for heap.
     @param heapSizeParam: Size of the block to be used for heap.
   */
   virtual STFResult Initialize (PADDR physicalStartAddressParam, uint32 heapSizeParam) = 0;

   /*!
     Allocate block of memory. @see Deallocate
     @param size: Size of block to allocate in bytes.
     @param alignmentFactor: The byte-boundary on which is start of the block to be aligned. The start
     address the block will be divisible by alignmentFactor.
     @param startAddress: Upon successful allocation, holds the kernel start address of the newly allocated block.
     Normally, this is the address used by CPU.
     @param physicalAddress: Upon successful allocation, holds physical start address of the newly allocated block.
     This is the address to be used by circuts other than CPU, such as DMA
     @return STFRES_RAISE_OK upon success, STFRES_NOT_ENOUGH_MEMORY if the free space big enough to hold required
     size could not be found, or if STFHeapMemoryManager has not been initialized.
   */
   virtual STFResult Allocate (uint32 size, uint32 alignmentFactor, uint8 * &startAddress, PADDR &physicalAddress) = 0;

   /*!
     Releases block of memory prevously allocated with Allocate(). @see Allocate
     @param startAddress: Kernel address of the block to free returned by Allocate().
     @param size: Size of the block specifed in Allocate()
     @return: STFRES_RAISE_OK
   */
   virtual STFResult Deallocate (uint8 *startAddress, uint32 size) = 0;

   /*!
     Returns the status of the heap.
     @param freeBytes [out]: Total amount of memory available for future allocations.
     @param freeLargestBlockSize [out]: The largest contiguous memory block size available for future allocations.
     @param usedBytes [out]: Total amount of the allocated memory.
     @return: STFRES_RAISE_OK
   */
   virtual STFResult GetStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes) = 0;
   };


/*!
  @brief STFFreeListHeapMemoryManager class definition

  Specific implementation of Memory Heap Manager. STFFreeListHeapMemoryManager maintains
  only one list of free memoryblocks to manage the heap.

  @see STFHeapMemoryManager
*/

class STFFreeListHeapMemoryManager : public STFHeapMemoryManager
   {
   protected:

   // Block management type
   struct FreeMemoryBlockHeader
      {
      uint32 blockSize;
      FreeMemoryBlockHeader * next;
      };

   STFMutex						allocationMutex;		// protects the internal variables during allocation/deallocation

   PADDR							physicalStartAddress;
   uint32						heapSize;
   FreeMemoryBlockHeader	*freeList;

   public:
   STFFreeListHeapMemoryManager (void);
   STFFreeListHeapMemoryManager (PADDR physicalStartAddressParam, uint32 heapSizeParam);
   virtual ~STFFreeListHeapMemoryManager (void) {}

   /*!
     Initializes heap manager. @see STFHeapMemoryManager
   */
   virtual STFResult Initialize (PADDR physicalStartAddressParam, uint32 heapSizeParam);

   /*!
     Allocate block of memory. @see STFHeapMemoryManager
   */
   virtual STFResult Allocate (uint32 size, uint32 alignmentFactor, uint8 * &startAddress, PADDR &physicalAddress);

   /*!
     Releases block of memory prevously allocated with Allocate(). @see STFHeapMemoryManager
   */
   virtual STFResult Deallocate (uint8 *startAddress, uint32 size);

   /*!
     Returns the status of the heap. @see STFHeapMemoryManager
   */
   virtual STFResult GetStatus (uint32 & freeBytes, uint32 & freeLargestBlockSize, uint32 & usedBytes);
   };



#endif

