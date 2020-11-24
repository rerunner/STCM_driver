///
/// @brief Generic debug methods
///

#include "STF/Interface/STFDebug.h"
#include "STF/Interface/STFSynchronisation.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>



#define DEBUGLOG_MAX_LENGTH	256

#define USE_LOGBUFFER	0
#define TEST_LOGBUFFER	0



char *DebugLogBuffer;
uint32 DebugLogBufferSize;

static uint32 DebugLogFreeBytes;
static uint32 DebugLogAddIndex;			// where stuff is added; always < DebugLogBufferSize
static uint32 DebugLogRemoveIndex;		// where stuff is removed/drained; always < DebugLogBufferSize
static uint32 DebugLogPendingLockSize;

#if USE_LOGBUFFER
static bool DebugLogOverflowMode;
static char DebugLogOverflowString[] = "*** LOG OVERFLOW ***\n";
#endif



// The filtering mechanism is implemented as an open chain hash table.
struct ChannelEntry
	{
	ChannelEntry	*next;
	uint32			id;
	char				*description;
	bool				enabled;
	};

#define CHANNEL_HASH_VALUE		59		// a prime number

static ChannelEntry *ChannelTable[CHANNEL_HASH_VALUE] = { NULL };
static volatile uint32 NumChannels = 0;



class ExternalChannelInfo : public STFDebugLogChannelInfo
	{
	private:
		uint32			id;
		char				*description;
		bool				enabled;

	public:
		// Constructor/destructor.
		ExternalChannelInfo (void);
		virtual ~ExternalChannelInfo (void);

		void Set (uint32 id, bool enabled, char * description);

	protected:
		// STFNode override implementation.
		virtual bool HigherPriorityThan (STFNode * n);

	public:
		// STFDebugLogChannelInfo implementation.
		virtual STFResult GetInfo (uint32 & id, bool & enabled, char *& description);
	};

class ExternalChannelList : public STFList
	{
	public:
		// Constructor/destructor.
		virtual ~ExternalChannelList (void);
	};



STFResult InitializeSTFDebugLog (uint32 bufferSize)
	{
#if TEST_LOGBUFFER
	static char someString[] = "A band of puppies, six weeks old, their noses nudged me, wet and cold";
	bufferSize = 100;
#endif

	// Allocate the buffer.
	DebugLogBuffer = new char[bufferSize];
	STFRES_ASSERT(DebugLogBuffer != NULL, STFRES_NOT_ENOUGH_MEMORY);

	// Initialize the working pointers.
	DebugLogBufferSize = bufferSize;
	DebugLogFreeBytes = bufferSize;
	DebugLogAddIndex = DebugLogRemoveIndex = 0;
#if USE_LOGBUFFER
	DebugLogOverflowMode = false;
#endif
#if TEST_LOGBUFFER
	// Test the logging buffer behaviour.
	char *p;
	uint32 lockedBytes;
	STFResult res;

	// Access 1 must work.
	DebugLog (0, someString);
	// Access 2 should enter overflow mode.
	DebugLog (0, someString);

	// Remove some parts.
	res = STFDebugLogManager::LockNextBufferChunk (80, p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));
	res = STFDebugLogManager::UnlockBufferChunk (p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));

	// Access 3 should have a wraparound at the array end and leave overflow mode.
	DebugLog (0, someString);

	// Remove some parts. lockedBytes should be less than 80 now.
	res = STFDebugLogManager::LockNextBufferChunk (80, p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));
	assert (lockedBytes < 80);
	res = STFDebugLogManager::UnlockBufferChunk (p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));

	// Access 4 should enter overflow mode.
	DebugLog (0, someString);

	// Remove some parts.
	res = STFDebugLogManager::LockNextBufferChunk (80, p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));
	res = STFDebugLogManager::UnlockBufferChunk (p, lockedBytes);
	assert (STFRES_SUCCEEDED(res));

	// Access 5 must work.
	DebugLog (0, someString);
#endif

	STFRES_RAISE_OK;
	}



STFResult STFDebugLogManager::RegisterChannel (uint32 id, bool enabled, const char * description)
	{
	uint32 i = id % CHANNEL_HASH_VALUE;
	ChannelEntry *channel, *newChannel;
	STFResult res = STFRES_OK;

	if (id == LOGID_ERROR_LOGGING)   // this channel is always enabled
		enabled = true;

	newChannel = new ChannelEntry;

	STFGlobalLock ();

	// Check if the channel is already registered.
	channel = ChannelTable[i];
	while (channel != NULL  &&  channel->id != id)
		channel = channel->next;

	if (channel != NULL)
		res = STFRES_OBJECT_EXISTS;
	else if (newChannel == NULL)
		res = STFRES_NOT_ENOUGH_MEMORY;
	else
		{
		newChannel->next = ChannelTable[i];
		newChannel->id = id;
		newChannel->description = (char *)description;
		newChannel->enabled = enabled;
		ChannelTable[i] = newChannel;
		NumChannels++;
		}

	STFGlobalUnlock ();

	if (STFRES_FAILED(res))
		delete newChannel;   // may be NULL

	STFRES_RAISE(res);
	}



STFResult STFDebugLogManager::EnableChannel (uint32 id, bool enabled)
	{
	uint32 i = id % CHANNEL_HASH_VALUE;
	ChannelEntry *channel;
	STFResult res = STFRES_OK;

	if (id == LOGID_ERROR_LOGGING)   // this channel is always enabled
		enabled = true;

	STFGlobalLock ();

	// Find the channel.
	channel = ChannelTable[i];
	while (channel != NULL  &&  channel->id != id)
		channel = channel->next;

	if (channel == NULL)
		res = STFRES_OBJECT_NOT_FOUND;
	else
		channel->enabled = enabled;

	STFGlobalUnlock ();

	STFRES_RAISE(res);
	}



STFResult STFDebugLogManager::IsChannelEnabled (uint32 id)
	{
	uint32 i = id % CHANNEL_HASH_VALUE;
	ChannelEntry *channel;
	STFResult res = STFRES_OBJECT_NOT_FOUND;

	STFGlobalLock ();

	// Find the channel.
	channel = ChannelTable[i];
	while (channel != NULL  &&  channel->id != id)
		channel = channel->next;

	if (channel != NULL)
		res = channel->enabled ? STFRES_TRUE : STFRES_FALSE;

	STFGlobalUnlock ();

	STFRES_RAISE(res);
	}



STFResult STFDebugLogManager::SetAllChannels (bool enabled)
	{
	uint32 i;
	ChannelEntry *channel;

	STFGlobalLock ();

	for (i = 0;  i < CHANNEL_HASH_VALUE;  i++)
		{
		channel = ChannelTable[i];
		while (channel != NULL)
			{
			// Note that some channels are always enabled.
			if (channel->id != LOGID_ERROR_LOGGING)
				channel->enabled = enabled;
			channel = channel->next;
			}
		}

	STFGlobalUnlock ();

	STFRES_RAISE_OK;
	}



STFResult STFDebugLogManager::CreateChannelList (STFList *& list)
	{
	uint32 i;
	ExternalChannelList channelList, freeList;
	ExternalChannelList *sortedList;
	ExternalChannelInfo *info;
	ChannelEntry *srcChannel;
	STFResult res = STFRES_OK;

	// We don't like expensive heap operations during STFGlobalLock(), so we pre-allocate as much as possible.
	sortedList = new ExternalChannelList;
	STFRES_ASSERT(sortedList != NULL, STFRES_NOT_ENOUGH_MEMORY);
	for (i = 0;  i < NumChannels;  i++)
		{
		info = new ExternalChannelInfo;
		if (info != NULL)
			freeList.InsertFirst (info);
		}

	STFGlobalLock ();

	// Make a copy of all hash table entries and put them into a list.
	for (i = 0;  i < CHANNEL_HASH_VALUE  &&  STFRES_SUCCEEDED(res);  i++)
		{
		srcChannel = ChannelTable[i];
		while (srcChannel != NULL  &&  STFRES_SUCCEEDED(res))
			{
			info = (ExternalChannelInfo *) freeList.Dequeue ();
			if (info == NULL)
				info = new ExternalChannelInfo;   // pool was exhausted, so heap must be used

			if (info != NULL)
				{
				info->Set (srcChannel->id, srcChannel->enabled, srcChannel->description);
				channelList.InsertFirst (info);
				}
			else
				res = STFRES_NOT_ENOUGH_MEMORY;

			srcChannel = srcChannel->next;
			}
		}

	STFGlobalUnlock ();

	if (STFRES_SUCCEEDED(res))
		{
		// In the second step, we sort the list by ID because we have more time outside of STFGlobalLock().

		//??? This results in an O(n*n) algorithm, but there are not so many channels,
		// and usually this is called in interaction with the user, so we have some time.
		while (NULL != (info = (ExternalChannelInfo *)channelList.Dequeue ()))
			sortedList->InsertByPriorityFromEnd (info);

		list = sortedList;
		}
	else
		delete sortedList;   // cleanup; free everything again

	STFRES_RAISE(res);
	}



STFResult STFDebugLogManager::WriteBufferContent (char *filename)
	{
	//??? not yet implemented
	STFRES_RAISE_OK;
	}



STFResult STFDebugLogManager::LockNextBufferChunk (uint32 maxBytes, char *& data, uint32 & numBytes)
	{
	uint32 availSize;

	STFGlobalLock ();

	if (DebugLogAddIndex >= DebugLogRemoveIndex)
		{
		availSize = DebugLogAddIndex - DebugLogRemoveIndex;
		if (availSize == 0  &&  DebugLogFreeBytes == 0)
			availSize = DebugLogBufferSize;   // this is "completely full", distinguish it from "completely empty"
		}
	else
		{
		// DebugLogAddIndex < DebugLogRemoveIndex.
		// We don't support auto-wraparound because the out parameters are a monolithic block.
		availSize = DebugLogBufferSize - DebugLogRemoveIndex;
		assert (availSize > 0);
		}
	data = &DebugLogBuffer[DebugLogRemoveIndex];
	numBytes = min (maxBytes, availSize);
	DebugLogPendingLockSize = numBytes;

	STFGlobalUnlock ();

	STFRES_RAISE(numBytes > 0 ? STFRES_OK : STFRES_OBJECT_EMPTY);
	}



STFResult STFDebugLogManager::UnlockBufferChunk (char * data, uint32 numBytes)
	{
	STFResult res = STFRES_OK;

	STFGlobalLock ();

	if (data == &DebugLogBuffer[DebugLogRemoveIndex]  &&  numBytes <= DebugLogPendingLockSize)
		{
		DebugLogFreeBytes += numBytes;
		DebugLogRemoveIndex += numBytes;
		if (DebugLogRemoveIndex >= DebugLogBufferSize)
			DebugLogRemoveIndex = 0;
		}
	else
		res = STFRES_INVALID_PARAMETERS;

	STFGlobalUnlock ();

	assert (STFRES_SUCCEEDED(res));   // when this assertion fails, it was illegal chunk
	STFRES_RAISE(res);
	}



// This function allows to redirect logging output depending on the "id" parameter.

void DebugLog (uint32 id, const char  * szFormat, ...)
	{
	char string[DEBUGLOG_MAX_LENGTH];
	va_list marker;

	va_start (marker, szFormat);
	vsprintf (string, szFormat, marker);

#if ! USE_LOGBUFFER
	// Old compatibility mode. Should be finally removed.
	switch (id)
		{
		case LOGID_ERROR_LOGGING:
			DP (string);
			break;
		case LOGID_BLOCK_LOGGING:
			DPR (string);
			break;
		default:
			DP (string);
			break;
		}
#else // USE_LOGBUFFER
	uint32 stringLength, overflowLength;
	uint32 part1Index, part1Size, part2Size;
	uint32 index = id % CHANNEL_HASH_VALUE;
	ChannelEntry *channel;

	part1Size = 0;
	part2Size = 0;
	stringLength = strlen(string);

	// We always leave also extra space for the overflow message.
	overflowLength = stringLength + sizeof(DebugLogOverflowString) - 1;

	STFGlobalLock ();

	part1Index = DebugLogAddIndex;   // remember old position

	// Find the channel.
	channel = ChannelTable[index];
	while (channel != NULL  &&  channel->id != id)
		channel = channel->next;

	// Enabled channels and unknown channels are logged.
	if (channel == NULL  ||  channel->enabled)
		{
		if (overflowLength > DebugLogFreeBytes)
			{
			// Overflow. There is not enough space for the string and the overflow string.
			if (! DebugLogOverflowMode)
				{
				// We store the overflow message.
				DebugLogOverflowMode = true;
				memcpy (string, DebugLogOverflowString, sizeof(DebugLogOverflowString));
				stringLength = sizeof(DebugLogOverflowString) - 1;
				}
			else
				stringLength = 0;   // nothing to do, last string already was the overflow message
			}
		else
			DebugLogOverflowMode = false;

		// At this point it is ensured that the string will fit (or it is empty).

		if (DebugLogAddIndex >= DebugLogRemoveIndex)
			{
			if (stringLength < DebugLogBufferSize - DebugLogAddIndex)
				{
				// There is at least one byte more available at the end.
				part1Size = stringLength;
				DebugLogAddIndex += part1Size;
				}
			else
				{
				// Split the string into two parts.
				part1Size = DebugLogBufferSize - DebugLogAddIndex;
				part2Size = stringLength - part1Size;
				DebugLogAddIndex = part2Size;
				}
			}
		else
			{
			// DebugLogAddIndex < DebugLogRemoveIndex.
			part1Size = stringLength;
			DebugLogAddIndex += stringLength;
			}

		DebugLogFreeBytes -= part1Size + part2Size;
		}

	STFGlobalUnlock ();

	if (part1Size > 0)
		{
		memcpy (DebugLogBuffer + part1Index, string, part1Size);
		}
	if (part2Size > 0)
		{
		memcpy (DebugLogBuffer, string + part1Size, part2Size);
		}

#endif // of USE_LOGBUFFER

	va_end (marker);
	}



////////////////////////////////////////////////////////////////////
// Implementation of the ExternalChannelInfo class.
////////////////////////////////////////////////////////////////////

ExternalChannelInfo::ExternalChannelInfo (void)
	{
	}

ExternalChannelInfo::~ExternalChannelInfo (void)
	{
	}

void ExternalChannelInfo::Set (uint32 id, bool enabled, char * description)
	{
	this->id = id;
	this->description = description;
	this->enabled = enabled;
	}

// STFNode override implementation.
bool ExternalChannelInfo::HigherPriorityThan (STFNode * n)
	{
	ExternalChannelInfo *info = (ExternalChannelInfo *)n;
	return id > info->id;
	}

// STFDebugLogChannelInfo implementation.
STFResult ExternalChannelInfo::GetInfo (uint32 & id, bool & enabled, char *& description)
	{
	id = this->id;
	description = this->description;
	enabled = this->enabled;
	STFRES_RAISE_OK;
	}



////////////////////////////////////////////////////////////////////
// Implementation of the ExternalChannelList class.
////////////////////////////////////////////////////////////////////

ExternalChannelList::~ExternalChannelList (void)
	{
	ExternalChannelInfo *info;

	// We delete all infos in this list.
	while (NULL != (info = (ExternalChannelInfo *)Dequeue ()))
		delete info;
	}
