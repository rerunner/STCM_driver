///
/// @brief  Hash table for STFString key hashing
///

#include "STF/Interface/Types/STFHash.h"
#include "STF/Interface/STFDataManipulationMacros.h"



// STFHash Iterator class...

class STFHashIterator : public STFIterator
	{
	private:
		STFHashNode *  nextNode;
		STFHash     *  hashTable;
		uint32         searchIndex;
	public:
		STFHashIterator(STFHash * pHash);
		void * Proceed(void);
		~STFHashIterator(void) {}
	};


// STFHashNode constructor...

STFHashNode::STFHashNode(STFString k)
	{
	this->next = NULL;	
	this->key = k;
	}

// STFHashNode copy constructor ...

STFHashNode::STFHashNode(STFHashNode & srcNode)
	{
	this->next = NULL;
	this->key = srcNode.key;
	}

//////////////////////////////////////
//
// STFHash class...
//
//////////////////////////////////////

//
// Constructor
//

STFHash::STFHash(uint32 hts)
	{
	this->hashTableSize = hts;

	// create hash table of given size...
	hashTable = new HashNodePtr [hashTableSize];

	// initalize hash table...
	for (uint32 i = 0; i < hashTableSize; i++)
		hashTable[i] = NULL;

   itemCount = 0;
	}

//
// STFHash copy constructor...
//

STFHash::STFHash(STFHash & srcHash)
	{
	// create variables...
	// copy hash values.....
	this->hashTableSize = srcHash.hashTableSize;
	this->hashTable = new HashNodePtr [hashTableSize];
   this->itemCount = srcHash.itemCount;

	// for each table entry....
	for (uint32 i = 0; i < hashTableSize; i++)
		{		
		this->hashTable[i] = NULL;

		if (srcHash.hashTable[i])
			{
			// do copy all list entries and hang into new list...
			STFHashNode * node = srcHash.hashTable[i];
			
			while (node)
				{
				STFHashNode * newNode = new STFHashNode(*node);
				newNode->next = this->hashTable[i];
				hashTable[i] = newNode;
				
				node = node->next;
				}
			}				
		}
	}

//
// Destructor
//

STFHash::~STFHash()
	{
	// delete all hash table entries...
	for (uint32 i = 0; i < hashTableSize; i++)
		{
		STFHashNode * node = hashTable[i];
		STFHashNode * oldNode = NULL;

		// delete all elements in collission queue...
		while (node)
			{
			oldNode = node;
			node = node->next;
			delete oldNode;
			}
		}

	delete[] hashTable;
	}

//
// Hash function
//

uint32 STFHash::Hash(STFString key)
	{	
	unsigned int hh = 0x1213241;

	char * p = (char*) key;

	while (*p)
		{
		hh = (hh << 1) ^ *p ^ (hh >> 16);
		p++;
		}

	return hh % this->hashTableSize;

	}

// 
// make new entry in hash table...
//

bool STFHash::Enter(STFHashNode * pNode, STFString key)
	{
	// generate hash value....
	uint32 hashValue = Hash(key);
		
	// search for key in hash table...
	STFHashNode * hashNode = hashTable[hashValue];
	while (hashNode && ( key != hashNode->key))
		hashNode = hashNode->next;

	if (hashNode)
		return false;

	// generate new hashtable entry ...
	STFHashNode * newHashEntry = pNode;
	newHashEntry->key = key;

	// set as first element in collision queue...
	newHashEntry->next = hashTable[hashValue];
	hashTable[hashValue] = newHashEntry;

   // increase number of items;
	itemCount++;

	return true;
	}

//
// remove element from hash table...
//

STFHashNode * STFHash::Remove(STFString key)
	{
	// generate hash value....
	uint32 hashValue = Hash(key);
	
	// search for key in hash table...
	STFHashNode * hashNode = hashTable[hashValue];
	STFHashNode * lastNode = NULL;
	while (hashNode && (key != hashNode->key))
		{
		lastNode = hashNode;
		hashNode = hashNode->next;
		}

	// if no entry found return null pointer...
	if (!hashNode) 
		return NULL;

	// repair collision queue...
	if (lastNode)
		lastNode->next = hashNode->next;
	else
		hashTable[hashValue] = hashNode->next;


   // decrease itemCount;
	itemCount--;

	// return result...
	return hashNode;
	}

// 
// look up hash table..
//

STFHashNode * STFHash::LookUp(STFString key)
	{
	// generate hash value...
	uint32 hashValue = Hash(key);

	// search for key in has table...
	STFHashNode * hashNode = hashTable[hashValue];
	while (hashNode && (key != hashNode->key))
		hashNode = hashNode->next;

	if (hashNode)
		return hashNode;
	else
		return NULL;
	}


STFIterator * STFHash::CreateIterator(void)
	{
	return new STFHashIterator(this);
	}



//////////////////////////////////////
//
// STFHashIterator class...
//
//////////////////////////////////////


STFHashIterator::STFHashIterator(STFHash * pHash) : STFIterator((STFIteratorHost*) pHash)
	{
	this->hashTable = pHash;
	this->searchIndex = 0;
	this->nextNode = NULL;
	}

void * STFHashIterator::Proceed(void)
	{
	// get down collision list...
	if (nextNode != NULL)
		nextNode = nextNode->next;
	
	// go to next element...
	if (!nextNode)
		{
		uint32 tabSize = this->hashTable->hashTableSize;
		while ((searchIndex < tabSize) && (!(nextNode = this->hashTable->hashTable[searchIndex++])))
			;		
		}

	// something found?
	if (!nextNode)
		searchIndex = 0;

	return nextNode;
	}

