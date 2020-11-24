//
// PURPOSE:   Iterators for abstract data types
//

#ifndef STFITERATOR_H
#define STFITERATOR_H

class STFIteratorHost;          
class STFIteratorHandle;

class STFIterator {          
	friend class STFIteratorHandle;    
	friend class STFIteratorHost;
	private:
		STFIteratorHost	*	host;
		STFIterator			*	succ;
		void *					data;
		bool					skipped;
		
		void * InternalProceed(void) {if (skipped) {skipped = false; return data;} else return (data = Proceed());}
		void CheckRemove(void * remove);
		virtual void * Proceed(void) = 0;
	protected:
		inline virtual ~STFIterator(void);
	public:
		STFIterator(STFIteratorHost * host);
	};

class STFIteratorHandle {
	private:
		STFIterator	*	iter;
	public:
		STFIteratorHandle(STFIterator * iter) {this->iter = iter;}
		virtual ~STFIteratorHandle(void) {delete iter;}
		void * Proceed(void) {return iter->InternalProceed();}
	};

class STFIteratorHost 
   {                              
   friend class STFIterator;
   private:
   STFIterator		*	head;
   protected:
   void CheckIteratorRemove(void * data);
   public:             
   STFIteratorHost(void) {head = NULL;}
   virtual ~STFIteratorHost() {}; // NHV: Added for g++ 4.1.1
   virtual STFIterator * CreateIterator(void) = 0;
   };

inline STFIterator::STFIterator(STFIteratorHost * host)
	{   
	this->host = host;
	succ = host->head;
	host->head = this;
	skipped = false;
	data = NULL;
	}

inline void STFIterator::CheckRemove(void * removed)
	{
	if (data == removed)
		{                           
		data = Proceed();
		skipped = true;
		}	
	}

inline void STFIteratorHost::CheckIteratorRemove(void * data)
	{
	STFIterator	*	p = head;

	while (p)
		{
		p->CheckRemove(data);
		p = p->succ;
		}		
	}

inline STFIterator::~STFIterator(void)
	{
	host->head = succ;
	}

			
#define ITERATE(element, host) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while ((void * &)element = _xcrsr.Proceed()) {

#define ITERATE_UNTIL(element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while (((void * &)element = _xcrsr.Proceed()) && !(condition)) {

#define ITERATE_FIND(element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while (((void * &)element = _xcrsr.Proceed()) && !(condition)) ; }
	
#define ITERATE_WHILE(element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while (((void * &)element = _xcrsr.Proceed()) && (condition)) {

#define ITERATE_IF(element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while ((void * &)element = _xcrsr.Proceed()) if (condition) {


#define ITERATE_TYPE(t0, t1, element, host) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while (element = (t1)(t0)(_xcrsr.Proceed())) {

#define ITERATE_UNTIL_TYPE(t0, t1, element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while ((element = (t1)(t0)(_xcrsr.Proceed())) && !(condition)) {

#define ITERATE_FIND_TYPE(t0, t1, element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while ((element = (t1)(t0)(_xcrsr.Proceed())) && !(condition)) ; }
	
#define ITERATE_WHILE_TYPE(t0, t1, element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while ((element = (t1)(t0)(_xcrsr.Proceed())) && (condition)) {

#define ITERATE_IF_TYPE(t0, t1, element, host, condition) \
	{ STFIteratorHandle _xcrsr((host)->CreateIterator()); \
	while (element = (t1)(t0)(_xcrsr.Proceed())) if (condition) {

#define ITERATE_END } }			

#endif
