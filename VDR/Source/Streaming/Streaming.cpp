
//
// PURPOSE:		VDR Internal Streaming Functions
//

/*! \file
	\brief VDR Internal Streaming Functions
*/

#include <string.h>
#include "IStreaming.h"
#include <string.h> // for memcpy
#include <assert.h>

#include "STF/Interface/STFDebug.h"
#include <assert.h>

#ifndef CONFIG_DEBUG_REFERENCE_COUNTING
#define CONFIG_DEBUG_REFERENCE_COUNTING 0
#endif

#define DEBUG_PACKET_HOLDERS (_DEBUG && CONFIG_DEBUG_REFERENCE_COUNTING)


///////////////////////////////////////////////////////////////////////////////
// Streaming Data Packet
///////////////////////////////////////////////////////////////////////////////

StreamingDataPacket::StreamingDataPacket(IStreamingDataPacketManager * originator)
	{
	this->originator = originator;
	vdrPacket.size = sizeof(VDRStreamingDataPacket);

#if _DEBUG
	// Signal that packet is in the "released" state
	vdrPacket.numRanges = 255;
	maxLoggingEntries = 8;
	numLoggingEntries = 0;
	loggingArray = new IVDRDataHolder * [maxLoggingEntries];
	assert (loggingArray != NULL);
#endif
	}

StreamingDataPacket::~StreamingDataPacket (void)
	{
#if _DEBUG
	delete[] loggingArray;
#endif
	}

//! Release the packet to return it to its originator
STFResult StreamingDataPacket::ReturnToOrigin(void)
	{
#if DEBUG_PACKET_HOLDERS
	if (numLoggingEntries)
		{
		DEBUGLOG(LOGID_ERROR_LOGGING, "Return to origin with ownership set\n");
		this->PrintDebugInfo(LOGID_ERROR_LOGGING);
		}
	assert(numLoggingEntries == 0);
	// Signal that packet is in the "released" state
	vdrPacket.numRanges = 255;
#endif

	STFRES_RAISE(originator->ReturnDataPacket(this));
	}


//! Add a reference to the contained Data Ranges
void StreamingDataPacket::AddRefToRanges(void)
	{
#if DEBUG_PACKET_HOLDERS
	// Check if the packet is not in the "released" state
	if (vdrPacket.numRanges == 255)
		{
		// Error! Someone tries to add references to a released packet!
		DEBUGLOG(LOGID_ERROR_LOGGING, "StreamingDataPacket::AddRefToRanges - Accessing released packet! Originator:\n");
		originator->PrintDebugInfo(LOGID_ERROR_LOGGING);
		}

	assert(vdrPacket.numRanges != 255);
#endif

	for (int i = 0; i < vdrPacket.numRanges; i++)
		{
		vdrPacket.tagRanges.ranges[vdrPacket.numTags + i].AddRef(this);
		}
	}


//! Release reference to the contained Data Ranges
void StreamingDataPacket::ReleaseRanges(void)
	{
#if DEBUG_PACKET_HOLDERS
	// Check if the packet is not in the "released" state
	if (vdrPacket.numRanges == 255)
		{
		// Error! Someone tries to add references to a released packet!
		DEBUGLOG(LOGID_ERROR_LOGGING, "StreamingDataPacket::ReleaseRanges - Accessing released packet! Originator:\n");
		originator->PrintDebugInfo(LOGID_ERROR_LOGGING);
		}

	assert(vdrPacket.numRanges != 255);
#endif

	for (int i = 0; i < vdrPacket.numRanges; i++)
		{
		vdrPacket.tagRanges.ranges[vdrPacket.numTags + i].Release(this);
		}
	}


void StreamingDataPacket::TransferRangesOwnership(IVDRDataHolder * holder)
	{
	for (int i = 0; i < vdrPacket.numRanges; i++)
		{
		vdrPacket.tagRanges.ranges[vdrPacket.numTags + i].AddRef(holder);
		}

	ReleaseRanges();
	}


void StreamingDataPacket::CopyFromVDRPacket(const VDRStreamingDataPacket * packet)
	{
	uint32 packetSize;
	
	packetSize = min(packet->size, sizeof(VDRStreamingDataPacket));
	memcpy(&vdrPacket, packet, packetSize);
	vdrPacket.size = packetSize;
	}


void StreamingDataPacket::CopyToVDRPacket(VDRStreamingDataPacket * packet)
	{
	uint32 packetSize;
	
	packetSize = min(packet->size, sizeof(VDRStreamingDataPacket));
	memcpy(packet, &vdrPacket, packetSize);
	packet->size = packetSize;
	}

uint32 StreamingDataPacket::GetPacketDataSize(void)
	{
	uint32	i, size;

	size = 0;
	for(i=0; i<vdrPacket.numRanges; i++)
		size += vdrPacket.tagRanges.ranges[vdrPacket.numTags + i].size;

	return size;
	}

#if _DEBUG

void StreamingDataPacket::AddToLogging (IVDRDataHolder * holder)
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
  		DEBUGLOG(LOGID_ERROR_LOGGING, "StreamingDataPacket::AddToLogging() : Logging array overflow. Holder:\n");

		if (holder == NULL)
	  		DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
		else
			{
			holder->PrintDebugInfo (LOGID_ERROR_LOGGING);
			DEBUGLOG(LOGID_ERROR_LOGGING, "\n");
			}

		DEBUGLOG(LOGID_ERROR_LOGGING, "Other current holders:\n");
		for (uint32 i = 0; i < numLoggingEntries; i++)
			{
			if (loggingArray[i] == NULL)
		  		DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
			else
				loggingArray[i]->PrintDebugInfo (LOGID_ERROR_LOGGING);
			}
		}

	STFGlobalUnlock();
	}

void StreamingDataPacket::RemoveFromLogging (IVDRDataHolder * holder)
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
  	DEBUGLOG(LOGID_ERROR_LOGGING, "StreamingDataPacket::RemoveFromLogging() : Holder not logged:\n");
	if (holder == NULL)
		DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
	else
		{
		holder->PrintDebugInfo (LOGID_ERROR_LOGGING);
		DEBUGLOG(LOGID_ERROR_LOGGING, "\n");
		}

	DEBUGLOG(LOGID_ERROR_LOGGING, "Other current holders:\n");
	for (i = 0; i < numLoggingEntries; i++)
		{
		if (loggingArray[i] == NULL)
		  	DEBUGLOG(LOGID_ERROR_LOGGING, "NULL\n");
		else
			loggingArray[i]->PrintDebugInfo (LOGID_ERROR_LOGGING);
		}
	
	STFGlobalUnlock();
	}

#endif

#if _DEBUG

STFResult StreamingDataPacket::PrintDebugInfo (uint32 id)
	{		
	STFGlobalLock();

	DEBUGLOG(id, "StreamingDataPacket %08x of unit: ", this);

	originator->PrintDebugInfo(id);

#if DEBUG_PACKET_HOLDERS
	for (uint32 i = 0; i < numLoggingEntries; i++)
		{
		DEBUGLOG(id, "....");
		if (loggingArray[i])
			loggingArray[i]->PrintDebugInfo(id);
		else
			DEBUGLOG(id, "NULL\n");
		}
#endif

	STFGlobalUnlock();

	STFRES_RAISE_OK;
	}

#endif

void StreamingDataPacket::AddPacketOwner(IVDRDataHolder * holder)
	{
#if DEBUG_PACKET_HOLDERS
	this->AddToLogging(holder);
#endif
	}

void StreamingDataPacket::RemPacketOwner(IVDRDataHolder * holder)
	{
#if DEBUG_PACKET_HOLDERS
	this->RemoveFromLogging(holder);
#endif
	}
