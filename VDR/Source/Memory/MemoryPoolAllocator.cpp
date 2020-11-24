///
/// @file       VDR/Source/Memory/MemoryPoolAllocator.cpp
///
/// @brief      VDR implementations for memory pool allocators, memory blocks and data ranges.
///
/// @author     Dietmar Heidrich
///
/// @par OWNER: Dietmar Heidrich
///
/// @par SCOPE: VDR private implementation file
///
/// @date       2003-01-15
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "STF/Interface/Types/STFBasicTypes.h"
#include "VDR/Source/Memory/MemoryPoolAllocator.h"
#include "STF/Interface/STFDebug.h"
#include <assert.h>

#define POOLNAME	"Unknown"

#ifndef CONFIG_DEBUG_REFERENCE_COUNTING
#define CONFIG_DEBUG_REFERENCE_COUNTING 0
#endif

#define REFERENCE_COUNT_DEBUG (_DEBUG && CONFIG_DEBUG_REFERENCE_COUNTING)


////////////////////////////////////////////////////////////////////
// MemoryPoolAllocator partial ITagUnit interface implementation.
////////////////////////////////////////////////////////////////////

STFResult MemoryPoolAllocator::GetTagIDs (VDRTID * & ids)
	{
	static const VDRTID supportedTagTypes[] =
		{
		VDRTID_MEMPOOL_ALLOCATOR,
		VDRTID_DONE
		};

	ids = (VDRTID *)supportedTagTypes;
	STFRES_RAISE_OK;
	}

STFResult MemoryPoolAllocator::InternalConfigureTags (TAG * tags)
	{
	STFRES_RAISE_OK;
	}


STFResult MemoryPoolAllocator::InternalUpdate (void)
	{
	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// MemoryPoolAllocator partial IPhysicalUnit implementation.
////////////////////////////////////////////////////////////////////
MemoryPoolAllocator::MemoryPoolAllocator (VDRUID unitID)
	 : SharedPhysicalUnit (unitID)
	{
	physicalMemoryPartition = NULL;
	messageDispatcherUnit = NULL;
	}

STFResult MemoryPoolAllocator::Create(uint64 * createParams)
	{
	// We do not take any parameters.
	if (createParams[0] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}

STFResult MemoryPoolAllocator::Connect(uint64 localID, IPhysicalUnit * source)
	{
	// We are only connected to our memory partition.
	switch (localID)
		{
		case 0:
			physicalMemoryPartition = source;
			break;
		case 1:
			messageDispatcherUnit = source;
			break;
		default:
			STFRES_RAISE(STFRES_INVALID_PARAMETERS);
		}

	STFRES_RAISE_OK;
	}

STFResult MemoryPoolAllocator::Initialize(uint64 * depUnitsParams)
	{
	if (!physicalMemoryPartition || !messageDispatcherUnit)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	// We are only connected to our memory partition, and it does not get any extra parameters from us.
	// Note that depUnitsParams[0] is the partition's global unit ID.
	if (depUnitsParams[1] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
//! VirtualMemoryPoolAllocatorBase implementation.
////////////////////////////////////////////////////////////////////

VirtualMemoryPoolAllocatorBase::VirtualMemoryPoolAllocatorBase (MemoryPoolAllocator * physicalUnit)
	: VirtualUnitCollection (physicalUnit, 2), MessageSinkRegistration(NULL)
	{
	this->physicalUnit = physicalUnit;

	memPartInterface = NULL;
	messageDispatcherUnit = NULL;

	totalSize = 0;
	alignmentFactor = 1;
	changeSet = 0;
	poolName = POOLNAME;
	}

VirtualMemoryPoolAllocatorBase::~VirtualMemoryPoolAllocatorBase (void)
	{
	// If we obtained the interface in AllocateChildUnits(), we need to release it now.
	if (memPartInterface != NULL)
		memPartInterface->Release ();
	if (messageDispatcherUnit != NULL)
		messageDispatcherUnit->Release ();
	}



////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocatorBase VirtualUnitCollection implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPoolAllocatorBase::AllocateChildUnits (void)
	{
	IVirtualUnit *virtMemPart;
	IVirtualUnit	*	virtualMessageDispatcher;
	STFMessageDispatcher	* dispatcher;
	STFResult res;

	// We only have a single reference to a virtual memory partition.
	STFRES_REASSERT(physicalUnit->physicalMemoryPartition->CreateVirtual (virtMemPart, this, root != NULL ? root : this));

	if (STFRES_SUCCEEDED(res = AddLeafUnit (virtMemPart)))
		{
		STFRES_REASSERT(virtMemPart->QueryInterface (VDRIID_MEMORYPARTITION, (void *&)memPartInterface));
		STFRES_REASSERT(physicalUnit->messageDispatcherUnit->CreateVirtual(virtualMessageDispatcher, this, root ? root : this));

		if (STFRES_SUCCEEDED(res = AddLeafUnit(virtualMessageDispatcher)))
			{
			STFRES_REASSERT(virtualMessageDispatcher->QueryInterface(VDRIID_MESSAGE_DISPATCHER, (void*&) messageDispatcherUnit));
			STFRES_REASSERT(messageDispatcherUnit->GetDispatcher(dispatcher));
			STFRES_REASSERT(SetDispatcher(dispatcher));

			STFRES_RAISE_OK;
			}
		else
			virtualMessageDispatcher->Release();
		}
	else
		virtMemPart->Release ();


	STFRES_RAISE(res);
	}


 ////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocatorBase partial IVDRBase implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPoolAllocatorBase::QueryInterface (VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT (VDRIID_VDR_MESSAGE_SINK_REGISTRATION, IVDRMessageSinkRegistration);
	VDRQI_END(VirtualUnitCollection);

	STFRES_RAISE_OK;
	}


////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocatorBase partial ITagUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPoolAllocatorBase::InternalConfigureTags (TAG * tags)
	{
	//lint --e{613}
	STFRES_REASSERT(VirtualUnitCollection::InternalConfigureTags (tags));

	PARSE_TAGS_START(tags, changeSet)
		GETSETC(MEMPOOL_TOTALSIZE,				totalSize, 0);
		GETSETC(MEMPOOL_ALIGNMENT_FACTOR,	alignmentFactor, 0);
		GETSETC(MEMPOOL_POOLNAME,				poolName, 0);
	PARSE_TAGS_END

	if (changeSet != 0)
		{
		// A value has changed. Pass the parameters to our virtual memory partition.
		STFRES_REASSERT(memPartInterface->SetAllocationParameters (totalSize, alignmentFactor, poolName));
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualMemoryPoolAllocatorBase::InternalUpdate (void)
	{
	changeSet = 0;

	STFRES_RAISE_OK;
	}


////////////////////////////////////////////////////////////////////
//! VirtualMemoryPoolAllocator implementation.
////////////////////////////////////////////////////////////////////

VirtualMemoryPoolAllocator::VirtualMemoryPoolAllocator (MemoryPoolAllocator * physicalUnit)
	: VirtualMemoryPoolAllocatorBase (physicalUnit)
	{
	blockSize = 0;
	numBlocks = 0;
	clusterSize = 1;
	}

////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocator partial IVDRBase implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPoolAllocator::QueryInterface (VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT (VDRIID_VDR_MEMORYPOOL_ALLOCATOR, IVDRMemoryPoolAllocator);
		VDRQI_IMPLEMENT (VDRIID_VDR_MESSAGE_SINK_REGISTRATION, IVDRMessageSinkRegistration);
	VDRQI_END(VirtualMemoryPoolAllocatorBase);

	STFRES_RAISE_OK;
	}


////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocator partial ITagUnit implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualMemoryPoolAllocator::InternalConfigureTags (TAG * tags)
	{
	STFRES_REASSERT(VirtualMemoryPoolAllocatorBase::InternalConfigureTags (tags));

	PARSE_TAGS_START(tags, changeSet)
		GETSETC(MEMPOOL_BLOCKSIZE,				blockSize, 0);
		GETSETC(MEMPOOL_CLUSTERSIZE,			clusterSize, 1);

		GETONLY(MEMPOOL_NUMBER_OF_BLOCKS,	numBlocks);
	PARSE_TAGS_END

	if (changeSet != 0)
		{
		assert(blockSize!=0);
		// A value has changed. Recalculate blockSize. This is important as each block
		// must fulfill the alignment requirements.
		blockSize = blockSize + alignmentFactor - 1;
		blockSize = blockSize - (blockSize % alignmentFactor);
		numBlocks = totalSize / blockSize;
		if (numBlocks * blockSize != totalSize)
			DP("\nWARNING: Your MEMPOOL_TOTALSIZE %d is too small, numBlocks is only %d, you may have forgotten the alignment!\n\n", totalSize, numBlocks);
		}

	STFRES_RAISE_OK;
	}


////////////////////////////////////////////////////////////////////
//! VirtualVariableSizeMemoryPoolAllocator implementation.
////////////////////////////////////////////////////////////////////

VirtualVariableSizeMemoryPoolAllocator::VirtualVariableSizeMemoryPoolAllocator (MemoryPoolAllocator * physicalUnit)
	: VirtualMemoryPoolAllocatorBase (physicalUnit)
	{
	}


////////////////////////////////////////////////////////////////////
// VirtualMemoryPoolAllocator partial IVDRBase implementation.
////////////////////////////////////////////////////////////////////

STFResult VirtualVariableSizeMemoryPoolAllocator::QueryInterface (VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT (VDRIID_VARIABLE_SIZE_MEMORYPOOL_ALLOCATOR, IVDRVariableSizeMemoryPoolAllocator);
	VDRQI_END(VirtualMemoryPoolAllocatorBase);

	STFRES_RAISE_OK;
	}



	

////////////////////////////////////////////////////////////////////
// MemoryPoolBlock implementation.
////////////////////////////////////////////////////////////////////

MemoryPoolBlock::MemoryPoolBlock (void)
	{
#if REFERENCE_COUNT_DEBUG
	maxLoggingEntries = 96;	// This number was increased from 64 to 96 because of packet
									// replication by InfiniteStreamReplicators with many outputs.
	numLoggingEntries = 0;
	loggingArray = new IVDRDataHolder * [maxLoggingEntries];
	assert (loggingArray != NULL);
#endif
	}



MemoryPoolBlock::~MemoryPoolBlock (void)
	{
#if REFERENCE_COUNT_DEBUG
	delete[] loggingArray;
#endif
	}



#if REFERENCE_COUNT_DEBUG

void MemoryPoolBlock::AddToLogging (IVDRDataHolder * holder)
	{
	STFGlobalLock();

	// Add to the logging array.
	if (numLoggingEntries < maxLoggingEntries)
		{
		loggingArray[numLoggingEntries] = holder;
		numLoggingEntries++;
		}
	else
		{
		// Logging array overflow
		DEBUGLOG(LOGID_ERROR_LOGGING, "MemoryPoolBlock (%08x size %d): Logging array overflow. Holder:\n", GetPhysicalAddress(), GetSize());

		if (holder == NULL)
			DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
		else
			holder->PrintDebugInfo (LOGID_ERROR_LOGGING);

		DEBUGLOG(LOGID_ERROR_LOGGING, "Other current holders:\n");
		for (uint32 i = 0; i < numLoggingEntries; i++)
			{
			IVDRDataHolder *prev;
			prev = loggingArray[i];

			if (prev == NULL)
				DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
			else if ((uint32)prev < 32)
				{
				DEBUGLOG(LOGID_ERROR_LOGGING, "%08x\n", prev);   // special marker which is not a pointer
				}
			else
				prev->PrintDebugInfo (LOGID_ERROR_LOGGING);
			}
		}

	STFGlobalUnlock();
	}

void MemoryPoolBlock::RemoveFromLogging (IVDRDataHolder * holder)
	{
	uint32 i;

	STFGlobalLock();

	for (i = 0; i < numLoggingEntries; i++)
		{
		if (holder == loggingArray[i])
			{
			// Found a holder. Remove it from the array.
			loggingArray[i] = loggingArray[numLoggingEntries-1];
			numLoggingEntries--;
			STFGlobalUnlock();

			return;
			}
		}

	// The holder was not found in the array.
	DEBUGLOG(LOGID_ERROR_LOGGING, "MemoryPoolBlock (%08x size %d): Holder not logged:\n", GetPhysicalAddress(), GetSize());
	if (holder == NULL)
		DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
	else if ((uint32)holder < 32)
		{
		DEBUGLOG(LOGID_ERROR_LOGGING, "%08x\n", holder);   // special marker which is not a pointer
		}
	else
		holder->PrintDebugInfo (LOGID_ERROR_LOGGING);

	DEBUGLOG(LOGID_ERROR_LOGGING, "Other current holders:\n");
	for (i = 0; i < numLoggingEntries; i++)
		{
		IVDRDataHolder *prev;
		prev = loggingArray[i];

		if (prev == NULL)
			DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
		else if ((uint32)prev < 32)
			{
			DEBUGLOG(LOGID_ERROR_LOGGING, "%08x\n", prev);   // special marker which is not a pointer
			}
		else
			prev->PrintDebugInfo (LOGID_ERROR_LOGGING);
		}

	STFGlobalUnlock();
	}

void MemoryPoolBlock::DumpLogging (void)
	{
	uint32 i;

	STFGlobalLock();

	if (numLoggingEntries == 0)
		{
		DEBUGLOG(LOGID_BLOCK_LOGGING, "MemoryPoolBlock (%08x size %d): Holder: --none--\n", GetPhysicalAddress(), GetSize());
		}
	else
		{
		DEBUGLOG(LOGID_BLOCK_LOGGING, "MemoryPoolBlock (%08x size %d): Holder:\n", GetPhysicalAddress(), GetSize());

		for (i = 0; i < numLoggingEntries; i++)
			{
			IVDRDataHolder *holder;
			holder = loggingArray[i];

			if (holder == NULL)
				DEBUGLOG(LOGID_BLOCK_LOGGING, "NULL\n");
			else if ((uint32)holder < 32)
				{
				DEBUGLOG(LOGID_BLOCK_LOGGING, "%08x\n", holder);   // special marker which is not a pointer
				}
			else
				holder->PrintDebugInfo (LOGID_BLOCK_LOGGING);
			}

		DEBUGLOG(LOGID_BLOCK_LOGGING, "\n");
		}

	STFGlobalUnlock();
	}

#endif



uint32 MemoryPoolBlock::AddRef (IVDRDataHolder * holder)
	{
	assert(counter > 0);

#if REFERENCE_COUNT_DEBUG
	AddToLogging (holder);
#endif
	return ++counter;
	}



uint32 MemoryPoolBlock::Release (IVDRDataHolder * holder)
	{
#if REFERENCE_COUNT_DEBUG
	RemoveFromLogging (holder);
#endif

	VDRMemoryBlock *block;
	int32 result = --counter;
	if (0 == result)
		{
		block = this;
		pool->ReturnMemoryBlocks (&block);
		}
	else
		{
		assert (result > 0);   // if this failed, there were too many Release() calls
		}

	return (uint32) result;
	}



void MemoryPoolBlock::SetInitialCounter (IVDRDataHolder * holder)
	{
#if REFERENCE_COUNT_DEBUG
	AddToLogging (holder);
#endif
	counter = 1;
	}


