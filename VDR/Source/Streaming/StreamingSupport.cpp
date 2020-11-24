///
/// @file       VDR/Source/Streaming/StreamingSupport.cpp
///
/// @brief      Classes to support Streaming
///
/// @author     Ulrich Sigmund, S. Herr
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @par SCOPE: INTERNAL Implementation File
///																																																 
/// @date       2003-10-21
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#include "StreamingSupport.h"

///////////////////////////////////////////////////////////////////////////////
// Data Range Stream Queue Helper class
///////////////////////////////////////////////////////////////////////////////

DataRangeStreamQueue::DataRangeStreamQueue(uint32 maxRanges)
	{
	this->maxRanges = maxRanges;
	numRanges = 0;
	sentRanges = 0;
	size = 0;
	ranges = new VDRDataRange[maxRanges];
	}

DataRangeStreamQueue::~DataRangeStreamQueue(void)
	{
	delete[] ranges;
	}


STFResult DataRangeStreamQueue::AppendRange(const VDRDataRange & range, IVDRDataHolder * holder)
	{
	if (numRanges < maxRanges)
		{
		if (range.size)
			{
			ranges[numRanges] = range;
			ranges[numRanges].AddRef(holder);
			size += range.size;
			numRanges++;
			}

		STFRES_RAISE_OK;
		}
	else
		{
		DPR("OBJ_FUL In StrmQueue:AppRang\n");
#if _DEBUG
		holder->PrintDebugInfo();
#endif
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}

STFResult DataRangeStreamQueue::AppendSubRange(const VDRDataRange & range, uint32 offset, uint32 size, IVDRDataHolder * holder)
	{
	if (numRanges < maxRanges)
		{
		if (range.size)
			{
			ranges[numRanges] = VDRSubDataRange(range, offset, size);
			ranges[numRanges].AddRef(holder);
			this->size += size;
			numRanges++;
			}

		STFRES_RAISE_OK;
		}
	else
		{
		DPR("OBJ_FUL In StrmQueue::AppSubRange\n");
#if _DEBUG
		holder->PrintDebugInfo();
#endif
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}

STFResult DataRangeStreamQueue::AppendRangeQueue(DataRangeStreamQueue & queue)
	{
	uint32	i;

	if (numRanges + queue.numRanges <= maxRanges)
		{
		for(i=0; i<queue.numRanges; i++)
			{
			size += queue.ranges[i].size;
			ranges[numRanges++] = queue.ranges[i];
			}

		queue.numRanges = 0;
		queue.size = 0;
		queue.sentRanges = 0;

		STFRES_RAISE_OK;
		}
	else
		{
		DPR("OBJ_FUL In StrmQueue::AppendRangeQueue\n");
		STFRES_RAISE(STFRES_OBJECT_FULL);
		}
	}

STFResult DataRangeStreamQueue::FlushRanges(IVDRDataHolder * holder)
	{
	uint32	i;

	for(i = sentRanges; i < numRanges; i++)
		ranges[i].Release(holder);

	numRanges = 0;
	sentRanges = 0;
	size = 0;

	STFRES_RAISE_OK;
	}


STFResult DataRangeStreamQueue::SendRanges(StreamingFormatter * formatter, IVDRDataHolder * holder)
	{
	while (sentRanges < numRanges)
		{
		STFRES_REASSERT(formatter->PutRange(ranges[sentRanges]));
		ranges[sentRanges].Release(holder);

		sentRanges++;
		}

	numRanges = 0;
	sentRanges = 0;
	size = 0;

	STFRES_RAISE_OK;
	}


#if _DEBUG
STFResult DataRangeStreamQueue::SendRangesDebugDump(StreamingFormatter * formatter, IVDRDataHolder * holder, FILE * file)
	{
	while (sentRanges < numRanges)
		{
		STFRES_REASSERT(formatter->PutRange(ranges[sentRanges]));
		if (file != NULL)
			fwrite(ranges[sentRanges].GetStart(), 1, ranges[sentRanges].size, file);
		ranges[sentRanges].Release(holder);

		sentRanges++;
		}

	numRanges = 0;
	sentRanges = 0;
	size = 0;

	STFRES_RAISE_OK;
	}
#endif

STFResult DataRangeStreamQueue::DropBytes(uint32 num, uint32 & done, IVDRDataHolder * holder)
	{
	uint32	i;

	done = 0;

	while (numRanges && num)
		{
		if (ranges[0].size > num)
			{
			ranges[0].offset += num;
			ranges[0].size -= num;

			done += num;
			num = 0;
			}
		else
			{
			num -= ranges[0].size;
			done += ranges[0].size;

			ranges[0].Release(holder);

			numRanges--;
			for(i=0; i<numRanges; i++)
				ranges[i] = ranges[i+1];
			}
		}

	size -= done;

	STFRES_RAISE_OK;
	}


STFResult DataRangeStreamQueue::LimitBytes(uint32 num, IVDRDataHolder * holder)
	{
	if (num < size)
		{
		num = size - num;
		size -= num;

		while (numRanges && num)
			{
			if (ranges[numRanges-1].size > num)
				{
				ranges[numRanges-1].size -= num;
				num = 0;
				}
			else
				{
				num -= ranges[numRanges-1].size;
				ranges[numRanges-1].Release(holder);
				numRanges--;
				}
			}
		}

	STFRES_RAISE_OK;
	}

STFResult DataRangeStreamQueue::SkipBytes(uint32 offset, uint32 num, IVDRDataHolder * holder)
	{
	uint32	done;
	uint32	firstRange, lastRange, i;

	if (num > 0)
		{
		if (offset == 0)
			{
			STFRES_RAISE(DropBytes(num, done, holder));
			}
		else if (offset + num >= size)
			{
			STFRES_RAISE(LimitBytes(offset, holder));
			}
		else
			{
			size -= num;

			// Find first range, in which to skip bytes
			firstRange = 0;
			while (offset > ranges[firstRange].size)
				{
				offset -= ranges[firstRange].size;
				firstRange++;
				}

			// Find last range, in which to skip bytes
			lastRange = firstRange;
			num += offset;
			while (num >= ranges[lastRange].size)
				{
				num -= ranges[lastRange].size;
				lastRange++;
				}

			if (firstRange == lastRange)
				{
				// whoopie, we have to skip inside a range, so we need a second one
				// to keep the remainder of the range;
				for(i = numRanges; i > lastRange; i--)
					ranges[i] = ranges[i - 1];

				numRanges++;
				lastRange++;
				ranges[lastRange].AddRef(holder);

				ranges[firstRange].size = offset;
				ranges[lastRange].size = ranges[lastRange].size - num;
				ranges[lastRange].offset += num;
				}
			else
				{
				ranges[firstRange].size = offset;
				ranges[lastRange].size = ranges[lastRange].size - num;
				ranges[lastRange].offset += num;
				if (lastRange != firstRange + 1)
					{
					for(i = firstRange + 1; i < lastRange; i++)
						ranges[i].Release(holder);
					for(i = lastRange; i < numRanges; i++)
						ranges[firstRange + 1 + i - lastRange] = ranges[i];
					numRanges -= lastRange - firstRange - 1;
					}
				}
			}
		}

	STFRES_RAISE_OK;
	}


uint8 & DataRangeStreamQueue::operator[](uint32 at)
	{
	uint32		i;

	assert(at < size);

	i = 0;
	while (at >= ranges[i].size)
		{
		at -= ranges[i].size;
		i++;
		}

	return ranges[i].GetStart()[at];
	}

TAGStreamQueue::TAGStreamQueue(uint32 size)
	{
	this->size = size;

	mask = 1;
	while (mask < size)
		mask *= 2;

	tqueue = new TAG[mask];
	mask -= 1;

	first   = 0;
	last    = 0;

	tcurrent = NULL;
	}

TAGStreamQueue::~TAGStreamQueue(void)
	{
	delete[] tqueue;
	}

STFResult TAGStreamQueue::FlushTags(void)
	{
	first   = 0;
	last    = 0;

	tcurrent = NULL;

	STFRES_RAISE_OK;
	}


STFResult TAGStreamQueue::AppendTags(TAG * tagList)
	{
	uint32		i;
	TAG		*	t;

	if (tagList && tagList[0].id)
		{
		t = tagList;
		i = 1;
		while (t->id)
			{
			i++;
			t += t->skip;
			}

		if (last + i + 1 < first + size)
			{
			if (!tcurrent)
				tcurrent = tqueue + (last & mask);

			t = tagList;
			while (t->id)
				{
				tqueue[last & mask] = *t;
				tqueue[last & mask].skip = ((last + 1) & mask) - (last & mask);
				last++;
				t += t->skip;
				}
			}
		else
			STFRES_RAISE(STFRES_OBJECT_FULL);
		}	

	STFRES_RAISE_OK;
	}

STFResult TAGStreamQueue::GetTags(TAG *& tagList)
	{
	tagList = tcurrent;

	if (tcurrent)
		{
		tqueue[last & mask] = TAGDONE;
		last++;
		tcurrent = NULL;

		STFRES_RAISE_TRUE;
		}
	else
		STFRES_RAISE_FALSE;
	}

STFResult TAGStreamQueue::ReleaseTags(TAG * tagList)
	{
	if (tagList)
		{
		assert(tagList == tqueue + (first & mask));

		do {
			first++;
			} while (tqueue[(first - 1) & mask].id);
		}

	STFRES_RAISE_OK;
	} 

/////////////////////////////////////////////
// Pending properties queue
/////////////////////////////////////////////

STFResult PendingPropertiesQueue::GetNextFreePropertyEntry(StreamProperty * & currentProperty, uint32 bytePosition)
	{
	uint32 currentPropertyIndex = pendingPropertiesQueueHead % maxStreamProperties;
	currentProperty = &pendingStreamProperties[currentPropertyIndex];
	pendingPropertiesQueueHead++;

	currentProperty->propertyType = 0;
	currentProperty->inputBytePosition = bytePosition;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::GetNextPropertyForBytePosition(uint32 bytePosition, StreamProperty * & property)
	{
	ASSERT((int32)(pendingPropertiesQueueHead - pendingPropertiesQueueTail) >= 0);

	if (pendingPropertiesQueueHead != pendingPropertiesQueueTail)
		{
		// we have properties pending
		uint32 currentPropertyIndex = pendingPropertiesQueueTail % maxStreamProperties;
		StreamProperty * firstProperty = &pendingStreamProperties[currentPropertyIndex];
		if ((int32) (firstProperty->inputBytePosition - bytePosition) <= 0)
			{
			property = firstProperty;
			STFRES_RAISE_OK;
			}
		else 
			{
			property = NULL;
			STFRES_RAISE(STFRES_OBJECT_EMPTY);
			}
		}

	property = NULL;
	STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}

STFResult PendingPropertiesQueue::RemoveFirstPendingProperty()
	{
	ASSERT((int32)(pendingPropertiesQueueHead - pendingPropertiesQueueTail) >= 0);
	pendingPropertiesQueueTail++;

	STFRES_RAISE_OK;
	}


STFResult PendingPropertiesQueue::InitializePendingPropertiesQueue(uint32 size)
	{
	ASSERT(size > 0);

	maxStreamProperties = size;
	pendingStreamProperties = new StreamProperty[maxStreamProperties];
	
	STFRES_ASSERT(pendingStreamProperties != NULL, STFRES_NOT_ENOUGH_MEMORY);

	for (uint32 i = 0; i < maxStreamProperties; i++)
		{		
		pendingStreamProperties[i].curOutputRangeProperties = NULL;
		pendingStreamProperties[i].groupOrSegmentNumber = 0;
		pendingStreamProperties[i].inputBytePosition = 0;		
		pendingStreamProperties[i].propertyType = 0;
		pendingStreamProperties[i].skipOrCutDurationDuration = ZERO_HI_PREC_32_BIT_DURATION;
		pendingStreamProperties[i].startOrEndTime = ZERO_HI_PREC_64_BIT_TIME;
		pendingStreamProperties[i].groupOrSegmentNumber = 0xffff;
		pendingStreamProperties[i].singleUnitGroup = false;
		pendingStreamProperties[i].requestNotification = false;
		}

	pendingPropertiesQueueHead = 0;
	pendingPropertiesQueueTail = 0;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::Cleanup()
	{
	pendingPropertiesQueueHead = 0;
	pendingPropertiesQueueTail = 0;
	
	maxStreamProperties = 0;
	
	delete[] pendingStreamProperties;
	pendingStreamProperties = NULL;

	STFRES_RAISE_OK;
	}


STFResult PendingPropertiesQueue::FlushPendingProperties()
	{
	pendingPropertiesQueueHead = 0;
	pendingPropertiesQueueTail = 0;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutBeginGroup(uint32 bytePosition, uint16 groupNumber, bool sendNotification, bool singleUnitGroup)
	{
	StreamProperty * currentStreamProperty = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(currentStreamProperty, bytePosition));

	currentStreamProperty->propertyType = VDR_MSMF_GROUP_START;
	currentStreamProperty->requestNotification = sendNotification;
	currentStreamProperty->singleUnitGroup = singleUnitGroup;
	currentStreamProperty->groupOrSegmentNumber = groupNumber;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutEndGroup(uint32 bytePosition, uint16 groupNumber, bool requestNotification)
	{
	StreamProperty * currentStreamProperty = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(currentStreamProperty, bytePosition));

	currentStreamProperty->propertyType = VDR_MSMF_GROUP_END;
	currentStreamProperty->requestNotification = requestNotification;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutBeginSegment(uint32 bytePosition, uint16 segmentNumber, bool sendNotification)
	{
	StreamProperty * currentStreamProperty = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(currentStreamProperty, bytePosition));

	currentStreamProperty->propertyType |= VDR_MSMF_SEGMENT_START;
	currentStreamProperty->groupOrSegmentNumber = segmentNumber;
	currentStreamProperty->requestNotification = sendNotification;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutEndSegment(uint32 bytePosition, uint16 segmentNumber, bool requestNotification)
	{
	// add the end segment to the list of properties
	StreamProperty * property = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(property, bytePosition));
	property->propertyType |= VDR_MSMF_SEGMENT_END;
	property->groupOrSegmentNumber = segmentNumber;
	property->requestNotification = requestNotification;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutTag(uint32 bytePosition, const TAG & tag)
	{
	StreamProperty * currentStreamProperty = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(currentStreamProperty, bytePosition));

	currentStreamProperty->propertyType |= VDR_MSMF_TAGS_VALID;
	currentStreamProperty->tag = tag;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutTimeDiscontinuity(uint32 bytePosition)
	{
	// add data discontinuity to pending property list
	StreamProperty * property = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(property, bytePosition));
	property->propertyType |= VDR_MSMF_TIME_DISCONTINUITY;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::PutDataDiscontinuity(uint32 bytePosition)
	{
	// add data discontinuity to pending property list
	StreamProperty * property = NULL;
	STFRES_REASSERT(GetNextFreePropertyEntry(property, bytePosition));
	property->propertyType |= VDR_MSMF_DATA_DISCONTINUITY;

	STFRES_RAISE_OK;
	}

STFResult PendingPropertiesQueue::ApplyPendingProperty(uint32 bytePosition, StreamingFormatter & outputFormatter)
	{	
	StreamProperty * property = NULL;
	STFRES_REASSERT(GetNextPropertyForBytePosition(bytePosition, property));

	switch (property->propertyType)
		{
		case VDR_MSMF_TAGS_VALID:
			if (property->tag.id == VDRTID_DONE)
				STFRES_REASSERT(outputFormatter.CompleteTags());
			else
				STFRES_REASSERT(outputFormatter.PutTag(property->tag));
			break;

		case VDR_MSMF_GROUP_START:
			STFRES_REASSERT(outputFormatter.BeginGroup(property->groupOrSegmentNumber, property->requestNotification, property->singleUnitGroup));
			break;

		case VDR_MSMF_GROUP_END:
			STFRES_REASSERT(outputFormatter.CompleteGroup(property->requestNotification));
			break;

		case VDR_MSMF_SEGMENT_START:
			STFRES_REASSERT(outputFormatter.BeginSegment(property->groupOrSegmentNumber, property->requestNotification));
			break;

		case VDR_MSMF_SEGMENT_END:
			STFRES_REASSERT(outputFormatter.CompleteSegment(property->requestNotification));
			break;

		case VDR_MSMF_START_TIME_VALID:
			STFRES_REASSERT(outputFormatter.PutStartTime(property->startOrEndTime));
			break;

		case VDR_MSMF_END_TIME_VALID:
			STFRES_REASSERT(outputFormatter.PutEndTime(property->startOrEndTime));
			break;

		case VDR_MSCF_CUT_AFTER:
			STFRES_REASSERT(outputFormatter.PutCutDuration(property->skipOrCutDurationDuration));
			break;

		case VDR_MSCF_SKIP_UNTIL:
			STFRES_REASSERT(outputFormatter.PutSkipDuration(property->skipOrCutDurationDuration));
			break;

		case VDR_MSMF_TIME_DISCONTINUITY:
			STFRES_REASSERT(outputFormatter.PutTimeDiscontinuity());
			break;

		case VDR_MSMF_DATA_DISCONTINUITY:
			STFRES_REASSERT(outputFormatter.PutDataDiscontinuity());
			break;

		default:
			DP("Unhandled property type %x\n", property->propertyType);
			ASSERT(false);

		}

	STFRES_REASSERT(RemoveFirstPendingProperty());

	STFRES_RAISE_OK;
	}
