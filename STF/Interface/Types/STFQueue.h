///
/// @brief      Generic queue implementations
///

#ifndef STFQUEUE_INC
#define STFQUEUE_INC

//BUG: Inclusion of the STFMemoryManagement.h leads to multiply defined symbols 
//in the included .cpp files under st20R1.9.6. Needs to be investigated !
//#include "STF/Interface/STFMemoryManagement.h"
#include "STF/Interface/STFSynchronisation.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFBasicTypes.h"
 
//! Queue class (FIFO)
class STFQueue
	{
	public:
		//! Default virtual destructor
		virtual ~STFQueue(void) {};

		//! Add element to queue
		/*! Adds a new element to the end of the queue.
		*/
		virtual STFResult Enqueue(void * data) = 0;

		//! Returns the first element of the queue
		/*! Returns the element at the beginning of the queue and removes
			this entry from the queue. 
			\param data  The result of the dequeue operation
			\returns     Error code
		*/
		virtual STFResult Dequeue(void * &data) = 0;

		//! Flush the queue
		/*! Flush all data from the queue
		*/
		virtual STFResult Flush (void) = 0;

		//! Test if queue is empty
		/*! Returns true if queue is empty, false otherwise
		*/
		virtual bool IsEmpty(void) = 0;

		//! Test if queue is full
		/*! Returns true if queue is full, false otherwise
		*/
		virtual bool IsFull(void) = 0;

		//! Get the number of elements currently in the queue
		/*! Returns the number of elements currently in the queue
		*/
		virtual uint32 NumElements(void) = 0;
	};



class STFFixedQueue : public STFQueue
	{
	private:
		void 		**	buffer;
		uint32		first, last, size, mask;
	public:
		STFFixedQueue(uint32 size);
		~STFFixedQueue(void);

		virtual STFResult Enqueue(void * data);
		virtual STFResult Dequeue(void * &data);
		virtual STFResult Peek(void * &data);
		virtual STFResult Flush (void);
		virtual bool IsEmpty(void);
		virtual bool IsFull(void);
		virtual uint32 NumElements(void);
	};



class STFLockedQueue : public STFFixedQueue
	{
	private:
		STFMutex mutex;
	public:
		STFLockedQueue(uint32 size);
		~STFLockedQueue();

		virtual STFResult Enqueue(void * data);
		virtual STFResult Dequeue(void * &data);
		virtual STFResult Peek(void * &data);
		virtual STFResult Flush (void);
		virtual bool IsEmpty(void);
		virtual bool IsFull(void);
		virtual uint32 NumElements(void);
	};



class STFIntQueue
	{
	public:
		virtual ~STFIntQueue(void) 
			{
			}
		virtual STFResult Enqueue(uint32 data) = 0;
		virtual STFResult Dequeue(uint32 &data) = 0;
		virtual STFResult Peek(uint32 & data) = 0;
		virtual bool IsEmpty(void) = 0;
		virtual bool IsFull(void) = 0;
		virtual int32 NumElements(void) = 0;
		virtual STFResult Flush(void) = 0;
	};



class STFFixedIntQueue : public STFIntQueue
	{
	private:
		uint32		*	buffer;
		int32			first, last, size, num;

	public:
		STFFixedIntQueue(int32 size);
		~STFFixedIntQueue(void);

		STFResult Enqueue(uint32 data);
		STFResult Dequeue(uint32  &data);
		STFResult Peek(uint32 & data);
		bool IsEmpty(void);
		bool IsFull(void);
		int32 NumElements(void);
		STFResult Flush(void);
	};



class STFDualIntQueue 
	{
	public:
		virtual ~STFDualIntQueue(void) {}
		virtual STFResult Enqueue(uint32 data1, uint32 data2) = 0;
		virtual STFResult Dequeue(uint32  &data1, uint32  &data2) = 0;
		virtual STFResult Peek(uint32 & data1, uint32 &data2) = 0;
		virtual bool IsEmpty(void) = 0;
		virtual bool IsFull(void) = 0;
		virtual int32 NumElements(void) = 0;
		virtual STFResult Flush(void) = 0;
	};



class STFFixedDualIntQueue : public STFDualIntQueue
	{
	private:
		struct DualIntEntry 
			{
			uint32	d1,d2;
			}	*	buffer;
		int32			first, last, size, num;
	public:
		STFFixedDualIntQueue(int32 size);
		~STFFixedDualIntQueue(void);

		STFResult Enqueue(uint32 data1, uint32 data2);
		STFResult Dequeue(uint32  &data1, uint32 &data2);
		STFResult Peek(uint32 &data1, uint32 &data2);
		bool IsEmpty(void);
		bool IsFull(void);
		int32 NumElements(void);
		STFResult Flush(void);
	};



//! Interlocked queue class (FIFO).
/*! Enqueue() can be called even from an interrupt, but the other operations are only possible
	 from tasks/threads. */

class STFInterlockedQueue : public STFQueue
	{
	protected:
		void				**	buffer;
		bool				*	isValid;
		uint32				bufferSize, indexMask, numEntries;

		STFMutex				mutex;

		uint32				readIndex;
		STFInterlockedInt	writeIndex;

	public:
		STFInterlockedQueue (void) { buffer = NULL;  isValid = NULL;  bufferSize = 0; }
		virtual ~STFInterlockedQueue (void) { delete[] isValid;  delete[] buffer; }

		//! For the size, a power of two must be specified. Otherwise an error is returned.
		virtual STFResult Init (uint32 size);

		//! Enqueue an element. This can be called from an interrupt.
		virtual STFResult Enqueue (void * data);

		virtual STFResult Dequeue (void *& data);
		virtual STFResult Flush (void);
		virtual bool IsEmpty (void);
		virtual bool IsFull (void);
		virtual uint32 NumElements (void);
	};



// LinkedQueue supports interrupt-safe access to a singly linked queue list.
// Please note: To work safely, the interrupt is only allowed to Dequeue().

class STFLinkedQueueElement
	{
	friend class STFLinkedQueue;
	protected:
		STFLinkedQueueElement *next;
	public:
		STFLinkedQueueElement (void) 
			{
			next = NULL;
			}
		virtual ~STFLinkedQueueElement (void) {}
	};



class STFLinkedQueue
	{
	private:
		volatile STFLinkedQueueElement *first;
		volatile STFLinkedQueueElement *last;
		volatile bool inserting;
	public:
		STFLinkedQueue (void) 
			{
			first = last = NULL; inserting = false;
			}
		
		virtual ~STFLinkedQueue (void) 
			{
			}
		
		virtual STFResult Enqueue (STFLinkedQueueElement * element);
		
		virtual STFResult Dequeue (STFLinkedQueueElement * &element);
		
		virtual STFResult Flush (void) 
			{
			first = last = NULL; 
			STFRES_RAISE_OK;
			}
		virtual bool IsEmpty (void) 
			{
			return (last==NULL);
			}
		
		// IsFull does not exist since there is no limit.
		virtual STFLinkedQueueElement *Peek (void);
	};

class STFIndexedQueue
	{
	public:

		STFIndexedQueue(uint32 queueSize);
		virtual ~STFIndexedQueue(void);

		virtual void				Flush(void);
		virtual uint32				NumberElements(void);
		virtual uint32				NumberEmpty(void);
		virtual bool				IsFull(void);
		virtual bool				IsEmpty(void);
		virtual STFResult			QueueItem(uint32 &queuePosition);
		virtual uint32				ItemQueued(void);
		virtual STFResult			GetNextQueuedItem(uint32 &queuePosition);
		virtual uint32				ProcessedQueueItem(void);

	private:
		
		uint32						queueSize;
		uint32						numberItems;	
		uint32						readPointer;
		uint32						writePointer;
		STFMutex					mutex;
	};



#endif // STFQUEUE_INC
