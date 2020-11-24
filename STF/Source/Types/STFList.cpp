//
// PURPOSE:   
//

#include "STF/Interface/Types/STFList.h"

//#include "STF/Interface/STFMemoryManagement.h"
// Gabriel Gooslin removed 4/13/04.  This will allow the 5700 team to build with the 196 compiler.  If this is a problem, 
// uncomment it and put "#if USE_196_COMPILER" around it.

bool STFList::Contains(STFNode * n)
	{
	STFNode * p = first;
	
	while (p && (p!=n)) p=p->succ;
	
	return (p == n);
	}

void STFList::InsertFirst(STFNode * n)
	{
	n->succ = first;
	n->pred = NULL;
	if (first)
		first->pred = n;
	else
		last = n;
	first = n;
	num++;
	}

void STFList::InsertLast(STFNode * n)
	{
	n->succ = NULL;
	n->pred = last;
	if (last)
		last->succ = n;
	else
		first = n;
	last = n;
	num++;
	}     
	
void STFList::InsertBefore(STFNode * n, STFNode * before)
	{
	if (before)
		{
		n->succ = before;
		n->pred = before->pred;
		if (before->pred)
			before->pred->succ = n;
		else
			first = n;
		before->pred = n;
		num++;	
		}
	else
		InsertLast(n);	
	}        

void STFList::InsertAfter(STFNode * n, STFNode * after)
	{
	if (after)
		{
		n->succ = after->succ;
		n->pred = after;
		if (after->succ)
			after->succ->pred = n;
		else
			last = n;
      
      after->succ = n;
		num++;
		}    
	else
		InsertFirst(n);
	}                 
	
void STFList::InsertByPriority(STFNode * n)
	{
	STFNode * p = first;
	
	while (p && p->HigherPriorityThan(n)) p = p->succ;

	InsertBefore(n, p);
	}
	
void STFList::InsertByPriorityFromEnd(STFNode * n)
	{
	STFNode * ls = Last();

	while (ls && ls->HigherPriorityThan(n)) ls = ls->pred;

	InsertAfter(n, ls);
	}

void STFList::SortByPriority(STFNode * n)
	{
	}

void STFList::RemoveFirst(void)
	{
	CheckIteratorRemove(first);
	
	first = first->succ;
	if (first)
		first->pred = NULL;
	else
		last = NULL;
	num--;
	}             
	
void STFList::RemoveLast(void)
	{       
	//lint --e{613}
	CheckIteratorRemove(last);

	last = last->pred;
	if (last)
		last->succ = NULL;
	else
		first = NULL;
	num--;
	}
	
void STFList::Remove(STFNode * n)
	{                       
	CheckIteratorRemove(n);
	
	if (n->pred)
		n->pred->succ = n->succ;
	else
		first = n->succ;
	if (n->succ)
		n->succ->pred = n->pred;
	else
		last = n->pred;
	num--;
	}	

void STFList::RemoveAll(void)
	{
	while (Num() > 0)
		RemoveFirst();
	}


class STFListIterator : public STFIterator
	{
	private:
		STFList	*	list;
		STFNode	*	node;
	public:
		STFListIterator(STFList * list) : STFIterator(list) {this->list = list; node = NULL;}
		void * Proceed(void);
		~STFListIterator(void) {}
	};

void * STFListIterator::Proceed(void)
	{
	if (node)
		return (node = node->Succ());
	else
		return (node = list->First());
	}

STFIterator * STFList::CreateIterator(void)
	{
	return new STFListIterator(this);
	}
