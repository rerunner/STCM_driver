///
/// @file       VDR/Source/Memory/MemoryPoolAllocator.h
///
/// @brief      VDR implementations for memory pool allocators, memory blocks and data ranges.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR private header file
///
/// @date       2002-12-05
///
/// &copy; 2002 ST Microelectronics. All Rights Reserved.
///

#ifndef MEMORYPOOLALLOCATOR_H
#define MEMORYPOOLALLOCATOR_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Interface/Memory/IVDRMemoryPoolAllocator.h"
#include "VDR/Source/Memory/MemoryPartition.h"
#include "VDR/Source/Unit/UnitCollection.h"
#include "VDR/Source/Unit/IMessageDispatcherUnit.h"
#include "VDR/Source/Base/VDRMessage.h"



class VirtualMemoryPoolAllocatorBase;   // forward declaration



////////////////////////////////////////////////////////////////////
//! MemoryPoolAllocator implementation.
////////////////////////////////////////////////////////////////////

class MemoryPoolAllocator : public SharedPhysicalUnit
	{
	friend class VirtualMemoryPoolAllocatorBase;

	protected:
		IPhysicalUnit	*	physicalMemoryPartition;
		IPhysicalUnit	*	messageDispatcherUnit;
	public:
		// Constructor.
		MemoryPoolAllocator (VDRUID unitID);

	public:
		// Partial ITagUnit implementation.

		virtual STFResult GetTagIDs (VDRTID * & ids);

		virtual STFResult InternalConfigureTags (TAG * tags);
		virtual STFResult InternalUpdate (void);

	public:
		// Partial IPhysicalUnit implementation.

		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};



// Base class for virtual memory pool allocator classes. Implements interaction with physical
// MemoryPoolAllocator class and some other functionality common for memory pool allocators.

class VirtualMemoryPoolAllocatorBase : public VirtualUnitCollection,
													public MessageSinkRegistration
	{
	protected:
		MemoryPoolAllocator		*	physicalUnit;

		IMemoryPartition			*	memPartInterface;
		IMessageDispatcherUnit	*	messageDispatcherUnit;

		uint32	totalSize;
		uint32	alignmentFactor;
		uint32	changeSet;
		char		*poolName;

	public:
		// Constructor/destructor.
		VirtualMemoryPoolAllocatorBase (MemoryPoolAllocator * physicalUnit);
		virtual ~VirtualMemoryPoolAllocatorBase (void);

	public:
		// Partial IVDRBase implementation.
		virtual STFResult QueryInterface (VDRIID iid, void * & ifp);

	protected:
		// VirtualUnitCollection implementation.
		virtual STFResult AllocateChildUnits (void);

	public:
		// Partial ITagUnit implementation.
		virtual STFResult InternalConfigureTags (TAG * tags);
		virtual STFResult InternalUpdate (void);
	};



// Virtual memory pool allocator for fixed block size.

class VirtualMemoryPoolAllocator : public VirtualMemoryPoolAllocatorBase,
											  virtual public IVDRMemoryPoolAllocator
	{
	protected:
		uint32	blockSize;
		uint32	clusterSize;

		uint32	numBlocks;		// read only!!!
	public:
		// Constructor.
		VirtualMemoryPoolAllocator (MemoryPoolAllocator * physicalUnit);

	public:
		// Partial IVDRBase implementation.
		virtual STFResult QueryInterface (VDRIID iid, void * & ifp);

	public:
		// Partial ITagUnit implementation.
		virtual STFResult InternalConfigureTags (TAG * tags);

		virtual void DumpMemoryBlockLog (void) {}
	};



// Virtual memory pool allocator for variable block size.

class VirtualVariableSizeMemoryPoolAllocator : public VirtualMemoryPoolAllocatorBase,
															  virtual public IVDRVariableSizeMemoryPoolAllocator
	{
	public:
		// Constructor.
		VirtualVariableSizeMemoryPoolAllocator (MemoryPoolAllocator * physicalUnit);

	public:
		// Partial IVDRBase implementation.
		virtual STFResult QueryInterface (VDRIID iid, void * & ifp);
	};



////////////////////////////////////////////////////////////////////
//! Memory Block implementation.
////////////////////////////////////////////////////////////////////

class MemoryPoolBlock : public VDRMemoryBlock
	{
	public:
#if _DEBUG
		IVDRDataHolder	**loggingArray;
		uint32			maxLoggingEntries;
		uint32			numLoggingEntries;

		void AddToLogging (IVDRDataHolder * holder);
		void RemoveFromLogging (IVDRDataHolder * holder);
#endif

		uint8		*userBase;		// User Mode logical address
		uint8		*kernelBase;	// Kernel Mode logical address

		PADDR		physicalAddress;
		uint32	size, clusters;

		IVDRMemoryPoolDeallocator	*pool;

		STFInterlockedInt counter;

#if _DEBUG
		void DumpLogging (void);
#endif
	public:
		// Constructor/destructor.
		MemoryPoolBlock (void);
		virtual ~MemoryPoolBlock (void);

	public:
		// VDRMemoryBlock implementation.

		virtual uint32 AddRef (IVDRDataHolder * holder = NULL);
		virtual uint32 Release (IVDRDataHolder * holder = NULL);

		virtual uint8 *GetStart (void) const	{return userBase;}
		virtual uint32 GetSize (void)	const		{return size;}

	public:
		// All of these functions may be used only in kernel mode!

		void SetInitialCounter (IVDRDataHolder * holder = NULL);

		virtual uint8 *GetKernelStart (void) {return kernelBase;}

		virtual PADDR GetPhysicalAddress (void) {return physicalAddress;}
	};



// This macro is convenient to use on a VDRDataRange.
#define GetKernelStart(dataRange) ((MemoryPoolBlock *)((dataRange)->block)->GetKernelStart() + (dataRange)->offset;)



#endif
