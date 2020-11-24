///
/// @brief      A data structure that allows multiple readers but only one writer.
///

#ifndef STFSHAREDDATABLOCK_H
#define STFSHAREDDATABLOCK_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include <string.h>

class STFSharedDataBlock
	{
	protected:
		uint8				*	data;							///< Double size data area
		uint32				size;							///< Size of shared data
		volatile uint32	count0, count1, count2;	///< Progress indicator of writer
		uint32				rcount;						///< Copy of count done during read
	public:
		STFSharedDataBlock(uint32 size);
		~STFSharedDataBlock(void);

		/// Put data into shared block
		STFResult WriteData(void * data);

		/// Read data from shared block
		STFResult ReadData(void * data);

		/// Check for the availability of so far unread data.
		bool NewDataPending(void)
			{return rcount != count1;}
	};


#endif //STFSHAREDDATABLOCK_H
