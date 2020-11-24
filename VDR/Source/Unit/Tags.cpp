//
// PURPOSE:    Classes for Tag processing
//

#include "Tags.h"
#include "VDR/Interface/Unit/IVDRTags.h"

//////////////////////////////////////////////////////////////////////////////
// Remaining implementations of TAGList (see IVDRTags.h"
//////////////////////////////////////////////////////////////////////////////

//
// Grow the dynamic tag list
//
void TAGList::Grow(void)
	{
	TAG	*	ftags;
	int		i;

	max *= 2;
	ftags = new TAG[max];
	for(i=0; i<num; i++)
		ftags[i] = etags[i];
	delete[] etags;
	etags = ftags;
	}

//
// Growing the dynamic array version
//
TAGList & TAGList::operator + (const TAG & tag)
	{
	if (num == max) Grow();

	etags[num++] = tag;

	return *this;
	}


// Try to find a tag in a list and return its data uint32. Provide the default
// if the tag is not in the list.

intptr_t FilterTags (TAG *tags, uint32 id, PADDR def)
	{
	while (tags->id != 0  &&  tags->id != id)
		tags++;
	return (tags->id != 0) ? tags->data : def;
	}                 


//
// Check, whether the masked part of the ID of a tag �s present in an array of
// IDs.  Based on the mask value, this function can be used to check for unit IDs
// or full tag IDs.
//
static inline bool TagInList(uint32 tag, uint32 * ids, uint32 mask)
	{
	tag &= mask;

	while (*ids && tag != *ids) ids++;

	return *ids != 0;
	}

// 
// Split a list of tags into two based on given IDs, a mask of valid bits in the
// tag ID and a flag, whether to return the matching or non matching segment.
//
TAG * TagSplitter::SplitLists(uint32 mask, bool matches, uint32 * ids)
	{
	int start, pstart, tail;
	bool match;

	//
	// If the list is currently split, merge it before processing
	//
	Merge();

	//
	// If the tag list is non empty...
	//
	if (tags && tags[0].id)
		{
		//
		// Check if the firs tag in the list matches the criterion
		//
		match = TagInList(tags[0].id, ids, mask);

		//
		// Find first tag that has a different match status than the zero index tag of the list
		// Search is done using a second trailing index to remember the tag before the one
		// looked for.
		//
		pstart = 0;
		start = tags[0].skip;	// Start with second tag in list
		while (tags[start].id && TagInList(tags[start].id, ids, mask) == match)
			{
			pstart = start;							// Advance trailing index
			start = start + tags[start].skip;	// Advance to next tag in list
			}

		//
		// If the tag list contains matching and non matching tags, a split is neccessary.
		//
		if (tags[start].id)
			{
			//
			// Found the head, so remember the split point as the first tag of the
			// non zero based segment.  Keep the tag before the split as potential last
			// tag of zero based segment.
			//
			ptail = pstart;
			head = pstart = start;

			//
			// Now divide the remaining tags on the two segments.  Start with the next
			// tag after the split
			//
			tail = start + tags[start].skip;

			while (tags[tail].id)
				{
				//
				// Place the current tag in one of the two lists, based on the match status
				//
				if (TagInList(tags[tail].id, ids, mask) == match)
					{
					//
					// Attach to zero based segment
					//
					tags[ptail].skip = tail - ptail;
					ptail = tail;
					}
				else
					{
					//
					// Attach to non zero based segment
					//
					tags[pstart].skip = tail - pstart;
					pstart = tail;
					}

				//
				// Proceed with next tag
				//
				tail += tags[tail].skip;
				}

			//
			// Terminate both segments by pointing the successor of both tails
			// to the done tag
			//
			tags[ptail].skip = tail - ptail;
			tags[pstart].skip = tail - pstart;

			//
			// Set the split status
			//
			split = true;

			//
			// Return the requested segment
			//
			if (match == matches)
				return tags;
			else
				return tags + start;
			}
		else
			{
			//
			// The list contains either only matching or non matching tags, so no split
			// is required.
			//
			if (match == matches)
				return tags;
			else
				return NULL;
			}
		}
	else
		{
		//
		// If the tag list is empty return NULL
		//
		return NULL;
		}
	}

TAG * TagSplitter::RemoveTags(uint32 * tags)
	{
	//
	// Split the list based on the full tag ID, and return the non
	// matching segment.
	//
	return SplitLists(ANYTAG, false, tags);
	}

TAG * TagSplitter::RemoveTagsByType(VDRTID * ids)
	{
	//
	// Split the list based on the "type" part of the ID, and return the non
	// matching segment.
	//
	return SplitLists(ANYTYPE, false, ids);
	}

TAG * TagSplitter::Split(VDRTID * ids)
	{
	//
	// Split the list based on the "type" part of the ID, and return
	// the matching segment.
	//
	return SplitLists(ANYTYPE, true, ids);
	}

STFResult MergeTagTypeIDList(const VDRTID * inIDs, const VDRTID * supportedIDs, VDRTID * & outIDs)
	{	
	uint32 i, j, tuIdx;
	uint32 tempNumIDs;
	
	tempNumIDs = 0;

	if (inIDs != NULL)
		{
		// Count entries of the called array
		i = 0;
		while(inIDs[i++] != NULL) 
			tempNumIDs++;
		}
		
	//get the Tag Unit IDs from our own unit
	if (supportedIDs != NULL)
		{
		i = 0;
		while (supportedIDs[i++] != NULL) 
			tempNumIDs++;
		}

	if (tempNumIDs > 0)
		{
		outIDs = new VDRTID[tempNumIDs + 1];

		if (outIDs == NULL)
			{
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
			}

		// Enter all Tag Unit IDs, avoid duplicacy
		tuIdx = 0;
		if (supportedIDs != NULL)
			{
			i=0;
			while(supportedIDs[i] != NULL)
				{
				outIDs[tuIdx] = supportedIDs[i];
				tuIdx++;
				i++;
				}
			}

		if (inIDs != NULL)
			{
			i=0;
			while (inIDs[i] != NULL)
				{
				// Search if already in tagUnitIDs array
				j = 0;
				while (j != tuIdx && outIDs[j] != inIDs[i]) j++;
				// Reached the last entry of tagUnitIDs? If yes, then it is not a double entry,
				// so we can make it.
				if (j == tuIdx)
					{
					outIDs[tuIdx] = inIDs[i];
					tuIdx++;
					}
				i++;
				}
			}

		outIDs[tuIdx] = VDRTID_DONE;
		}
	else
		outIDs = NULL;

	STFRES_RAISE_OK;
	}

STFResult MergeTagTypeIDLists(VDRTID ** inIDLists, uint32 numLists, VDRTID * & outIDs)
	{
	uint32	i, j, k;
	uint32	numOutIDs;

	numOutIDs = 0;
	for (i=0; i<numLists; i++)
		{
		if (inIDLists[i] != NULL)
			{
			j = 0;
			while (inIDLists[i][j] != VDRTID_DONE) 
				j++;

			numOutIDs += j;
			}
		}

	if (numOutIDs)
		{
		outIDs = new VDRTID[numOutIDs + 1];
		numOutIDs = 0;
		for (i=0; i<numLists; i++)
			{
			if (inIDLists[i] != NULL)
				{
				j = 0;
				while (inIDLists[i][j] != VDRTID_DONE)
					{
					k = 0;
					while (k < numOutIDs && outIDs[k] != inIDLists[i][j])
						k++;

					if (k == numOutIDs)
						{
						outIDs[numOutIDs] = inIDLists[i][j];

						numOutIDs++;
						}
					j++;
					}
				}
			}
		outIDs[numOutIDs] = VDRTID_DONE;
		}
	else
		outIDs = NULL;

	STFRES_RAISE_OK;
	}
