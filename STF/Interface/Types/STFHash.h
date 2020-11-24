///
/// @brief  Hashing table for STFString key hashing
///

#ifndef STFHASH_H
#define STFHASH_H

#include "STF/Interface/Types/STFString.h"
#include "STF/Interface/Types/STFIterator.h"


#define defaultHashTableSize 32


// Entry in STFHash list....
class STFHashNode
	{
	friend class STFHash;
	friend class STFHashIterator;

	protected:		
		STFString key;			
		STFHashNode * next;	
		STFHashNode(STFString key);
		STFHashNode(STFHashNode &);
	};


typedef STFHashNode * HashNodePtr;




//
// STFHash class...
//

//! Hash table using STFStrings as keys
/*! This class uses STFStrings as keys into a hashtable. 
	The hash table is a fixed size table. Every key must be unique in 
	the hashtable. Collissions (keys mapped to the same table entry) are 
	resolved with linked lists.
*/
class STFHash : public STFIteratorHost
	{

	friend class STFHashIterator;
	
	private:		
		STFHashNode ** hashTable;
	
	protected:
		uint32 hashTableSize;
      uint32 itemCount;

		// hash fuction....
		uint32 Hash(STFString key);
      
	public:
		//! Constructor
		/*! Creates a hash. 
			\param hashTableSize The size of the hashtable. Default size is 32			
		*/
		STFHash(uint32 hashTableSize = defaultHashTableSize);

		//! Copy constructor ...
		/*! Creates a copy of the hash given as parameter
			\param value The hashtable to copy
		*/
		STFHash(STFHash & value);

		//! Destructor ....
		virtual ~STFHash();

      /// Return the number of elements in the hashtable
      uint32 GetItemNum() {return itemCount;}

		//! Enter an element to the hash
		/*! Enter an element to hash table under a key. 
			There MUST be no other element in the hash with the same key.
			\param pHash The hash entry to add
			\param key   The key under wich to add the key
			\returns		 true if added, false otherwise
		*/
		bool Enter(STFHashNode * pHash, STFString key);		

		//! Remove an element from the hash
		/*! Remove an element with a specified key
			Returns the removed node.
			\param key   The key to find the node for
			\returns     The element found under this key, NULL otherwise
		*/
		STFHashNode* Remove(STFString key);
		
		//! Return an element with a specific key
		/*! Searches an element with a specific key and returns it.		
			\param key   The key to find the node for
			\returns     The element found under this key, NULL otherwise
		*/
		STFHashNode* LookUp(STFString key);

		//! Create an iterator to iterate through the elements of hash
		/*! 
			\return The iterator.
		*/
		STFIterator * CreateIterator(void);
	};


#endif // STFHASH_H
