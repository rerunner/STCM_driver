///
/// @brief      Foundation classes for range management.
///

#ifndef STFRANGEMANAGER_H
#define STFRANGEMANAGER_H

#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/STFCallingConventions.h"


////////////////////////////////////////////////////////////////////
//
//  STFRangeManager Class
//
////////////////////////////////////////////////////////////////////

class LIBRARYCALL STFRangeManager
	{
	private:
		struct Range
			{
			uint32	start, size;
			void		*userData;
			uint32	dummy;		///< makes struct 16 bytes large
			};

		Range		*ranges;			///< array of ranges
		uint32	maxRanges;		///< maximum size of the array
		uint32	numRanges;		///< current valid entries in the array

	public:
		STFRangeManager (void);
		~STFRangeManager (void);

		/// @brief Add a range.
		///
		/// @param start    [IN] Beginning of the range.
		/// @param size     [IN] Size of the range.
		/// @param userData [IN] A client value associated with the range.
		///
		/// @return         Result value, indicates success or failure.
		/// @retval STFRES_OK
		/// @retval STFRES_NOT_ENOUGH_MEMORY
		/// @retval STFRES_INVALID_PARAMETERS  Range is empty.
		/// @retval STFRES_RANGE_VIOLATION     Range overlaps with an existing range.
		///
		STFResult Add (uint32 start, uint32 size, void * userData);

		/// @brief Remove a range.
		///
		/// Note that this is a simple removal. It will not cut out of ranges.
		///
		/// @param start   [IN] Beginning of the range.
		/// @param size    [IN] Size of the range.
		///
		/// @return        Result value, indicates success or failure.
		/// @retval STFRES_OK
		/// @retval STFRES_OBJECT_NOT_FOUND  Start or size do not fit any existing range.
		///
		STFResult Remove (uint32 start, uint32 size);

		/// @brief Get the number of stored ranges.
		///
		/// @return        Number of ranges.
		///
		uint32 GetNumRanges (void) const {return numRanges;}

		/// @brief Read out the stored ranges in increasing order.
		///
		/// @param index    [IN] Index of the range, must be 0..GetNumRanges()-1.
		/// @param start    [OUT] Beginning of the range.
		/// @param size     [OUT] Size of the range.
		/// @param userData [OUT] A client value associated with the range.
		///
		/// @return         Result value, indicates success or failure.
		/// @retval STFRES_OK
		/// @retval STFRES_OBJECT_NOT_FOUND  There is no first/next range.
		///
//		STFResult GetFirstRange const (uint32 & start, uint32 & size);
//		STFResult GetNextRange const (uint32 & start, uint32 & size);
		STFResult GetRange (uint32 index, uint32 & start, uint32 & size, void *& userData) const;
	};



#endif
