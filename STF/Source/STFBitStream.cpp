
#include <stdio.h>
#include <memory.h>
#include "STF/Interface/STFBitStream.h"

	//
	// Advance the buffer
	//
bool STFBufferedBitInStream::Advance(void)
	{     
	int32 advance;
	               
	if (!eof)
		{
		//
		// If there is still some data in the buffer
		//
		if (buffSize>=BITSTREAM_ADVANCE)
			{       
			//
			// preserve it during advance
			//
			advance = buffSize-BITSTREAM_BACKTRACK;
			//
			// copy the remaining bytes
			//
			memcpy(buffer+8, buffer+advance, BITSTREAM_BACKTRACK);
         
         //
         // advance position
         //
			position+=advance;
			buffPos-=advance;
			buffSize-=advance;
         //
         // Read a new block of data
         //
			buffSize+=ReadBytes(position+BITSTREAM_BACKTRACK, BITSTREAM_ADVANCE, buffer+BITSTREAM_BACKTRACK);					
			}
		else
			buffSize+=ReadBytes(position+buffSize, BITSTREAM_BUFFSIZE-buffSize, buffer+buffSize);					
		
		eof = buffPos == buffSize;
		}
	return eof;	
	}

	//
	// Read a single bit
	//	
bool STFBufferedBitInStream::ReadBit(void)
	{                   
	bool res;
	
	if (buffPos==buffSize) if (Advance()) return false;				

	buffBits--;
	res = (buffer[buffPos] & (1UL << buffBits)) != 0;
	if (!buffBits) 
		{
		buffBits = 8;
		buffPos++;
		}       
	return res;				
	}  

bool STFBufferedBitInStream::PeekBit(void)
	{                   
	if (buffPos==buffSize) if (Advance()) return false;

	return (buffer[buffPos] & (1UL << (buffBits-1))) != 0;
	}  

uint32 STFBufferedBitInStream::ReadBits(int32 num)
	{                   
	uint32 res = 0;           
	
	if (num>0)
		{	
		if (num>=buffBits)
			{
			if (buffPos==buffSize) if (Advance()) return 0;
	
			res = (res << buffBits) | (buffer[buffPos] & ((1UL << buffBits) -1));
			num-=buffBits;
			buffBits = 8;
			buffPos++;
	
			while (num>=8)
				{
				if (buffPos==buffSize) if (Advance()) return 0;
	
				res = (res << 8) | buffer[buffPos];
				num-=8;
				buffPos++;
				}
			}                                           
		if (num)  
			{
			if (buffPos==buffSize) if (Advance()) return 0;
	
			res = (res << num) | ((buffer[buffPos] & ((1UL << buffBits) -1)) >> (buffBits-num)) ;
			buffBits-=num;
			}
		}
		
	return res;
	}  

void STFBufferedBitInStream::ReadBits(int32 num, void * buff)
	{  
	uint8		* bp = (uint8  *)buff;
	int32			i;
	int32 			bytes = (int32)(num/8);
	int32			bbytes;
	
	if (buffPos == 8)
		{
		if (bytes<=buffSize-buffPos)
			{
			memcpy(bp, buffer+buffPos, bytes);
			}
		else
			{                    
			bbytes = buffSize-buffPos;
			memcpy(bp, buffer+buffPos, bbytes);
			bp+=bbytes;
			bytes-=bbytes;
			position+=buffSize;
			ReadBytes(position, bytes, bp);
			bp+=bytes;
			buffSize = 0;
			buffPos = 0;
			position+=bytes;						
			}		                   
		}          
	else
		{
		bp = (uint8  *)buff;
		for(i=0; i<bytes; i++)
			{                    
			*bp++ = (uint8)(ReadBits(8));			
			}			        
		}
	*bp = (uint8)(ReadBits((int32)(num%8)));
	}

uint32 STFBufferedBitInStream::PeekBits(int32 num)
	{                   
	uint32 res = 0;             
	int32 onum = num;
	
	if (num>=buffBits)
		{
		if (buffPos==buffSize) if (Advance()) return 0;

		res = (res << buffBits) | (buffer[buffPos] & ((1UL << buffBits) -1));
		num-=buffBits;
		buffBits = 8;
		buffPos++;

		while (num>=8)
			{
			if (buffPos==buffSize) if (Advance()) return 0;

			res = (res << 8) | buffer[buffPos];
			num-=8;
			buffPos++;
			}
		}                                           
	if (num)  
		{
		if (buffPos==buffSize) if (Advance()) return 0;

		res = (res << num) | ((buffer[buffPos] & ((1UL << buffBits) -1)) >> (buffBits-num)) ;
		buffBits-=num;
		}          
	
	buffBits+=onum;    
	buffPos-=buffBits/8;
	buffBits%=8;
	
	if (!buffBits) 
		{
		buffBits=8;
		buffPos++;
		}
		
	return res;
	}  

STFBufferedBitInStream::STFBufferedBitInStream(void)
	{
	buffPos = 0;
	buffSize = 0;
	position = 0;
	buffBits = 8;	                
	eof = false;
	}            
	
STFBufferedBitInStream::~STFBufferedBitInStream(void) {}
		
void STFBufferedBitInStream::Reset(void)
	{
	buffPos = 0;
	buffSize = 0;
	position = 0;
	buffBits = 8;	                
	eof = false;
	}
	
void STFBufferedBitInStream::Skip(STFInt64 bits)
	{              
	int32 bitsLeft;
	
	if (buffBits>bits)
		buffBits -= bits.ToInt32();
	else
		{
		bitsLeft = 8*(buffSize-buffPos)-8+buffBits;
	
		if (bitsLeft>bits)
			{                   
			bits -= buffBits;
			buffPos++;
			buffBits = 8-(bits & 7).ToInt32();
			buffPos += bits.ToInt32()/8;			
			}
		else
			{
			bits-=bitsLeft;
			position+=buffSize;
			buffPos=0;      
			buffSize=0;
			position+=bits/8;
			buffBits = 8-(bits & 7).ToInt32();		                 
			}		
		}
	}
