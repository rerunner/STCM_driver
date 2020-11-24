///
/// @brief		 A data structure that allows multiple readers but only one writer.
///

#include "STF/Interface/Types/STFSharedDataBlock.h"

STFSharedDataBlock::STFSharedDataBlock(uint32 size)
	{
	this->size = size;

	//
	// Allocate a double size buffer, to keep two copies.  The second
	// copy avoids a wait for data, if one copy is currently beign written
	// to.  For every point in time, one of the two copies contains a
	// valid result.  Thus if there is no multiple preemption of reader
	// and writer during operation on the shared data, the read will
	// succeed without a retry.
	//
	this->data = new uint8[2 * size];

	//
	// Initialize progress indicators
	//
	count0 = count1 = count2 = 0;
	}

STFSharedDataBlock::~STFSharedDataBlock(void)
	{
	delete[] data;
	}

STFResult STFSharedDataBlock::WriteData(void * data)
	{
	//
	// Signal begin of write to buffer0
	//
	count0++;
	//
	// Write to buffer0
	//
	memcpy(this->data, data, size);
	//
	// Signal completion of write to buffer0, and start of write
	// to buffer1
	//
	count1 = count0;
	//
	// Write to buffer1
	//
	memcpy(this->data + size, data, size);
	//
	// Signal completion of write to buffer1
	//
	count2 = count0;

	STFRES_RAISE_OK;
	}

STFResult STFSharedDataBlock::ReadData(void * data)
	{
	uint32	c0, c1, c2;

	//
	// We need to loop, in case we are preempted during our read
	// operation.
	//
	for(;;)
		{
		//
		// Read completion indicator for buffer1
		//
		c2 = count2;
		//
		// Get data of buffer1
		//
		memcpy(data, this->data + size, size);
		//
		// Read start indication of buffer1 (and completion indicator for buffer0)
		//
		c1 = count1;
		//
		// If start indication == completion indication, the buffer is valid and
		// we are done.
		//
		if (c2 == c1) 
			{
			rcount = c1;
			STFRES_RAISE_OK;
			}

		//
		// Get data of buffer 0
		//
		memcpy(data, this->data, size);
		//
		// Read start indication of buffer 0
		//
		c0 = count0;
		//
		// If start indication == completion indication, the buffer is valid and
		// we are done.
		//
		if (c0 == c1) 
			{
			rcount = c1;
			STFRES_RAISE_OK;
			}
		}
	}
