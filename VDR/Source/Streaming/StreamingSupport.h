#ifndef STREAMINGSUPPORT_H
#define STREAMINGSUPPORT_H

///
/// @file       VDR/Source/Streaming/StreamingSupport.h
///
/// @brief      Classes to support Streaming
///
/// @author     Ulrich Sigmund, S. Herr
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Header File
///
/// @date       2003-10-21
///			 
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "VDR/Source/Streaming/StreamingFormatter.h"

#if _DEBUG
#include <stdio.h>
#endif


///////////////////////////////////////////////////////////////////////////////
// Data Range Stream Queue Helper class
///////////////////////////////////////////////////////////////////////////////

class DataRangeStreamQueue
	{
	protected:
		VDRDataRange	*	ranges;
		uint32				sentRanges, numRanges, maxRanges;
		uint32				size;
	public:
		DataRangeStreamQueue(uint32 maxRanges);
		~DataRangeStreamQueue(void);

		STFResult AppendRange(const VDRDataRange & range, IVDRDataHolder * holder = NULL);
		STFResult AppendSubRange(const VDRDataRange & range, uint32 offset, uint32 size, IVDRDataHolder * holder = NULL);
		STFResult AppendRangeQueue(DataRangeStreamQueue & queue);
		STFResult FlushRanges(IVDRDataHolder * holder = NULL);
		STFResult SendRanges(StreamingFormatter * formatter, IVDRDataHolder * holder = NULL);
#if _DEBUG
		STFResult SendRangesDebugDump(StreamingFormatter * formatter, IVDRDataHolder * holder, FILE * file);
#endif

		STFResult DropBytes(uint32 num, uint32 & done,  IVDRDataHolder * holder = NULL);
		STFResult LimitBytes(uint32 num, IVDRDataHolder * holder = NULL);
		STFResult SkipBytes(uint32 offset, uint32 num, IVDRDataHolder * holder = NULL);
 
		uint32 Size(void) {return size;}

		uint8 & operator[](uint32 at);
	};


class TAGStreamQueue
	{
	protected:
		TAG				*	tqueue, * tcurrent;
		uint32				first, last, size, mask;
	public:
		TAGStreamQueue(uint32 size);
		~TAGStreamQueue(void);

		STFResult FlushTags(void);

		STFResult AppendTags(TAG * tagList);
		STFResult GetTags(TAG *& tagList);
		STFResult ReleaseTags(TAG * tagList);
	};

struct StreamProperty
	{
	// Flags which should be delivered to the packet 
	uint32 propertyType;
	
	STFHiPrec64BitTime     startOrEndTime;               // Start or end time
	STFHiPrec32BitDuration skipOrCutDurationDuration;    // The scip / cut duration 

	bool                   requestNotification;          // Requeste a notification when the command is completed
	bool                   singleUnitGroup;              // This flag indicates a single unit group (group start)
	uint16                 groupOrSegmentNumber;         // Group or segment number (segment start, segment end, group 
	                                                     // start, group end)
	                                                     // holds the group or segment number (segment start, segment end, 
	                                                     // group start, group end)

	uint32                 inputBytePosition;            // the number of the byte these properties apply
	TAG                    tag;                          // if this property is a tag property, this holds the tag value

	void *                 curOutputRangeProperties;     // Additional user data
	};

class PendingPropertiesQueue
	{
	private:
		uint32 maxStreamProperties;               // Maximum number of pedning stream properties
		uint32 pendingPropertiesQueueHead;        // index of the next UNUSED (empty) element
		uint32 pendingPropertiesQueueTail;        // index of last filled element
		StreamProperty * pendingStreamProperties;

	public:
		STFResult InitializePendingPropertiesQueue(uint32 size);    
		STFResult Cleanup();

		STFResult FlushPendingProperties();

		STFResult GetNextFreePropertyEntry(StreamProperty * & currentProperty, uint32 position);
		STFResult GetNextPropertyForBytePosition(uint32 bytePosition, StreamProperty * & property);
		STFResult RemoveFirstPendingProperty();

		STFResult PutBeginGroup(uint32 bytePosition, uint16 groupNumber, bool sendNotification, bool singleUnitGroup);
		STFResult PutEndGroup(uint32 bytePosition, uint16 groupNumber, bool requestNotification);
		STFResult PutBeginSegment(uint32 bytePosition, uint16 segmentNumber, bool sendNotification);
		STFResult PutEndSegment(uint32 bytePosition, uint16 segmentNumber, bool requestNotification);
		STFResult PutTag(uint32 bytePosition, const TAG & tag);
		STFResult PutTimeDiscontinuity(uint32 bytePosition);
		STFResult PutDataDiscontinuity(uint32 bytePosition);

		STFResult ApplyPendingProperty(uint32 bytePosition, StreamingFormatter & outputFormatter);
	};

#endif	// #ifndef STREAMINGSUPPORT_H

