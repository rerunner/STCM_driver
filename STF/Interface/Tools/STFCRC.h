///
/// @brief      CRC calculation class
///

#ifndef STFCRC_H
#define STFCRC_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"

/// @class STFCRC
///
/// @brief CRC16 calculation class
///
/// This class calculates a CCCIT compliant Cyclic Redundancy Checksum. There are two ways to use this 
/// class:
/// In a static mode: You use the class function CalculateCRC to calculate a checksum over a block of data, and
/// get the checksum of the block. 
/// In a streaming mode: You consecutively feed in several blocks into an instance of the STFCRC class. The 
/// checksum is calculated iterative over all data you pass in. If you finally fed in all data you want to 
/// be convered by the checksum, you can retrive the CRC from the object. 
///
class STFCRC
	{
	/// Precalculated table to speed up the calculation of the table
	static const uint16 CRCTable[256];

	private:
		/// The current value of the CRC. You can access this value by \ref GetCRC
		uint16 currentCRC;
	public:
		/// @brief Construct an CRC object for streaming mode and sets an initial value. 
		/// @param initializationValue [in]  The initial value of the CRC
		STFCRC(uint16 initalizationValue) { currentCRC = initalizationValue; };

		/// @brief Update the checksum in streaming mode by calculating further the checksum over 
		/// a block of data. 
		/// @param block [in] The block to calculate the (partial) checksum on. This may not be NULL
		/// @param size [in]  The size of the block passed in. This may not be 0
		STFResult Update(uint8 * block, uint32 size);

		/// @brief Get the current value of the CRC. 
		/// 
		/// If no blocks have been passed in, this call will 
		/// return the initial value passed in to the costructor.
		/// @param crc [out]  The current value of the CRC.
		STFResult GetCRC(uint16 & crc) { crc = currentCRC; STFRES_RAISE_OK; };

		/// @brief Calculates the checksum over one block of data
		///
		/// This method calculates the checksum over a block of byte data. 
		/// @param block [in] The data block to calculate the CRC on. This may not be NULL.
		/// @param size [in]  The size of the block to calculate the CRC on. This may not be 0
		/// @param initializationValue [in] The initial value of the CRC
		/// @param crc [out]  The calculated checksum
		static STFResult CalculateCRC(uint8 * block, uint32 size, uint16 initializationValue, uint16 & crc);
	};

#endif // STFCRC_H
