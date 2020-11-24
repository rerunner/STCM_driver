//
// PURPOSE:   VDR declarations for memory pool allocators, memory blocks and data ranges.
//

/*! \file
  \brief VDR declarations for memory pool allocators, memory blocks and data ranges.
*/

#ifndef IVDR_MEMORYPOOLALLOCATOR_H
#define IVDR_MEMORYPOOLALLOCATOR_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFMessage.h"
#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Base/IVDRDebug.h"
#include "VDR/Interface/Base/IVDRMessage.h"
#include "VDR/Interface/Unit/IVDRTags.h"



// Forward declaration.
class VDRMemoryBlock;



////////////////////////////////////////////////////////////////////
// Tag type definition.
////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_MEMPOOL_ALLOCATOR = 0x00019000;   // created by the VDR ID value manager

//MKTAG (MEMPOOL_WAIT_FOR_BLOCKS,		VDRTID_MEMPOOL_ALLOCATOR, 0x0000000, bool)   NOTE: this tag is now obsolete
MKTAG (MEMPOOL_TOTALSIZE,				VDRTID_MEMPOOL_ALLOCATOR, 0x0000001, uint32)		// total size in bytes
   MKTAG (MEMPOOL_BLOCKSIZE,				VDRTID_MEMPOOL_ALLOCATOR, 0x0000002, uint32)		// size of single block in bytes

   MKTAG (MEMPOOL_ALIGNMENT_FACTOR,		VDRTID_MEMPOOL_ALLOCATOR, 0x0000003, uint32)		// in bytes, e.g. "8" for 64 bit alignment

   MKTAG (MEMPOOL_NUMBER_OF_BLOCKS,		VDRTID_MEMPOOL_ALLOCATOR, 0x0000004, uint32)		// read only!!!

   MKTAG (MEMPOOL_POOLNAME,				VDRTID_MEMPOOL_ALLOCATOR, 0x0000005, char *)		// pointer to ASCII name
   MKTAG (MEMPOOL_CLUSTERSIZE,			VDRTID_MEMPOOL_ALLOCATOR, 0x0000006, uint32)	

   static const VDRMID VDRMID_MEMORY_POOL_ALLOCATOR = 0x0004f000;
   static const VDRMID VDRMID_MEMORY_POOL_ALLOCATOR_BLOCKS_AVAILABLE	= VDRMID_MEMORY_POOL_ALLOCATOR + 1;



////////////////////////////////////////////////////////////////////
/// Interface for tracking Memory Block Referencing
////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_VDR_DATA_HOLDER = 0x0000001e;

class IVDRDataHolder : virtual public IVDRGenericDebugInfo
   {
   };


////////////////////////////////////////////////////////////////////
//! Memory Pool Allocator public interface.
////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_VDR_MEMORYPOOL_ALLOCATOR = 0x00000017;   // created by the VDR ID value manager
static const VDRIID VDRIID_VARIABLE_SIZE_MEMORYPOOL_ALLOCATOR = 0x8000005c;   // created by the VDR ID value manager
static const VDRIID VDRIID_VDR_CONSTANTOFFSET_MEMORYPOOL_ALLOCATOR = 0x0000001c;		// created by the ID value manager

/*!
  \brief Base class for memory pool allocator interfaces

  This base class is not to be queried as an interface. Its purpose is to provide ReturnMemoryBlocks() access
  to object types that are shared between different types of memory pool allocators. (MemoryPoolBlock is one example
  of such objects.) When initializing these objects, previously obtained pointer to specific memory pool interface
  should be simply type-casted to IVDRMemoryPoolDeallocator.
*/

class IVDRMemoryPoolDeallocator : virtual public IVDRBase
   {
   public:
   //! Return blocks to memory pool, regardless of their reference counter values.
   /*!
     \param blocks IN: The array of VDRMemoryBlock pointers.
     \param number IN: The number of memory blocks to return.

     \return Standard Error

     Note that it must be possible to call this function from an interrupt.
   */
   virtual STFResult ReturnMemoryBlocks (VDRMemoryBlock ** blocks, uint32 number = 1) = 0;
   };


class IVDRMemoryPoolAllocator : virtual public IVDRMemoryPoolDeallocator
   {
   public:
   //! Get several memory blocks from the pool allocator.
   /*!
     \param blocks INOUT: The array of VDRMemoryBlock pointers to fill in with the obtained blocks.
     \param minWait IN: The minimum number of blocks to wait for before the call returns.
     \param number IN: The number of memory blocks to obtain.
     \param done OUT: The number of memory blocks that were obtained (see below).

     \return Standard Error

     Gets a number of memory blocks from the allocator's pool. If not enough blocks are available,
     the call waits until "minWait" blocks are available. It can return with less blocks than
     requested if "minWait" < "number". After returning, "done" will reflect the number of
     successfully obtained blocks.

     The reference counter of each memory block is set to 1.

     If "minWait" is zero, then the call may return immediately with "done" being zero as well.
     If "minWait" >= "number", then the call will return with "done" equal to "number".
   */
   virtual STFResult GetMemoryBlocks (VDRMemoryBlock ** blocks, uint32 minWait, uint32 number, uint32 &done, IVDRDataHolder * holder = NULL) = 0;

   virtual void DumpMemoryBlockLog (void) = 0;
   };


typedef IVDRMemoryPoolAllocator	*	IVDRMemoryPoolAllocatorPtr;


//! Variable block size memory pool allocator.
/*!
  Provides blocks of variable sizes from previously allocated memory pool. Required block size is specified
  at block request time in GetMemoryBlock().
*/
class IVDRVariableSizeMemoryPoolAllocator : virtual public IVDRMemoryPoolDeallocator
   {
   public:
   //! Get several memory blocks from the pool allocator.
   /*!
     \param blocks INOUT: VDRMemoryBlock pointer to fill in with the obtained blocks.
     \param size IN: Size of the block in bytes.
	
     \return Standard Error

     Gets a memory block of specified size from allocator's memory pool.

     The reference counter of block is set to 1.
   */
   virtual STFResult GetMemoryBlock (VDRMemoryBlock ** block, uint32 size, IVDRDataHolder * holder = NULL) = 0;
   };


/*!
  "Memory Pool Allocator with constant data range offset" public unit interface.
  This pool allocator cannot set the MEMPOOL_TOTALSIZE tag.
*/
class IVDRConstantOffsetMemoryPoolAllocator : virtual public IVDRMemoryPoolAllocator
   {
   public:
   //! Get the offset value for Data Ranges associated with this allocator.
   /*!
     \param offset OUT: The offset value for Data Ranges.

     \return Standard Error

     Note that while the Data Range Offset Buffer Pool is activated, the offset is 
     constant and clients are permitted to cache it in a local variable.
   */
   virtual STFResult GetDataRangeOffset (uint32 & offset) = 0;
   };


////////////////////////////////////////////////////////////////////
//! Memory Block abstract class declaration.
////////////////////////////////////////////////////////////////////

class VDRMemoryBlock
   {
   public:
   virtual ~VDRMemoryBlock(){}; // NHV: Added for g++ 4.1.1
   virtual uint32 AddRef (IVDRDataHolder * holder = NULL) = 0;
   virtual uint32 Release (IVDRDataHolder * holder = NULL) = 0;   // callable from an interrupt

   virtual uint8 *GetStart (void) const = 0;
   virtual uint32 GetSize  (void) const = 0;
   };



////////////////////////////////////////////////////////////////////
//! Data Range is no class on purpose, as it should not be extended.
////////////////////////////////////////////////////////////////////

///@brief VDRDataRange memory type
///
/// Each VDRRange has a specific memory type ID, which designates the
/// type and format of data that is stored in the memory referenced by
/// the memory range.
///
/// The base ID for each memory type shall be allocated using the
/// STIVAMA identifier manager.  The lower 12 bits of the ID are used
/// for additional information regarding the memory layout.
///
/// The base ID VDRRID_BYTES designates an unorderd sequence of BYTES.
/// Is is used as a default, if no better description is available.
///
/// Ranges may be split or span one or multiple access unit.  The sematic
/// of an access unit depends on the memory type itself.  A sequence of
/// bytes does not have more than one access unit.  An access unit may
/// be an audio frame, or a DVD sector or a bitmap header etc.
///
/// Additional flags are:
///
///		VDRRIDF_UNIT_START	: The range contains a start of an access unit
///
///		VDRRIDF_UNIT_END		: The range contains an end of an access unit
///
///		VDRRIDF_UNIT_ALIGNED	: The range is aligned to an access unit
///
/// 
typedef	uint32	VDRRID;

#define DATA_RANGEID_TYPE(id)		((id) & 0xfffff000)
#define DATA_RANGEID_FLAGS(id)	((id) & 0x00000fff)

static const uint32 VDRRIDF_UNIT_START		=	0x00000001;
static const uint32 VDRRIDF_UNIT_END		=	0x00000002;
static const uint32 VDRRIDF_UNIT_ALIGNED	=	0x00000004;

static const VDRRID VDRRID_BYTES				=	0x00001000;

///@brief Reference to a typed range of memory
///
/// A VDRDataRange is a reference to a type range of memory.  The memory itself
/// is provided by a VDRMemoryBlock.  The VDRDataRange points to a segment inside
/// this memory block.  A type identifier can be used to provide information
/// about the content of this data block.
///
struct VDRDataRange
   {
   /// Reference to the memory block that contains the memory
   /// section of this range.
   VDRMemoryBlock		*block;

   /// The offset into the memory block, where this range starts.
   /// The starting address of this range is thus the start of the
   /// block + the offset.
   uint32				offset;	

   /// The size of this range.  The end of the range is therefore
   /// the start of the block + offset + size
   uint32				size;		

   /// The type of the data stored in the memory of this range.
   uint32				type;		

   /// Increment the reference counter of the memory block used by
   /// this data range.
   uint32 AddRef  (IVDRDataHolder * holder = NULL)	{return block->AddRef (holder);}

   /// Decrement the reference counter of the memory block used by
   /// this data range.  When the memory counter reaches zero, the
   /// block is returned to the allocator.
   uint32 Release (IVDRDataHolder * holder = NULL)	{return block->Release (holder);} 

   /// Get the start address of the ranges data.
   uint8 *GetStart (void)	const	{return block->GetStart() + offset;}

   /// Initialize a data range.  We avoid the definition of a C++ constructor to
   /// enable the use of a memory range inside a union.  You should always pass
   /// valid values into the "Init" call.
   ///
   /// @param block	The memory block used by the range.
   /// @param offset The offset into the memory block, where the range starts
   /// @param size   The size of the range
   /// @param type   The type of data stored in the memory of the range
   ///
   void Init (VDRMemoryBlock *block, uint32 offset, uint32 size, VDRRID type = VDRRID_BYTES)
      { 
      if (block != NULL)
         assert(offset + size <= block->GetSize());

      this->block = block;  this->offset = offset;  this->size = size; this->type = type;
      }
   };

///@brief Create a sub section of a data range
///
/// This function creates a data range that describes part of the source
/// data range.  The reference counter of the memory object is _not_ incremented.
/// The size of this new range may not be larger than the original range.
/// The new range will have the same type as the parent range.
///
///@param range The parent range
///@param offset Offset into the parent range (not into the memory block)
///@param size Size of the new range
///@result a subrange inside the parent range
///
static inline VDRDataRange VDRSubDataRange(VDRDataRange range, uint32 offset, uint32 size)
   {
   assert(offset + size <= range.size);
	
   range.offset += offset;
   range.size = size;

   return range;
   }

///@brief Create a sub section of a data range with a new type
///
/// This function creates a data range that describes part of the source
/// data range.  The reference counter of the memory object is _not_ incremented.
/// The size of this new range may not be larger than the original range.
/// The new range will receive a new type.
///
///@param range The parent range
///@param offset Offset into the parent range (not into the memory block)
///@param size Size of the new range
///@type Type of the new data range
///@result a subrange inside the parent range
///
static inline VDRDataRange VDRSubDataRange(VDRDataRange range, uint32 offset, uint32 size, VDRRID type)
   {
   assert(offset + size <= range.size);

   range.offset += offset;
   range.size = size;
   range.type = type;

   return range;
   }



#endif
