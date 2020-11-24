///
/// @brief 
///

#ifndef STFARRAY_H
#define STFARRAY_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include <assert.h>

/// @class STFIntArray
///
/// @brief this class is not tested! use at own risk
///
class STFIntArray
	{
	protected:
		uint32 size;
		uint32 numElements;
		int32 * data;
	public:
		STFIntArray(uint32 initialSize)
			{
			assert(initialSize > 0);
			
			size = initialSize;
			numElements = 0;

			data = new int32[initialSize];
			};

		~STFIntArray()
			{
			if (data)
				delete[] data;
			};

		/// this function is not tested.
		/// @todo at least setting empty fields to 0 is missing 
		STFResult Add(int32 adddata)
			{
			if (numElements == size)
				{
				int32 * newData = new int32[numElements * 2];
				memcpy(newData, data, numElements*sizeof(uint32));
				size *= 2;				
				delete[] data;
				data = newData;				
				};
			
			data[numElements++] = adddata;

			STFRES_RAISE_OK;
			};

		/// this function is not tested.
		/// @todo at least setting empty fields to 0 is missing 
		STFResult SetAt(uint32 at, int32 setdata)
			{
			if (at >= size)
				{		
				size = at + 1;
				
				int32 * newData = new int32[size];
				memcpy(newData, data, numElements*sizeof(uint32));
				numElements = at;
				delete[] data;
				data = newData;
				}
			data[at] = setdata;
			
			STFRES_RAISE_OK;
			};

		STFResult ElementAt(uint32 at, int32 & atdata) const
			{
			if (at < numElements)
				{
				atdata = data[at];
				}
			else
				{
				STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
				}

			STFRES_RAISE_OK;
			};

		uint32 Size() const
			{
			return numElements;
			}		
	};

/// @class STFPointerArray
///
/// @brief an dynamic array of pointers
///
/// this implementation is not thread-safe!
///
class STFPointerArray
	{
	protected:
		uint32 size;
		uint32 numElements;
		pointer * data;
	public:
		/// @brief constructor
		///
		/// @param initialSize the initial Size of the internal Array.
		///
		STFPointerArray(uint32 initialSize)
			{
			//ASSERT(initialSize > 0);
			
			size = initialSize;
			numElements = 0;

			data = new pointer[initialSize];
			};

		~STFPointerArray()
			{
			if (data)
				delete[] data;
			};

		/// @brief adds a new pointer.
		///
		/// Adds a pointer to the end of the array,  if the array is to small, its size
		/// is doubled.
		///
		STFResult Add(pointer adddata)
			{
			if (numElements == size)
				{
				pointer * newData = new pointer[numElements * 2];
				memset(newData,0,numElements * 2);
				memcpy(newData, data, numElements * 4);
				
				size *= 2;
				delete[] data;
				data = newData;				
				};
			
			data[numElements++] = adddata;
			STFRES_RAISE_OK;
			};

		
		STFResult SetAt(uint32 at, pointer setdata)
			{
			if (at >= size)
				{		
				size = at + 1;
				
				pointer * newData = new pointer[size];
				memset(newData, 0, size * 4);
				memcpy(newData, data, numElements * 4);
				numElements = at + 1;
				delete[] data;
				data = newData;
				}

			data[at] = setdata;

			STFRES_RAISE_OK;
			};
		
		/// @brief selects a pointer
		///
		/// puts the pointer of element at into atdata
		///
		/// @param at the index of the element
		/// @param atdata the pointer to fill.
		///
		STFResult ElementAt(uint32 at, pointer & atdata) const
			{
			if (at < numElements)
				{
				atdata = data[at];
				}
			else
				{
				STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
				}

			STFRES_RAISE_OK;
			};

		/// @brief returns the number of Elements.
		uint32 Size() const
			{
			return numElements;
			}		
	};

#endif //STFARRAY_H
