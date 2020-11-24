//
// PURPOSE:   Class for bitstreams (generic, file, memory)
//

#ifndef STFBITSTREAM_INC
#define STFBITSTREAM_INC
 
#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFInt64.h"

#define BITSTREAM_BACKTRACK	8
#define BITSTREAM_ADVANCE		256
#define BITSTREAM_BUFFSIZE		(BITSTREAM_BACKTRACK+BITSTREAM_ADVANCE)

	//
	// Bit input stream
	//
class STFBitInStream {
	public:
		virtual ~STFBitInStream() {}           
		//
		// Read a set of bits (up to 32)
		//
		virtual uint32 ReadBits(int32 num) = 0;
		//
		// Read a set of bits
		//
		virtual void ReadBits(int32 num, void * buff) = 0;
		//
		// Read a single bit
		//
		virtual bool ReadBit(void) = 0;
		
		//
		// Read a set of bits (up to 32) without advancing
		//
		virtual uint32 PeekBits(int32 num) = 0;
		//
		// Read a single bit without advancing
		//
		virtual bool PeekBit(void) = 0; 
		
		//
		// Get the current bit position
		//
		virtual STFInt64 BitPosition(void) = 0;
		virtual STFInt64 BitLength(void) = 0;
		//
		// Check for end of file
		//
		virtual bool EndOfFile(void) = 0;
		
		//
		// Reset bit position
		//
		virtual void Reset(void) = 0;
		
		//
		// Skip a number of bits
		//
		virtual void Skip(STFInt64 bits) = 0;
		virtual void SeekBits(STFInt64 at) 
			{
			STFInt64 pos = BitPosition();
			if (at < pos) 
				{
				Reset(); 
				Skip(at);
				}
			else 
				{
				Skip(at - pos);
				}
			}			
	};

class STFBitOutStream {
	public:
		virtual ~STFBitOutStream() {}
		virtual void WriteBits(int32 num, uint32 data) = 0;
		virtual void WriteBits(int32 num, void * data) = 0;
		virtual void WriteBit(bool data) = 0;
		
		virtual STFInt64 BitPosition(void) = 0;
		virtual STFInt64 Length(void) = 0;
	};

class STFBitInOutStream : public STFBitInStream, public STFBitOutStream {		
	};

	//
	// Buffered version of the bit input stream
	//
class STFBufferedBitInStream : public STFBitInStream {
	private:
		uint8 buffer[BITSTREAM_BUFFSIZE];
		STFInt64 position;
		int32 buffSize;
		int32 buffPos;
		int32 buffBits;
		bool eof;
		
		bool Advance(void);
	protected:
		//
		// Read a number of bytes, to be implemented by subclass
		// implementation
		//
		virtual int32 ReadBytes(STFInt64 at, int32 num, void * buff) = 0;
		virtual STFInt64 SizeBytes(void) = 0;
	public:                 
		STFBufferedBitInStream(void);
		~STFBufferedBitInStream(void);

		uint32 ReadBits(int32 num);
		void ReadBits(int32 num, void * buff);
		bool ReadBit(void);
		
		uint32 PeekBits(int32 num);
		bool PeekBit(void);
		
		STFInt64 BitPosition(void) {return (position+buffPos)*8+(8-buffBits);}
		STFInt64 BitLength(void) {return SizeBytes() * 8;} 
		bool EndOfFile(void) {return eof;}
		
		void Skip(STFInt64 bits); 
		void Reset(void);
	};               

class STFFileBitInStream : public STFBufferedBitInStream {
	private:
		int32	file;                     
		int32	fpos;
	public:
		STFFileBitInStream(const STFString & name);
		~STFFileBitInStream(void);
		
		int32 ReadBytes(STFInt64 at, int32 num, void * buff);
	};

class STFMemoryBitInStream : public STFBitInStream {
	private:
		struct BuffNode {
			struct BuffNode * succ;
			void *				data;
			int32					size;
			};
		BuffNode *		first;			
		BuffNode	*		last;
	public:
				
	};

class STFBufferedBitOutStream : public STFBitOutStream {
	private:
		uint8 buffer[BITSTREAM_BUFFSIZE];
		STFInt64 position;
		int32 buffSize;
		int32 buffPos;
		bool eof;        
	protected:
		virtual int32 WriteBytes(STFInt64 at, int32 num, void * buff) = 0;	
	public:                 
		STFBufferedBitOutStream(void);
		~STFBufferedBitOutStream(void);

		void WriteBits(int32 num, uint32 data);
		void WriteBits(int32 num, void * data);
		void WriteBit(bool data);
		
		STFInt64 BitPosition(void) {return position;}
		STFInt64 Length(void) {return position;}
	};

class STFFileBitOutStream : public STFBufferedBitOutStream {
	private:
		int32	file;
		int32	fpos;
	public:
		STFFileBitOutStream(const STFString & name);
		~STFFileBitOutStream(void);
		
		int32 WriteBytes(STFInt64 at, int32 num, void * buff);
	};

#endif // STFBITSTREAM_INC
