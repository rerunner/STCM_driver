//
// PURPOSE: 
//

#include "STF/Interface/Types/STFBitField.h"
#include "STF/Interface/STFDataManipulationMacros.h"


STFBitArray::STFBitArray(const STFBitArray &a)
	{ 
	int32 i;
	                         
	dwSize = a.dwSize;       
	if (dwSize)
		{
		data = new uint32[dwSize];
	
		for(i=0; i<dwSize; i++)
			data[i] = a.data[i];
		}
	else
		data = NULL;
	}
		
STFBitArray::~STFBitArray(void)
	{               
	if (data) delete[] data;
	}            

		
STFBitArray::operator bool()
	{               
	int32 i;
	//lint --e{613}
	for(i=0; i<dwSize; i++)
		if (data[i]) return true;
	return false;
	}            

STFBitArray& STFBitArray::operator=(STFBitArray a)
	{            
	int32 i;
	//lint --e{613}
	if (dwSize != a.dwSize)
		{
		delete[] data;
		dwSize = a.dwSize;
		if (dwSize)
			data = new uint32[dwSize];		
		else
			data = NULL;
		}                        
	
	for(i=0; i<dwSize; i++)
		data[i] = a.data[i];
		    
	return *this;
	} 
	
		
STFBitArray operator| (STFBitArray a1, STFBitArray a2)
	{
	STFBitArray r;
	int32 i;
	
	if (a1.dwSize > a2.dwSize)
		{
		r.dwSize = a1.dwSize;
		r.data = new uint32[r.dwSize];
		for(i=0; i<a2.dwSize; i++)
			r.data[i] = a1.data[i] | a2.data[i];
		for(i=a2.dwSize; i<a1.dwSize; i++)
			r.data[i] = a1.data[i];
		}
	else if (a2.dwSize)
		{                         
		r.dwSize = a2.dwSize;
		r.data = new uint32[r.dwSize];
		for(i=0; i<a1.dwSize; i++)
			r.data[i] = a1.data[i] | a2.data[i];
		for(i=a1.dwSize; i<a2.dwSize; i++)
			r.data[i] = a2.data[i];		
		}

	return r;	
	}

STFBitArray operator& (STFBitArray a1, STFBitArray a2)
	{
	STFBitArray r;
	int32 i, n;

	n = min(a1.dwSize, a2.dwSize);
	while ((n>0) && (!(a1.data[n-1] & a2.data[n-1]))) n--;
	if (n)
		{
		r.dwSize = n;
		r.data = new uint32[r.dwSize];
		for(i=0; i<n; i++)
			r.data[i] = a1.data[i] & a2.data[i]; 
		}

	return r;	
	}

STFBitArray operator^ (STFBitArray a1, STFBitArray a2)
	{
	STFBitArray r;
	int32 i, n;
	
	if (a1.dwSize > a2.dwSize)
		{
		r.dwSize = a1.dwSize;
		r.data = new uint32[r.dwSize];
		for(i=0; i<a2.dwSize; i++)
			r.data[i] = a1.data[i] ^ a2.data[i];
		for(i=a2.dwSize; i<a1.dwSize; i++)
			r.data[i] = a1.data[i];
		}
	else if (a2.dwSize > a1.dwSize)
		{                         
		r.dwSize = a2.dwSize;
		r.data = new uint32[r.dwSize];
		for(i=0; i<a1.dwSize; i++)
			r.data[i] = a1.data[i] ^ a2.data[i];
		for(i=a1.dwSize; i<a2.dwSize; i++)
			r.data[i] = a2.data[i];		
		}                         
	else
		{
		n = a1.dwSize;
		while ((n>0) && (a1.data[n-1] == a2.data[n-1])) n--;
		if (n)
			{
			r.dwSize = n;
			r.data = new uint32[r.dwSize];
			for(i=0; i<n; i++)
				r.data[i] = a1.data[i] ^ a2.data[i]; 
			}
		}

	return r;	
	}

STFBitArray operator- (STFBitArray a1, STFBitArray a2)
	{
	STFBitArray r;
	int32 i, n;
   
   if (a1.dwSize > a2.dwSize)
   	{
		r.dwSize = a1.dwSize;
		r.data = new uint32[r.dwSize];
		for(i=0; i<a2.dwSize; i++)
			r.data[i] = a1.data[i] & ~a2.data[i];
		for(i=a2.dwSize; i<a1.dwSize; i++)
			r.data[i] = a1.data[i];
   	}
   else
   	{
		n = a1.dwSize;
		while ((n>0) && (!(a1.data[n-1] & ~a2.data[n-1]))) n--;
		if (n)
			{
			r.dwSize = n;
			r.data = new uint32[r.dwSize];
			for(i=0; i<n; i++)
				r.data[i] = a1.data[i] & ~a2.data[i]; 
			}
		}

	return r;	
	}                            
	

STFBitArray& STFBitArray::operator|= (STFBitArray a)
	{
	int32 i;
	uint32 * nd;
	//lint --e{613}
	if (dwSize>a.dwSize)
		{
		for(i=0; i<a.dwSize; i++)
			data[i] |= a.data[i];
		}                       
	else
		{       
		nd = new uint32[a.dwSize];
		
		for(i=0; i<dwSize; i++)
			nd[i] = data[i] | a.data[i];
		for(i=dwSize; i<a.dwSize; i++)
			nd[i] = a.data[i];
			
		if (data) delete[] data;
		data = nd;
		}
			
	return *this;
	}

STFBitArray& STFBitArray::operator&= (STFBitArray a)
	{
	int32 i,n;
	uint32 * nd;
	//lint --e{613}
	n = min(dwSize, a.dwSize);              
	while ((n>0) && (!(data[n-1] & a.data[n-1]))) n--;
	if (n != dwSize)
		{
		dwSize = n;
		
		if (n)
			{        
			nd = new uint32[dwSize];
			
			for(i=0; i<dwSize; i++)
				nd[i] = data[i] & a.data[i];
				
			if (data) delete[] data;
			data = nd;			
			}
		else
			{
			if (data) delete[] data;
			data = NULL;
			}
		}              
	else
		{
		for(i=0; i<a.dwSize; i++)
			data[i] &= a.data[i];
		}
	
	return *this;	
	}
	
STFBitArray& STFBitArray::operator^= (STFBitArray a)
	{
	int32 i, n;
	uint32 * nd;
	//lint --e{613}
	if (dwSize>a.dwSize)
		{
		for(i=0; i<a.dwSize; i++)
			data[i] ^= a.data[i];
		}                       
	else if (a.dwSize>dwSize)
		{       
		nd = new uint32[a.dwSize];
		
		for(i=0; i<dwSize; i++)
			nd[i] = data[i] | a.data[i];
		for(i=dwSize; i<a.dwSize; i++)
			nd[i] = a.data[i];
			
		if (data) delete[] data;
		data = nd;
		}                     
	else
		{
		n = dwSize;              
		while ((n>0) && (data[n-1] == a.data[n-1])) n--;
		if (n != dwSize)
			{
			dwSize = n;
			
			if (n)
				{        
				nd = new uint32[dwSize];
				
				for(i=0; i<dwSize; i++)
					nd[i] = data[i] ^ a.data[i];
					
				if (data) delete[] data;
				data = nd;			
				}
			else
				{
				if (data) delete[] data;
				data = NULL;
				}
			}              
		else
			{
			for(i=0; i<a.dwSize; i++)
				data[i] ^= a.data[i];
			}
		}
			
	return *this;
	}

STFBitArray& STFBitArray::operator-= (STFBitArray a)
	{
	int32 i, n;
	uint32 * nd;
	//lint --e{613}
	if (dwSize>a.dwSize)
		{
		for(i=0; i<a.dwSize; i++)
			data[i] &= ~a.data[i];
		}                       
	else
		{
		n = min(dwSize, a.dwSize);              
		while ((n>0) && (!(data[n-1] & ~a.data[n-1]))) n--;
		if (n != dwSize)
			{
			dwSize = n;
			
			if (n)
				{        
				nd = new uint32[dwSize];
				
				for(i=0; i<dwSize; i++)
					nd[i] = data[i] & ~a.data[i];
					
				if (data) delete[] data;
				data = nd;			
				}
			else
				{
				if (data) delete[] data;
				data = NULL;
				}
			}              
		else
			{
			for(i=0; i<a.dwSize; i++)
				data[i] &= ~a.data[i];
			}
		}

	return *this;	
	}
	
		
bool operator== (STFBitArray a1, STFBitArray a2)
	{              
	int32 i;
	//lint --e{613}
	if (a1.dwSize == a2.dwSize)
		{
		for(i=0; i<a1.dwSize; i++)
			if (a1.data[i]!=a2.data[i]) return false;
		return true;	
		}     
	else
		return false;		
	}
	
bool operator!= (STFBitArray a1, STFBitArray a2)
	{
	int32 i;
	//lint --e{613}
	if (a1.dwSize == a2.dwSize)
		{
		for(i=0; i<a1.dwSize; i++)
			if (a1.data[i]!=a2.data[i]) return true;
		return false;	
		}     
	else
		return true;	
	}
	
bool operator<= (STFBitArray a1, STFBitArray a2)
	{
	int32 i;
	//lint --e{613}
	if (a1.dwSize <= a2.dwSize)
		{
		for(i=0; i<a1.dwSize; i++)
			if (a1.data[i] & ~a2.data[i]) return false;
		return true;	
		}     
	else
		return false;	
	}

bool operator>= (STFBitArray a1, STFBitArray a2)
	{
	int32 i;
	//lint --e{613}
	if (a2.dwSize <= a1.dwSize)
		{
		for(i=0; i<a2.dwSize; i++)
			if (a2.data[i] & ~a1.data[i]) return false;
		return true;	
		}     
	else
		return false;	
	}

		
void STFBitArray::INCL(int32 at)
	{ 
	int32 i;
	int32 dw = at >> 5;
	uint32 * nd;
	//lint --e{613}
	//lint --e{661}
	if (dw>=dwSize)
		{
		nd = new uint32[dw+1];
		
		
		for(i=0; i<dwSize; i++)
			nd[i] = data[i];

		dwSize = dw+1;

		if (data) delete[] data;		
		data = nd;		
		}

	data[dw] |= (1UL << (at & 0x1f));
	}

void STFBitArray::EXCL(int32 at)
	{     
	int32 i;
	int32 dw = at >> 5;
	uint32 * nd;
	//lint --e{613}
	//lint --e{661}
	if (dw<dwSize)
		{          
		data[dw] |= (1UL << (at & 0x1f));
		
		if ((dw==dwSize-1) && (!data[dw]))
			{
			while ((dw>0) && (!data[dw-1])) dw--;
			
			dwSize = dw;
			nd = new uint32[dwSize];
			
			
			for(i=0; i<dwSize; i++)
				nd[i] = data[i];
	
			if (data) delete[] data;		
			data = nd;
			}
		}

	}
	
bool STFBitArray::Contains(int32 at)
	{
	int32 dw = at >> 5;
	//lint --e{613}
	//lint --e{661}
	if (dw>=dwSize)
		return false;
	else
		return (data[dw] & (1UL << (dw & 0x1f))) != 0;
	}
