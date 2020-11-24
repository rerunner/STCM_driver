//
// PURPOSE:   Generic linear list classes
//

#ifndef STFLIST_H
#define STFLIST_H

//BUG: Inclusion of the STFMemoryManagement.h leads to multiply defined symbols 
//in the included .cpp files under st20R1.9.6. Needs to be investigated !
//#include "STF/Interface/STFMemoryManagement.h"
#include "STF/Interface/Types/STFBasicTypes.h"	// enter right folder here
#include "STF/Interface/Types/STFIterator.h"				// enter right folder here

class STFList;

class STFNode
	{
	friend class STFList;
	friend class STFListIterator;
		
	private:
		STFNode	*	succ;
		STFNode	*	pred;
	protected:
		virtual bool HigherPriorityThan(STFNode * n) {return false;}
	public:
		STFNode(void) {succ = NULL;pred = NULL;};
		virtual ~STFNode(void) {;};
		
		STFNode * Succ(void) {return succ;};
		STFNode * Pred(void) {return pred;};
		
		bool IsFirst(void) {return pred == NULL;};
		bool IsLast(void) {return succ == NULL;};			
	};

class STFList : public STFIteratorHost
	{  
	private:
		STFNode				*	first;
		STFNode				*	last;
		int32					num;
	public:
		STFList(void) {first = NULL; last = NULL; num = 0;};
		virtual ~STFList(void) {;};
				
		void Clear(void) {first = NULL; last = NULL; num = 0;}

		inline STFNode * First(void) {return first;};
		inline STFNode * Last(void) {return last;};

		inline bool IsEmpty(void) {return first == NULL;};
		bool Contains(STFNode * n);
		int32 Num(void) {return num;};		
		
		void InsertFirst(STFNode * n);
		void InsertLast(STFNode * n);
		void InsertBefore(STFNode * n, STFNode * before);
		void InsertAfter(STFNode * n, STFNode * after);
		
		void InsertByPriority(STFNode * n);
		void InsertByPriorityFromEnd(STFNode * n);
		void SortByPriority(STFNode * n);
		
		void RemoveFirst(void);
		void RemoveLast(void);
		void Remove(STFNode * n);
		void RemoveAll(void);
		
		void Enqueue(STFNode * n) {InsertLast(n);}
		STFNode * Dequeue(void) {STFNode * n = first; if (n) RemoveFirst(); return n;}
		
		void Push(STFNode * n) {InsertFirst(n);}
		STFNode * Pop(void) {return Dequeue();}		
		
		virtual STFIterator * CreateIterator(void);
	};

#endif
