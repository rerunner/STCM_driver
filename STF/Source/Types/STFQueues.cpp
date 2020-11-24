#include "STF/Interface/Types/STFQueue.h"


///////////////////////////////////////////////////////////////////////////////
// Fixed  Queue
///////////////////////////////////////////////////////////////////////////////

typedef void * VOIDPTR;

STFFixedQueue::STFFixedQueue(uint32 size)
	{
	this->size = size;
	mask = 1;
	while (mask < size)
		mask <<= 1;
	buffer = new VOIDPTR[mask];
	mask -= 1;
	Flush ();
	}

STFFixedQueue::~STFFixedQueue(void)
	{
	delete[] buffer;
	}

STFResult STFFixedQueue::Enqueue(void * data)
	{
	STFResult error;
	
	if (last - first < size)
		{
		buffer[last & mask] = data;
		last++;
		              
		error = STFRES_OK;
		}
	else
		error = STFRES_OBJECT_FULL;
	
	return error;
	}

STFResult STFFixedQueue::Dequeue(void * &data)
	{
	STFResult error;
	
	if (first != last)
		{                       
		data = buffer[first & mask];
		first++;
		
		error = STFRES_OK;
		}
	else
		error = STFRES_OBJECT_EMPTY;
	
	return error;
	}

STFResult STFFixedQueue::Peek(void * &data)
	{
	STFResult error;
	
	if (first != last)
		{                       
		data = buffer[first & mask];

		error = STFRES_OK;
		}
	else
		error = STFRES_OBJECT_EMPTY;
	
	return error;
	}

STFResult STFFixedQueue::Flush (void)
	{
	first = 0;
	last = 0;

	STFRES_RAISE_OK;
	}

bool STFFixedQueue::IsEmpty(void)
	{
	return first == last;
	}

bool STFFixedQueue::IsFull(void)
	{
	return last - first == size;
	}
		
uint32 STFFixedQueue::NumElements(void)
	{
	return last - first;
	}






STFFixedIntQueue::STFFixedIntQueue(int32 size)
	{
	this->size = size;
	buffer = new uint32[size];
	first = 0;
	last = 0;
	num = 0;
	}
	
STFFixedIntQueue::~STFFixedIntQueue(void)
	{
	delete[] buffer;
	}
	
		
STFResult STFFixedIntQueue::Enqueue(uint32 data)
	{
	if (num<size)
		{        
		
		buffer[last] = data;
		last = (last + 1) % size;
		num++;
		              
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_FULL);
	}
	
STFResult STFFixedIntQueue::Dequeue(uint32 &data)
	{
	if (num>0)
		{
		data = buffer[first];
		first = (first + 1) % size;
		num--;
		
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}
	
STFResult STFFixedIntQueue::Peek(uint32 & data)
	{
	if (num>0)
		{
		data = buffer[first];
		
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}
	
bool STFFixedIntQueue::IsEmpty(void)
	{
	return num == 0;
	}
	
bool STFFixedIntQueue::IsFull(void)
	{
	return num == size;
	}
	
int32 STFFixedIntQueue::NumElements(void)
	{
	return num;
	}
	
STFResult STFFixedIntQueue::Flush(void)
	{
	first = 0;
	last = 0;
	num = 0;

	STFRES_RAISE_OK;
	}
	

STFFixedDualIntQueue::STFFixedDualIntQueue(int32 size)
	{
	this->size = size;
	buffer = new DualIntEntry[size];
	first = 0;
	last = 0;
	num = 0;
	}
	
STFFixedDualIntQueue::~STFFixedDualIntQueue(void)
	{
	delete[] buffer;
	}
	
		
STFResult STFFixedDualIntQueue::Enqueue(uint32 data1, uint32 data2)
	{
	if (num<size)
		{        
		
		buffer[last].d1 = data1;
		buffer[last].d2 = data2;
		last = (last + 1) % size;
		num++;
		              
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_FULL);
	}
	
STFResult STFFixedDualIntQueue::Dequeue(uint32 & data1, uint32 & data2)
	{
	if (num>0)
		{
		data1 = buffer[first].d1;
		data2 = buffer[first].d2;
		first = (first + 1) % size;
		num--;
		
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}
	
STFResult STFFixedDualIntQueue::Peek(uint32 & data1, uint32 & data2)
	{
	if (num>0)
		{
		data1 = buffer[first].d1;
		data2 = buffer[first].d2;
		
		STFRES_RAISE_OK;
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}
	
bool STFFixedDualIntQueue::IsEmpty(void)
	{
	return num == 0;
	}
	
bool STFFixedDualIntQueue::IsFull(void)
	{
	return num == size;
	}
	
int32 STFFixedDualIntQueue::NumElements(void)
	{
	return num;
	}
	
STFResult STFFixedDualIntQueue::Flush(void)
	{
	first = 0;
	last = 0;
	num = 0;

	STFRES_RAISE_OK;
	}



STFLockedQueue::STFLockedQueue(uint32 size)
	: STFFixedQueue(size)
	{
	}

STFLockedQueue::~STFLockedQueue()
	{
	}

STFResult STFLockedQueue::Enqueue(void * data)
	{
	STFResult	res;

	mutex.Enter();
	res = STFFixedQueue::Enqueue(data);
	mutex.Leave();

	return res;
	}

STFResult STFLockedQueue::Dequeue(void * &data)
	{
	STFResult	res;

	mutex.Enter();
	res = STFFixedQueue::Dequeue(data);
	mutex.Leave();

	return res;
	}

STFResult STFLockedQueue::Peek(void * &data)	
	{
	STFResult	res;

	mutex.Enter();
	res = STFFixedQueue::Peek(data);
	mutex.Leave();

	return res;
	}

STFResult STFLockedQueue::Flush (void)
	{
	STFResult	res;

	mutex.Enter();
	res = STFFixedQueue::Flush();
	mutex.Leave();

	return res;
	}

bool STFLockedQueue::IsEmpty(void)
 	{
	bool	result;

	mutex.Enter();
	result = STFFixedQueue::IsEmpty();
	mutex.Leave();

	return result;
	}

bool STFLockedQueue::IsFull(void)
 	{
	bool	result;

	mutex.Enter();
	result = STFFixedQueue::IsFull();
	mutex.Leave();

	return result;
	}

uint32 STFLockedQueue::NumElements(void)
 	{
	uint32	result;

	mutex.Enter();
	result = STFFixedQueue::NumElements();
	mutex.Leave();

	return result;
	}



////////////////////////////////////////////////////////////////////
// STFInterlockedQueue
////////////////////////////////////////////////////////////////////

STFResult STFInterlockedQueue::Init (uint32 size)
	{
	delete[] buffer;
	delete[] isValid;
	buffer = NULL;
	isValid = NULL;

	// Check if the size is a reasonable power of two (1..2^30).
	for (uint32 i = 1;  i < 0x80000000;  i *= 2)
		{
		if (size == i)
			{
			// Size is correct. Allocate the buffer.
			bufferSize = size;
			buffer = new VOIDPTR[size];
			if (buffer != NULL)
				{
				// Allocate the isValid array.
				isValid = new bool[size];
				if (isValid != NULL)
					STFRES_RAISE(Flush ());

				delete[] buffer;
				}
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
			}
		}
	STFRES_RAISE(STFRES_INVALID_PARAMETERS);
	}



STFResult STFInterlockedQueue::Enqueue (void * data)
	{
	// Note that getting an index from the interlocked counter yields a unique number,
	// which takes care of synchronization even for calls from an interrupt.
	uint32 i = ((uint32)(writeIndex++)) & indexMask;
	//lint --e{613}
	buffer[i] = data;
	isValid[i] = true;   // after this, the entry is valid

	STFRES_RAISE_OK;
	}



STFResult STFInterlockedQueue::Dequeue (void *& data)
	{
	void *d = NULL;
	//lint --e{613}
	mutex.Enter ();

	uint32 i = readIndex & indexMask;
	if (isValid[i])
		{
		// There is another element left.
		d = buffer[i];
		isValid[i] = false;
		readIndex++;
		}

	mutex.Leave ();
	data = d;
	STFRES_RAISE_OK;
	}



STFResult STFInterlockedQueue::Flush (void)
	{
	//lint --e{613}
	mutex.Enter ();
	for (uint32 i = 0;  i < bufferSize;  i++)
		isValid[i] = false;
	indexMask = bufferSize - 1;
	readIndex = 0;
	writeIndex = 0;
	mutex.Leave ();
	STFRES_RAISE_OK;
	}



bool STFInterlockedQueue::IsEmpty (void)
	{
	//lint --e{613}
	return ! isValid[readIndex & indexMask];
	}



bool STFInterlockedQueue::IsFull (void)
	{
	bool isFull = true;
	//lint --e{613}
	mutex.Enter ();
	for (uint32 i = 0;  i < bufferSize;  i++)
		{
		if (! isValid[i])
			isFull = false;
		}
	mutex.Leave ();
	return isFull;
	}



uint32 STFInterlockedQueue::NumElements (void)
	{
	//lint --e{613}
	return (uint32)writeIndex - readIndex;
	}



//************************************************************************
// LinkedQueue

STFResult STFLinkedQueue::Enqueue (STFLinkedQueueElement *element)
	{
	if (element != NULL)
		{
		element->next = NULL;
		inserting = true;
		if (first != NULL)
			first->next = element;
		else
			last = element;
		first = element;
		inserting = false;
		}
	STFRES_RAISE_OK;
	}

STFResult STFLinkedQueue::Dequeue (STFLinkedQueueElement  *  &element)
	{
	element = NULL;
	if (last != NULL)
		{
		if (first != last)
			{
			element = (STFLinkedQueueElement  *)last;
			last = last->next;
			}
		else // first == last
			{
			if (! inserting)
				{
				element = (STFLinkedQueueElement *)last;
				first = last = NULL;
				}
			else
				{
				// Enqueue is concurrently inserting. Assume the queue is empty.
				}
			}
		}
	if (element != NULL)
		STFRES_RAISE_OK;
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);
	}

STFLinkedQueueElement *STFLinkedQueue::Peek (void)
	{
	if (first == last  &&  inserting)
		// Enqueue is concurrently inserting. Assume the queue is empty.
		return NULL;
	else
		return (STFLinkedQueueElement *)last;
	}

////////////////////////////////////////////////////////////

STFIndexedQueue::STFIndexedQueue(uint32 queueSize)
	{
	this->queueSize = queueSize;
	Flush();
	}

STFIndexedQueue::~STFIndexedQueue(void)
	{
	}

uint32 STFIndexedQueue::ItemQueued(void)
	{
	int temp = writePointer;

	mutex.Enter ();
	writePointer = (writePointer +1) % queueSize; 
	numberItems++;
	mutex.Leave ();
	
	return temp;
	}

uint32 STFIndexedQueue::ProcessedQueueItem(void)
	{
	int temp = readPointer;

	mutex.Enter ();
	readPointer = (readPointer +1) % queueSize; 
	numberItems--;
	mutex.Leave ();

	return temp;
	}

uint32 STFIndexedQueue::NumberElements(void)
	{
	mutex.Enter ();
	uint32 threadSafeReturn = numberItems;
	mutex.Leave ();
	return threadSafeReturn;
	}

uint32 STFIndexedQueue::NumberEmpty(void)
	{
	mutex.Enter ();
	uint32 threadSafeReturn = queueSize - numberItems;
	mutex.Leave ();
	return threadSafeReturn;
	}

bool STFIndexedQueue::IsFull(void)
	{
	mutex.Enter ();
	bool threadSafeReturn = (numberItems == queueSize);
	mutex.Leave ();
	return threadSafeReturn;
	}

bool STFIndexedQueue::IsEmpty(void)
	{
	mutex.Enter ();
	bool threadSafeReturn = (numberItems == 0);
	mutex.Leave ();
	return threadSafeReturn;
	}

void STFIndexedQueue::Flush(void)
	{
	mutex.Enter ();
	readPointer = 0;
	writePointer = 0;
	numberItems = 0;
	mutex.Leave ();
	}

STFResult STFIndexedQueue::QueueItem(uint32 &queuePosition)
	{
	STFResult ret = STFRES_OK;
	mutex.Enter ();
	queuePosition = writePointer;	
	if (numberItems == queueSize)
		ret = STFRES_OBJECT_FULL;
	mutex.Leave ();
	STFRES_RAISE(ret);
	}

STFResult STFIndexedQueue::GetNextQueuedItem(uint32 &queuePosition)
	{
	STFResult ret = STFRES_OK;
	mutex.Enter ();
	queuePosition = readPointer;
	if (numberItems == 0)
		ret = STFRES_OBJECT_NOT_FOUND;
	mutex.Leave ();
	STFRES_RAISE(ret);
	}


