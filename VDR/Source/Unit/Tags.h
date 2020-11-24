///
/// @file       VDR/Source/Unit/Tags.h
///
/// @brief      Classes for Tag processing
///
/// @author     Ulrich Sigmund, Stefan Herr
///
/// @date       2002-12-12 
///
/// @par OWNER: 8k Audio Decoding Team
///
/// @par SCOPE: INTERNAL Implementation File
///
/// Classes for Tag processing
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#ifndef ITAGS_H
#define ITAGS_H

#include "ITagUnit.h"

///////////////////////////////////////////////////////////////////////////////
//! TagFilter class
///////////////////////////////////////////////////////////////////////////////
/*! 
  Collector class to dynamically build tag lists of tags that are not
  yet known at compile time.
*/
class TagFilter 
   {
   protected:
   TAG	* list;		/// First tag of the list
   TAG	*  ntp;		/// Last tag of the list
		
   int 	size;		/// Maximum size of the list
   int	num;		/// Actual size of the list
		
   /// Allocate or reallocate the array for the tag list
   virtual STFResult InternalStart(int size)
         {
         if (!list)
            {
            this->size = size;
            list = new TAG[size];				
            }
         else if (this->size < size)
            {
            delete[] list;
            this->size = size;
            list = new TAG[size];				
            }
			
         this->num = 0;
         ntp = list;
			
         STFRES_RAISE_OK;
         }
			
   virtual STFResult InternalAdd(TAG tag) {*ntp++ = tag; STFRES_RAISE_OK;}
   virtual STFResult InternalDone(void) {return InternalAdd(TAGDONE);}
   public:
   virtual ~TagFilter(){}; // NHV: Added for g++ 4.1.1
   virtual STFResult Start(int size = 100) {return InternalStart(size);}
   virtual STFResult Add(TAG tag) {return InternalAdd(tag);}
   virtual STFResult Done(void) {return InternalDone();}

   operator TAG * (void) {return list;}
   };


///////////////////////////////////////////////////////////////////////////////
//! TagSplitter class
///////////////////////////////////////////////////////////////////////////////

/*!
  Splitter that implements splitting a tag list into two lists, based
  on an array of tag type IDs. The resulting lists may further be split by
  other splitters. When the splitter is destructed or a new split
  is performed, the lists are joined again. This way, the splitter can
  be used to split the list several times, each time with a new set of
  tag units. A single splitter cannot be used for recursive splits, a
  new splitter is needed for each recursion.

  The splitter splits the list into two segments, one starting at offset
  zero, and the other one not. Both segments are disjunct except for the
  final TAGDONE tag. To merge the segments back into one list, the non
  zero based segment is attached to the zero based segment.
*/
class TagSplitter
   {
   protected:
   TAG	*	tags;		/// The complete tag list
   bool		split;	/// Flag, whether the list is currently split

   int		head;		/// offset of first tag of not zero start segment
   int		ptail;	/// offset of last tag of zero start segment

   // Merge the two lists back into one
   void Merge(void)
         {
         if (split)	// If we have split lists,
            {
            // link the last tag of the zero based segment to the first of
            // the non zero based segment
            tags[ptail].skip = head - ptail;
            split = false;
            }
         }

   TAG * SplitLists(uint32 mask, bool matches, uint32 * id);

   public:
   /// Constructor, requires the tag list, that will be split
   TagSplitter(TAG * tags_) : tags(tags_) {split = false;}

   /// Destructor ensures that the tag list is merged after use
   ~TagSplitter(void) {Merge();}

   /*!
     Split the list, and return the segment, that contains the tags
     which have a tag _type_ ID that is also present in the *id array.
     This can be used to limit a tag list to the tags that are
     used by a downstream or child unit.
     If no tag matches the tag type IDs the function returns NULL.
   */
   TAG * Split(uint32 * id);

   /*!
     Split the list, and return the segment, that contains the tags
     which do not have a _tag _ID that is present in the *id array.
     This can be used to remove speficic tags from a tag list, before
     it is sent to a downstream or child unit for further processing.
   */
   TAG * RemoveTags(uint32 * tags);

   /*!
     Split the list and return the segment that contains the tags
     which do not have a _tag_type_ID that is present in the *id array.
     This can be used to remove entire tag type groups from a tag list, before
     it is sent to a downstream or child unit for further processing.
   */
   TAG * RemoveTagsByType(uint32 * id);
   };

/// Merge two tag type lists into one, if both input lists are empty, the
/// resulting list will be NULL.  It is the callers responsibility to delete
/// the resulting list.
STFResult MergeTagTypeIDList(const VDRTID * inIDs, const VDRTID * supportedIDs, VDRTID * & outIDs);

/// Merge multiple tag type lists into one, if all input lists are empty, the
/// resulting list will be NULL.  It is the callers responsibility to delete
/// the resulting list.
STFResult MergeTagTypeIDLists(VDRTID ** inIDLists, uint32 nunLists, VDRTID * & outIDs);

#endif	// of #ifndef TAGS_H

