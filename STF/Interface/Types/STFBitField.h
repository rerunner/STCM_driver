/// @brief Classes for bitfield/bitarray manipulation

#ifndef STFBITFIELD_H
#define STFBITFIELD_H

#include "STF/Interface/Types/STFBasicTypes.h"

/*! \brief represents a bitfield
  */
class STFBitField {
	private:
		uint32		data; //!< the bitfield
		STFBitField(uint32 data) {this->data = data;}
	public:
		STFBitField(void) {data = 0;}//!< empty constructor
		///constructor
		///
		/// \param at the position from which the values are set
		/// \param num the length of the value
		/// \param val the value to set
		STFBitField(int32 at, int32 num, uint32 val) {data = ((uint32)val & ((1UL << num) -1)) << at;}
		STFBitField(int32 at, bool val) {data = val ? (1UL << at) : 0;}
		
		~STFBitField(void) {;}//!< destructor               
		operator bool() {return data != 0;}
		
		friend STFBitField operator| (STFBitField, STFBitField);
		friend STFBitField operator& (STFBitField, STFBitField);
		friend STFBitField operator^ (STFBitField, STFBitField);
		friend STFBitField operator- (STFBitField, STFBitField);

		inline STFBitField& operator= (volatile STFBitField);
		STFBitField& operator|= (STFBitField);
		STFBitField& operator&= (STFBitField);
		STFBitField& operator^= (STFBitField);
		STFBitField& operator-= (STFBitField);
		
		friend bool operator== (STFBitField, STFBitField);
		friend bool operator!= (STFBitField, STFBitField);
		friend bool operator<= (STFBitField, STFBitField);
		friend bool operator>= (STFBitField, STFBitField);
		
		void Put(int32 at, int32 num, uint32 val);
		void INCL(int32 at);
		void EXCL(int32 at);
		bool Contains(int32 at);		
		uint32 Get(int32 at, int32 num);		
	}; 
	
class STFBitArray {
	private:
		uint32	*	data;
		int32 dwSize;
	public:
		STFBitArray(void) {data = NULL; dwSize = 0;}
		STFBitArray(const STFBitArray&);
		~STFBitArray(void);
		
		STFBitArray& operator= (STFBitArray);
		
		operator bool();
		
		friend STFBitArray operator| (STFBitArray, STFBitArray);
		friend STFBitArray operator& (STFBitArray, STFBitArray);
		friend STFBitArray operator^ (STFBitArray, STFBitArray);
		friend STFBitArray operator- (STFBitArray, STFBitArray);
		
		STFBitArray& operator|= (STFBitArray);
		STFBitArray& operator&= (STFBitArray);
		STFBitArray& operator^= (STFBitArray);
		STFBitArray& operator-= (STFBitArray);
		
		friend bool operator== (STFBitArray, STFBitArray);
		friend bool operator!= (STFBitArray, STFBitArray);
		friend bool operator<= (STFBitArray, STFBitArray);
		friend bool operator>= (STFBitArray, STFBitArray);
		
		void INCL(int32 at);
		void EXCL(int32 at);
		bool Contains(int32 at);		
	};

//
// Inline method for bit classes
//
                         
inline STFBitField operator| (STFBitField f1, STFBitField f2)
	{
	return STFBitField(f1.data | f2.data);
	}
	
inline STFBitField operator& (STFBitField f1, STFBitField f2)
	{
	return STFBitField(f1.data & f2.data);
	}  
	
inline STFBitField operator^ (STFBitField f1, STFBitField f2)
	{
	return STFBitField(f1.data ^ f2.data);
	}
	
inline STFBitField operator- (STFBitField f1, STFBitField f2)
	{
	return STFBitField(f1.data & ~f2.data);
	}
	

inline STFBitField& STFBitField::operator= (volatile STFBitField f)
	{
	data = f.data;
	return *this;
	}

inline STFBitField& STFBitField::operator|= (STFBitField f)
	{      
	data |= f.data;
	return *this;
	}
	
inline STFBitField& STFBitField::operator&= (STFBitField f)
	{
	data &= f.data;                     
	return *this;
	}
	
inline STFBitField& STFBitField::operator^= (STFBitField f)
	{
	data ^= f.data;                     
	return *this;
	}
		
inline STFBitField& STFBitField::operator-= (STFBitField f)
	{
	data &= ~f.data;                     
	return *this;
	}
		
inline bool operator== (STFBitField f1, STFBitField f2)
	{
	return f1.data == f2.data;
	}
	
inline bool operator!= (STFBitField f1, STFBitField f2)
	{
	return f1.data != f2.data;
	}
	
inline bool operator<= (STFBitField f1, STFBitField f2)
	{
	return f1-f2;
	}
	
inline bool operator>= (STFBitField f1, STFBitField f2)
	{
	return f2-f1;
	}
		

inline void STFBitField::Put(int32 at, int32 num, uint32 val)
	{
	uint32 mask = ((1UL << num) -1) << at;	
	data = (val & mask) | (data & ~mask);		
	}

inline void STFBitField::INCL(int32 at)
	{
	data |= 1UL << at;
	}                 

inline void STFBitField::EXCL(int32 at)
	{
	data &= ~(1UL << at);
	}
	
	
inline bool STFBitField::Contains(int32 at)
	{
	return (data & (1UL << at)) != 0;
	}
	
inline uint32 STFBitField::Get(int32 at, int32 num)
	{
	return (data >> at) & ((1UL << num) -1);
	}

#endif               
