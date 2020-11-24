///
/// @brief Interface for configuration profiles.
///

#include "STF/Interface/STFProfile.h"

///////////////////////
// STFGenericProfile
///////////////////////

STFGenericProfile::STFGenericProfile (void)
	{
	size = 0;
	dataContainer = NULL;
	}

STFGenericProfile::STFGenericProfile(uint32 size, uint8 * data)
	{
	this->size = size;

	dataContainer = data;

	if (dataContainer != NULL)
		ASSERT(size != 0);
	}

STFGenericProfile::~STFGenericProfile() {}

STFResult STFGenericProfile::GetDataBlock (uint32 & size, uint8 *& data)
	{
	size = this->size;
	data = dataContainer;
	STFRES_RAISE_OK;
	}


///////////////////////
// STFProfile
///////////////////////

STFProfile::STFProfile (void)	: STFGenericProfile() {}

STFProfile::STFProfile (uint32 size, uint8 * data) : STFGenericProfile(size, data)
	{
	/*
	parentKey = STFString("");

	parentProfile = NULL;
	*/
	}

STFProfile::~STFProfile (void) {}

/*
STFProfile::STFProfile (STFString key, STFProfile * root) : STFGenericProfile (0, NULL)
	{
	parentKey = key;

	parentProfile = root;
	}


STFResult STFProfile::Initialize (void)
	{
	STFRES_RAISE_OK;
	}

STFProfile* STFProfile::GetSubProfile (STFString key)
	{
	if (parentProfile == NULL)
		return new STFProfile (key, this);
	else
		return new STFProfile (parentKey + key, parentProfile);
	}
*/

STFResult STFProfile::Write (STFString key, STFString name, int32 value)
	{
	STFResult	res			= STFRES_OK;
	uint32		position		= 0;
	uint8		*	nodes			= new uint8[128];
	uint8			numOfNodes	= 0;
	//uint8			keyNumber	= 0;
	//uint32		data			= 0;
	STFString	keyElement	= STFString ("");
	STFString	keyPart		= STFString ("");
	STFString	insertKey	= STFString ("");

	if (dataContainer == NULL)
		{
		res = STFRES_OK;
		}
	else
		{
		res = FindEntry(key, name, position);
		if (res == STFRES_OK) //Entry exists
			{
			res = ChangeIEntryAt (key, name, (uint32)value);
			}
		else if (res == STFRES_OBJECT_NOT_FOUND) //Entry does not exist
			{
			res = GetKeyStructure (key, nodes, numOfNodes);

			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 i = 0; i < numOfNodes; i++)
					{
					keyPart = STFString("");
					position = 0;

					for (uint32 j = 0; j <= i; j++)
						{
						if (STFRES_SUCCEEDED (res))
							{
							res = GetKeyElement(key, nodes, j, keyElement); 
							keyPart = keyPart + keyElement + STFString("/");
							}
						}
					if (STFRES_SUCCEEDED (res))
						{
						res = FindKey (keyPart, position);

						if (res == STFRES_OBJECT_NOT_FOUND)
							{
							insertKey = keyPart.Head (keyPart.Length() - keyElement.Length() - 1);
							if (insertKey.Length() == 0)
								insertKey = STFString("");
							res = InsertNodeAt (insertKey, keyElement);
							}
						}
					}
				}

			res =  InsertIEntryAt (key, name, (uint32)value);	
			}
		else //Something is very wrong
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	delete nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::Read  (STFString key, STFString name, int32 & value, int32 deflt)
	{
	STFResult	res;
	uint32		position = 0;
	uint32		data		= 0;

	value = deflt;

	if (dataContainer == NULL)
		STFRES_RAISE_OK;

	res = FindEntry (key, name, position);
	
	if (STFRES_SUCCEEDED(res))
		{
		res = GetIntegerValueAt (position, data);
		if (STFRES_SUCCEEDED(res))
			value = (int32)data;
		}
	else
		{
		value = deflt;
		res = STFRES_OBJECT_NOT_FOUND;
		}

	STFRES_RAISE(res);
	}
		
STFResult STFProfile::Write (STFString key, STFString name, bool value)
	{
	uint32		uiValue		= (value == true ? 1 : 0);

	STFResult	res			= STFRES_OK;
	uint32		position		= 0;
	uint8		*	nodes			= new uint8[128];
	uint8			numOfNodes	= 0;
	//uint8			keyNumber	= 0;
	//uint32		data			= 0;
	STFString	keyElement	= STFString ("");
	STFString	keyPart		= STFString ("");
	STFString	insertKey	= STFString ("");

	if (dataContainer == NULL)
		{
		res = STFRES_OK;
		}
	else
		{
		res = FindEntry(key, name, position);
		if (res == STFRES_OK) //Entry exists
			{
			res = ChangeIEntryAt (key, name, uiValue);
			}
		else if (res == STFRES_OBJECT_NOT_FOUND) //Entry does not exist
			{
			res = GetKeyStructure (key, nodes, numOfNodes);

			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 i = 0; i < numOfNodes; i++)
					{
					keyPart = STFString("");
					position = 0;

					for (uint32 j = 0; j <= i; j++)
						{
						if (STFRES_SUCCEEDED (res))
							{
							res = GetKeyElement(key, nodes, j, keyElement); 
							keyPart = keyPart + keyElement + STFString("/");
							}
						}
					if (STFRES_SUCCEEDED (res))
						{
						res = FindKey (keyPart, position);

						if (res == STFRES_OBJECT_NOT_FOUND)
							{
							insertKey = keyPart.Head (keyPart.Length() - keyElement.Length() - 1);
							if (insertKey.Length() == 0)
								insertKey = STFString("");
							res = InsertNodeAt (insertKey, keyElement);
							}
						}
					}
				}

			res =  InsertIEntryAt (key, name, uiValue);	
			}
		else //Something is very wrong
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	delete nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::Read  (STFString key, STFString name, bool & value, bool deflt)
	{
	STFResult	res;
	uint32		position = 0;
	uint32		data		= 0;

	value = deflt;

	if (dataContainer == NULL)
		STFRES_RAISE_OK;

	res = FindEntry (key, name, position);
	
	if (STFRES_SUCCEEDED(res))
		{
		res = GetIntegerValueAt (position, data);
		if (STFRES_SUCCEEDED(res))
			value = (data > 0 ? true : false);
		}
	else
		{
		value = deflt;
		res = STFRES_OBJECT_NOT_FOUND;
		}

	STFRES_RAISE(res);
	}
		
STFResult STFProfile::Write (STFString key, STFString name, uint32 value)
	{
	STFResult	res			= STFRES_OK;
	uint32		position		= 0;
	uint8		*	nodes			= new uint8[128];
	uint8			numOfNodes	= 0;
	//uint8			keyNumber	= 0;
	//uint32		data			= 0;
	STFString	keyElement	= STFString ("");
	STFString	keyPart		= STFString ("");
	STFString	insertKey	= STFString ("");

	if (dataContainer == NULL)
		{
		res = STFRES_OK;
		}
	else
		{
		res = FindEntry(key, name, position);
		if (res == STFRES_OK) //Entry exists
			{
			res = ChangeIEntryAt (key, name, value);
			}
		else if (res == STFRES_OBJECT_NOT_FOUND) //Entry does not exist
			{
			res = GetKeyStructure (key, nodes, numOfNodes);

			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 i = 0; i < numOfNodes; i++)
					{
					keyPart = STFString("");
					position = 0;

					for (uint32 j = 0; j <= i; j++)
						{
						if (STFRES_SUCCEEDED (res))
							{
							res = GetKeyElement(key, nodes, j, keyElement); 
							keyPart = keyPart + keyElement + STFString("/");
							}
						}
					if (STFRES_SUCCEEDED (res))
						{
						res = FindKey (keyPart, position);

						if (res == STFRES_OBJECT_NOT_FOUND)
							{
							insertKey = keyPart.Head (keyPart.Length() - keyElement.Length() - 1);
							if (insertKey.Length() == 0)
								insertKey = STFString("");
							res = InsertNodeAt (insertKey, keyElement);
							}
						}
					}
				}

			res =  InsertIEntryAt (key, name, value);	
			}
		else //Something is very wrong
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	delete nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::Read  (STFString key, STFString name, uint32 & value, uint32 deflt)
	{
	STFResult	res;
	uint32		data;
	uint32		position = 0;

	value = deflt;

	if (dataContainer == NULL)
		STFRES_RAISE_OK;

	res = FindEntry (key, name, position);
	
	if (STFRES_SUCCEEDED(res))
		{
		res = GetIntegerValueAt (position, data);
		if (STFRES_SUCCEEDED(res))
			value = data;
		}
	else
		{
		//value = deflt;
		res = STFRES_OBJECT_NOT_FOUND;
		}
	
	STFRES_RAISE(res);
	}
		
STFResult STFProfile::Write (STFString key, STFString name, STFString value)
	{
	STFResult	res			= STFRES_OK;
	uint32		position		= 0;
	uint8		*	nodes			= new uint8[128];
	uint8			numOfNodes	= 0;
	//uint8			keyNumber	= 0;
	//uint32		data			= 0;
	STFString	keyElement	= STFString ("");
	STFString	keyPart		= STFString ("");
	STFString	insertKey	= STFString ("");

	if (dataContainer == NULL)
		{
		res = STFRES_OK;
		}
	else
		{
		res = FindEntry(key, name, position);
		if (res == STFRES_OK) //Entry exists
			{
			res = ChangeSEntryAt (key, name, value);
			}
		else if (res == STFRES_OBJECT_NOT_FOUND) //Entry does not exist
			{
			res = GetKeyStructure (key, nodes, numOfNodes);

			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 i = 0; i < numOfNodes; i++)
					{
					keyPart = STFString("");
					position = 0;

					for (uint32 j = 0; j <= i; j++)
						{
						if (STFRES_SUCCEEDED (res))
							{
							res = GetKeyElement(key, nodes, j, keyElement); 
							keyPart = keyPart + keyElement + STFString("/");
							}
						}
					if (STFRES_SUCCEEDED (res))
						{
						res = FindKey (keyPart, position);

						if (res == STFRES_OBJECT_NOT_FOUND)
							{
							insertKey = keyPart.Head (keyPart.Length() - keyElement.Length() - 1);
							if (insertKey.Length() == 0)
								insertKey = STFString("");
							res = InsertNodeAt (insertKey, keyElement);
							}
						}
					}
				}

			res =  InsertSEntryAt (key, name, value);	
			}
		else //Something is very wrong
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	delete nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::Read  (STFString key, STFString name, STFString& value, STFString deflt)
	{
	STFResult	res;
	STFString	data;
	uint32		position = 0;

	value = deflt;

	if (dataContainer == NULL)
		STFRES_RAISE_OK;

	res = FindEntry (key, name, position);
	
	if (STFRES_SUCCEEDED(res))
		{
		res = GetStringValueAt (position, data);
		if (STFRES_SUCCEEDED(res))
			value = data;
		}
	else
		{
		//value = deflt;
		res = STFRES_OBJECT_NOT_FOUND;
		}

	STFRES_RAISE(res);
	}


STFResult STFProfile::GetKeyStructure (STFString key, uint8 *& nodes, uint8 & totalNum)
	{
	uint8 position = 0;
	//lint --e{661}
        //lint --e{662}
	//Fill array with 0s
	//nodes = new uint8[128]; //Should not be needed as array is created before this method is called.
	ASSERT (nodes != NULL);

   for (uint32 i = 0;  i < 128;  i++)
		nodes[i] = 0;

	totalNum = 0;

	//Cut "/" when it appears at start or end of string
	if (key.First (STFString("/")) == 0)
		key = key.Tail (key.Length() - 1);

	if (key.Last (STFString("/")) == key.Length() - 1)
		key = key.Head (key.Length() - 1);

	
	//key is empty
	if (key.Length() == 0)
		{
		//STFRES_RAISE(STFRES_OBJECT_EMPTY);
		totalNum = 0;
		STFRES_RAISE_OK;
		}
	
	
	totalNum = 1;

	while (position < key.Length() - 1)
		{
		position = key.Next (STFString("/"), position);

		if (position == 255)
			STFRES_RAISE_OK;

		nodes[totalNum] = position + 1;

		totalNum++;
		}

	STFRES_RAISE_OK;
	}

STFResult STFProfile::GetKeyElement (STFString key, uint8 * nodes, uint8 num, STFString & element)
	{
	element = STFString ("");
	//First check if key is empty and wanted key element is valid
	if (key.Length() == 0)
		{
		//STFRES_RAISE(STFRES_OBJECT_EMPTY);
		STFRES_RAISE_OK;
		}

	if (nodes[num] == 0 && nodes[num+1] == 0 && num != 0) //???What happens when only one element exists???
		STFRES_RAISE (STFRES_RANGE_VIOLATION);

	//Cut "/" when it appears at start or end of string
	if (key.First (STFString("/")) == 0)
		key = key.Tail (key.Length() - 1);

	if (key.Last (STFString("/")) == key.Length() - 1)
		key = key.Head (key.Length() - 1);

	//Now cut away everything from key that does not belong to wanted element.
	if (num != 0)
		key = key.Tail (key.Length() - nodes[num]);
	
	if (nodes[num+1] != 0)
		key = key.Head (nodes[num+1] - nodes[num] - 1);

	element = key;

	STFRES_RAISE_OK;
	}

STFResult STFProfile::FindKey	(STFString key, uint32 & position)
	{
	STFString	actualElement;
	uint32		elementSize		= 0;
	uint32		siblingOffset	= 0;
	STFString	keyElement;
	STFString	lastKeyElement;
	bool			keyFound			= false;
	uint8		*	nodes				= new uint8[128];
	uint8			numOfNodes		= 0;
	uint8			keyNumber		= 0;
	uint32		data				= 0;
	uint32		profileSize		= 0;
	STFResult	res				= STFRES_OK;

	res = GetProfileUsageSize (profileSize);

	if (STFRES_SUCCEEDED (res))
		{
		res = GetKeyStructure (key, nodes, numOfNodes);
		}
	if (STFRES_SUCCEEDED (res))
		{
		res = GetKeyElement (key, nodes, numOfNodes-1, lastKeyElement);
		}
	
	if (STFRES_SUCCEEDED (res))
		{
		//1. Start at position 0 -> actualElement = root (root element is not part of key!)
		position = 0;

		if (key.Length() == 0)
			{
			res = STFRES_OK;
			}
		else
			{
			res = GetNameAt (position, actualElement);

			if (STFRES_SUCCEEDED (res))
				{
				if (actualElement != STFString("root"))
					{
					res = STFRES_OBJECT_INVALID;
					}
				}

			if (STFRES_SUCCEEDED (res))
				{
				res = GetElementSize (position, elementSize);
				position += elementSize;
				}

			if (STFRES_SUCCEEDED (res))
				{
				//2. Do
				do
					{
					if (STFRES_SUCCEEDED (res))
						{
						res = GetKeyElement (key, nodes, keyNumber, keyElement);
						}

					//2a. Go through siblings of actualElement and find next keyElement among them
					if (STFRES_SUCCEEDED (res))
						{
						res = Read4Bytes (position, data);
						}

					if (STFRES_SUCCEEDED (res))
						{
						if (XTBF(31, data)) //This is a node,
							{
							res = GetNameAt (position, actualElement);

							if (STFRES_SUCCEEDED (res))
								{
								if (actualElement == keyElement) //keyElement found. Go one step deeper in tree.
									{
									keyNumber++;
									res = GetElementSize (position, elementSize);
									position += elementSize;
									keyFound = true;
									}
								else //keyElement not found. Go to next sibling.
									{
									//siblingOffset = ((data << 1) >> 1);
									siblingOffset = data & 0x7fffffff;
									position += siblingOffset;
									keyFound = false;

									//2b. When keyElement is not found return error
									if (siblingOffset == 0)
										res = STFRES_OBJECT_NOT_FOUND;
									}
								}
							}
						else // This is an entry
							{
							res = GetElementSize (position, elementSize);
							position += elementSize;
							keyFound = false;
							}
						}
					}
				while (	(actualElement != lastKeyElement) && 
							(keyNumber < numOfNodes) && 
							(position < profileSize) && 
							(res != STFRES_OBJECT_NOT_FOUND)	); // While lastkeyElement is found
				}

			if (STFRES_SUCCEEDED (res))
				{
				if (keyFound)
					{
					position -= elementSize;
					res = STFRES_OK;
					}
				else //2b. When keyElement is not found return error
					{
					res = STFRES_OBJECT_NOT_FOUND;
					}
				}
			}
		}

	delete[] nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::FindEntryAt (STFString name, uint32 & position)
	{
	STFString	actualElement;
	uint32		elementSize	= 0;
	uint32		data			= 0;
	//3. Go through siblings of actualElement and find leaf with name

	STFRES_REASSERT (GetElementSize(position, elementSize));

	do
		{
		position += elementSize;
		STFRES_REASSERT (Read4Bytes (position, data));
		
		if (XTBF(31, data)) //This is a node,
			{
			STFRES_RAISE (STFRES_OBJECT_NOT_FOUND);
			}
		else // This is an entry
			{
			STFRES_REASSERT (GetNameAt (position, actualElement));
			}
		
		STFRES_REASSERT (GetElementSize (position, elementSize));	
		}
		while(actualElement != name);

	STFRES_RAISE_OK;
	}

STFResult STFProfile::FindEntry (STFString key, STFString name, uint32 & position)
	{
/*
	STFString	actualElement;
	uint32		elementSize		= 0;
	uint32		siblingOffset	= 0;
	STFString	keyElement;
	STFString	lastKeyElement;
	bool			keyFound			= false;
	//bool		entryVisited	= false;
	//STFResult	res;
	uint8		*	nodes				= new uint8[128];
	uint8			numOfNodes		= 0;
	uint8			keyNumber		= 0;
	uint32		data				= 0;
	

	STFRES_REASSERT(GetKeyStructure (key, nodes, numOfNodes));
	STFRES_REASSERT(GetKeyElement (key, nodes, numOfNodes-1, lastKeyElement));
	
	//1. Start at position 0 -> actualElement = root (root element is not part of key!)
	position = 0;
	STFRES_REASSERT (GetNameAt (position, actualElement));
	
	if (actualElement != STFString("root"))
		{
		STFRES_RAISE(STFRES_OBJECT_INVALID);
		}
	
	STFRES_REASSERT(GetElementSize (position, elementSize));
	position += elementSize;
	
	//2. Do
	do
		{
		STFRES_REASSERT (GetKeyElement (key, nodes, keyNumber, keyElement));
		
		//2a. Go through siblings of actualElement and find next keyElement among them
		STFRES_REASSERT (Read4Bytes (position, data));
		
		if (XTBF(31, data)) //This is a node,
			{
			STFRES_REASSERT (GetNameAt (position, actualElement));
			}
		else // This is an entry
			{
			STFRES_REASSERT (GetElementSize (position, elementSize));
			position += elementSize;
			continue;
			}
		
		if (actualElement == keyElement) //keyElement found. Go one step deeper in tree.
			{
			keyNumber++;
			STFRES_REASSERT (GetElementSize (position, elementSize));
			position += elementSize;
			keyFound = true;
			}
		else //keyElement not found. Go to next sibling.
			{
			//siblingOffset = ((data << 1) >> 1);
			siblingOffset = data & 0x7fffffff;
			position += siblingOffset;
			keyFound = false;
			
			//2b. When keyElement is not found return error
			if (siblingOffset == 0)
				STFRES_RAISE (STFRES_OBJECT_NOT_FOUND);
			}
		}
	while ((actualElement != lastKeyElement) || (keyNumber < numOfNodes)); // While lastkeyElement is found
	
		
	if (keyFound)
		{
		elementSize = 0;
		//3. Go through siblings of actualElement and find leaf with name
		do
			{
			position += elementSize;
			STFRES_REASSERT (Read4Bytes (position, data));
			
			if (XTBF(31, data)) //This is a node,
				{
				//if (entryVisited)
				STFRES_RAISE (STFRES_OBJECT_NOT_FOUND);
				}
			else // This is an entry
				{
				STFRES_REASSERT (GetNameAt (position, actualElement));
				//entryVisited = true;
				}
			
			STFRES_REASSERT (GetElementSize (position, elementSize));	
			}
			while(actualElement != name);
		}
	else //2b. When keyElement is not found return error
		{
		STFRES_RAISE (STFRES_OBJECT_NOT_FOUND);
		}
	
	//4. return position of this leaf.
	
*/
	STFRES_REASSERT (FindKey(key, position));
	STFRES_REASSERT (FindEntryAt(name, position));

	STFRES_RAISE_OK;
	}

STFResult STFProfile::IsChildOf (STFString key, uint32 possiblePosition, bool & isCh)
	{
	uint32		position						= 0;
	uint32		offset						= 0;
	uint8		*	nodes							= new uint8[128];
	uint8			numOfNodes					= 0;
	uint32	*	nodeSibling					= NULL;
	STFString	partKey						= STFString("");
	STFString	keyElement					= STFString("");

	uint32		data							= 0;
	uint32		profileSize					= 0;
	STFResult	res							= STFRES_OK;

	res = GetProfileUsageSize (profileSize);
	//lint --e{613}
	if (STFRES_SUCCEEDED (res))
		{
		if (possiblePosition == profileSize)
			{
			isCh = false;
			res = STFRES_OK;
			}
		else
			{
			isCh = true;

			res = GetKeyStructure (key, nodes, numOfNodes);

			if (STFRES_SUCCEEDED (res))
				{
				//Fill nodeSibling with the sibling positions.
				nodeSibling = new uint32[numOfNodes];
				}

			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 i = 0; i < numOfNodes; i++)
					{
					//Fill partKey
					partKey = STFString("");
					position = 0;
					offset = 0;

					for (uint32 j = 0; j <= i; j++)
						{
						if (STFRES_SUCCEEDED (res))
							{
							res = GetKeyElement (key, nodes, j, keyElement);
							partKey = partKey + keyElement + STFString("/");
							}
						}

					// Find partKey and fill first sibling position in nodeSibling
					if (STFRES_SUCCEEDED (res))
						{
						res = FindKey (partKey, position);
						}
					if (STFRES_SUCCEEDED (res))
						{
						res = Read4Bytes (position, data);
						}

					offset = data & 0x7fffffff;

					nodeSibling[i] = position + offset;
					}
				}

			//See if all positions in nodeSibling point behind the possibleChildPosition.
			//If there is one position in nodeSibling that equals possibleChildPositions lastKeyElement has no child
			if (STFRES_SUCCEEDED (res))
				{
				for (uint32 k = 0; k < numOfNodes; k++)
					{
					if (nodeSibling[k] == possiblePosition)
						isCh = false;
					}
				}
			}
		}

	if (nodeSibling)
		delete[] nodeSibling;
	delete[] nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::AdjustNextSiblingOffset (STFString key, uint32 addSize)
	{
	STFString	keyPart			= STFString("");
	STFString	keyElement;
	uint8		*	nodes				= new uint8[128];
	uint8			numOfNodes		= 0;
	uint32		position			= 0;
	uint32		data				= 0;
	uint32		siblingOffset	= 0;
	STFResult	res				= STFRES_OK;

	res = GetKeyStructure (key, nodes, numOfNodes);

	if (STFRES_SUCCEEDED (res))
		{
		for (uint32 i = 0; i < numOfNodes; i++)
			{
			keyPart = STFString("");
			position = 0;
			siblingOffset = 0;

			for (uint32 j = 0; j <= i; j++)
				{
				if (STFRES_SUCCEEDED (res))
					{
					res = GetKeyElement(key, nodes, j, keyElement); 
					keyPart = keyPart + keyElement + STFString("/");
					}
				}

			if (STFRES_SUCCEEDED (res))
				{
				res = FindKey (keyPart, position);
				}

			if (STFRES_SUCCEEDED (res))
				{
				res = Read4Bytes (position, data);
				}
			
			if (STFRES_SUCCEEDED (res))
				{ 
				if (XTBF(31, data)) //This is a node,
					{
					siblingOffset = data & 0x7fffffff;
					if (siblingOffset != 0)
						res = Write4Bytes (position, data + addSize);
					}
				else //This is an entry
					{
					res = STFRES_OBJECT_INVALID;
					}
				}
			}
		}
	delete[] nodes;
	STFRES_RAISE (res);
	}

STFResult STFProfile::InsertNodeAt (STFString key, STFString name)
	{
	uint32	position		= 0;
	uint32	elementSize	= 0;
	uint32	data			= 0;
	
	bool		hasSibling	= false;
	uint32	nextSibling	= 0;
	uint32	nameSize		= 0;

	uint32	nodeSize		= 0;
	uint32	profileSize	= 0;

	uint8	*	tempData;

	STFRES_REASSERT (GetProfileUsageSize(profileSize));

	//Calculate size of this node
	nameSize = ((name.Length()+1 + 3) / 4) * 4;
	nodeSize = 8 + nameSize;

	STFRES_REASSERT (FindKey(key, position));

	//Go through entries and find position for insertion
	do
		{
		STFRES_REASSERT (GetElementSize (position, elementSize));
		position += elementSize;
		STFRES_REASSERT (Read4Bytes (position, data));
		}
	while ((XTBF(31, data) != true) && (position < profileSize) );

	//create nextSibling field
	STFRES_REASSERT (IsChildOf (key, position, hasSibling));
	nextSibling = 0x80000000;
	if (hasSibling)
		{
		nextSibling += nodeSize;
		}
	else
		{
		nextSibling += 0;
		}

	//When position is found move the rest by size of this node
	if (profileSize + nodeSize < this->size)
		{
		tempData = new uint8[profileSize - position];
		for (uint32 i = 0; i < profileSize - position; i++)
			{
			tempData[i] = dataContainer[position + i];
			dataContainer[position + i] = 0;
			}
		for (uint32 j = 0; j < profileSize - position; j++)
			{
			dataContainer[position + nodeSize + j] = tempData[j];
			}
		delete tempData;

		//Insert nextSibling at position
		STFRES_REASSERT (Write4Bytes (position, nextSibling));

		//Insert nameSize at position + 4
		STFRES_REASSERT (Write4Bytes (position + 4, nameSize));

		//Insert name at position + 8
		STFRES_REASSERT (WriteString (position + 8, name, nameSize));

		STFRES_REASSERT (AdjustNextSiblingOffset(key, nodeSize));

		STFRES_RAISE_OK;
		}
	else
		{
		DP("STFProfile: Can't insert Node. Not enough space left in profile");
		STFRES_RAISE (STFRES_NOT_ENOUGH_MEMORY);
		}
	}

STFResult STFProfile::InsertSEntryAt (STFString key, STFString name, STFString value)
	{
	uint32	position			= 0;
	uint32	elementSize		= 0;
	//uint32	data				= 0;

	//uint32	nextSibling		= 0;
	uint32	nameSize			= 0;
	uint32	nameSizeField	= 0;
	uint32	valueSize		= 0;

	uint32	entrySize		= 0;
	uint32	profileSize		= 0;

	uint8	*	tempData;

	//Calculate size of this entry
	nameSize = ((name.Length()+1 + 3) / 4) * 4;
	valueSize = ((value.Length()+1 + 3) / 4) * 4;
	entrySize = 8 + nameSize + valueSize;

	//Create nameSize field
	nameSizeField = nameSize + 0x40000000;

	//Find position for insertion
	STFRES_REASSERT (FindKey(key, position));
	STFRES_REASSERT (GetElementSize (position, elementSize));
	position += elementSize;

	//When position is found move the rest by size of this entry
	STFRES_REASSERT (GetProfileUsageSize(profileSize));

	if (profileSize + entrySize < this->size)
		{
		tempData = new uint8[profileSize - position];
		for (uint32 i = 0; i < profileSize - position; i++)
			{
			tempData[i] = dataContainer[position + i];
			dataContainer[position + i] = 0;
			}
		for (uint32 j = 0; j < profileSize - position; j++)
			{
			dataContainer[position + entrySize + j] = tempData[j];
			}
		delete tempData;
		
		//Insert nameSizeField at position
		STFRES_REASSERT (Write4Bytes (position, nameSizeField));

		//Insert name field at position + 4
		STFRES_REASSERT (WriteString (position + 4, name, nameSize));

		//Insert valueSize field at position + 4 + nameSize
		STFRES_REASSERT (Write4Bytes (position + 4 + nameSize, valueSize));

		//Insert value at position + 8 + nameSize
		STFRES_REASSERT (WriteString (position + 8 + nameSize, value, valueSize));

		STFRES_REASSERT (AdjustNextSiblingOffset(key, entrySize));

		STFRES_RAISE_OK;
		}
	else
		{
		DP("STFProfile: Can't insert sEntry. Not enough space left in profile");
		STFRES_RAISE (STFRES_NOT_ENOUGH_MEMORY);
		}
	}

STFResult STFProfile::InsertIEntryAt (STFString key, STFString name, uint32 value)
	{
	uint32	position			= 0;
	uint32	elementSize		= 0;
	//uint32	data				= 0;

	//uint32	nextSibling		= 0;
	uint32	nameSize			= 0;
	uint32	nameSizeField	= 0;

	uint32	entrySize		= 0;
	uint32	profileSize		= 0;

	uint8	*	tempData;

	//Calculate size of this entry
	nameSize = ((name.Length()+1 + 3) / 4) * 4;
	entrySize = 8 + nameSize;

	//Create nameSize field
	nameSizeField = nameSize + 0x0;

	//Find position for insertion
	STFRES_REASSERT (FindKey(key, position));
	STFRES_REASSERT (GetElementSize (position, elementSize));
	position += elementSize;

	//When position is found move the rest by size of this entry
	STFRES_REASSERT (GetProfileUsageSize(profileSize));

	if (profileSize + entrySize < this->size)
		{
		tempData = new uint8[profileSize - position];
		for (uint32 i = 0; i < profileSize - position; i++)
			{
			tempData[i] = dataContainer[position + i];
			dataContainer[position + i] = 0;
			}
		for (uint32 j = 0; j < profileSize - position; j++)
			{
			dataContainer[position + entrySize + j] = tempData[j];
			}
		delete tempData;
		
		//Insert nameSizeField at position
		STFRES_REASSERT (Write4Bytes (position, nameSizeField));

		//Insert name field at position + 4
		STFRES_REASSERT (WriteString (position + 4, name, nameSize));

		//Insert value at position + 4 + nameSize
		STFRES_REASSERT (Write4Bytes (position + 4 + nameSize, value));

		STFRES_REASSERT (AdjustNextSiblingOffset(key, entrySize));

		STFRES_RAISE_OK;
		}
	else
		{
		DP("STFProfile: Can't insert iEntry. Not enough space left in profile");
		STFRES_RAISE (STFRES_NOT_ENOUGH_MEMORY);
		}
	}

STFResult STFProfile::ChangeSEntryAt (STFString key, STFString name, STFString value)
	{
	STFResult	res			= STFRES_OK;
	uint32		data			= 0;
	uint32		nameSize		= 0;
	uint32		valSize		= 0;
	uint32		newSize		= 0;
	int32			sDiff			= 0;
	uint32		profileSize = 0;
	uint32		position		= 0;
	uint32		i				= 0;
	uint8		*	tempData;


	STFRES_REASSERT (GetProfileUsageSize(profileSize));

	STFRES_REASSERT (FindEntry (key, name, position));

	newSize = ((value.Length()+1 + 3) / 4) * 4;

	STFRES_REASSERT (Read4Bytes (position, data));

	if (XTBF(31, data)) //This is a node, not an entry.
		{
		res = STFRES_OBJECT_INVALID;
		}
	else //This is an entry
		{
		if (XTBF(30, data)) //value is string
			{
			nameSize = data & 0x3fffffff;
			position = position + nameSize + 4;
		
			STFRES_REASSERT (Read4Bytes (position, valSize));
			
			if (valSize < newSize) //value field must be expanded
				{
				sDiff = newSize - valSize;
				STFRES_REASSERT (Write4Bytes (position, newSize));
				position += 4; //point to value

				if (profileSize + sDiff < this->size)
					{
					tempData = new uint8[profileSize - position];
					for (i = 0; i < profileSize - position; i++)
						{
						tempData[i] = dataContainer[position + i];
						dataContainer[position + i] = 0;
						}
					for (i = 0; i < profileSize - position; i++)
						{
						dataContainer[position + sDiff + i] = tempData[i];
						}
					for (i = 0; i < newSize; i++) //blank the field first
						{
						dataContainer[position + i] = 0;
						} 
					delete[] tempData;

					STFRES_REASSERT (WriteString (position, value, newSize));
					STFRES_REASSERT (AdjustNextSiblingOffset (key, sDiff));
					}
				else
					{
					DP("STFProfile: Can't change sEntry. Not enough space left in profile");
					STFRES_RAISE (STFRES_NOT_ENOUGH_MEMORY);
					}
				}
			else if (valSize > newSize) //value field must be shortened
				{
				sDiff = valSize - newSize;
				STFRES_REASSERT (Write4Bytes (position, newSize));
				position += 4; //point to value

				tempData = new uint8[profileSize - position - valSize];
				for (i = 0; i < profileSize - position - valSize; i++)
					{
					tempData[i] = dataContainer[position + valSize + i];
					}
				for (i = 0; i < profileSize - position; i++)
					{
					dataContainer[position + i] = 0;
					}
				for (i = 0; i < profileSize - position - valSize; i++)
					{
					dataContainer[position + newSize + i] = tempData[i];
					}
				delete[] tempData;

				STFRES_REASSERT (WriteString (position, value, newSize));
				STFRES_REASSERT (AdjustNextSiblingOffset (key, -sDiff));
				}
			else //valSize = newSize: just blank the value field and fill in new value
				{
				position +=4;
				for (i = 0; i < newSize; i++) //blank the field first
					{
					dataContainer[position + i] = 0;
					} 
				STFRES_REASSERT (WriteString (position, value, newSize)); 
				} 

			res = STFRES_OK;
			}
		else //value is integer
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	STFRES_RAISE (res);
	}

STFResult STFProfile::ChangeIEntryAt (STFString key, STFString name, uint32 value)
	{
	uint32		position	= 0;
	uint32		data		= 0;
	uint32		nameSize	= 0;
	STFResult	res		= STFRES_OK;

	STFRES_REASSERT (FindEntry (key, name, position));

	STFRES_REASSERT (Read4Bytes (position, data));

	if (XTBF(31, data)) //This is a node, not an entry.
		{
		res = STFRES_OBJECT_INVALID;
		}
	else //This is an entry
		{
		if (XTBF(30, data)) //value is string
			{
			res = STFRES_OBJECT_INVALID;
			}
		else //value is integer
			{
			nameSize = data & 0x3fffffff;
			position = position + nameSize + 4;

			STFRES_REASSERT (Write4Bytes (position, value));
			res = STFRES_OK;
			}
		}
	STFRES_RAISE (res);
	}

STFResult STFProfile::Read4Bytes	(uint32 position, uint32 & data)
	{
	//lint --e{613}
	if (position + 3 <= size)
		{
		data = MAKELONG4(dataContainer[position + 0], 
							  dataContainer[position + 1], 
							  dataContainer[position + 2], 
							  dataContainer[position + 3]);
		STFRES_RAISE_OK;
		}
	else
		{
		data = 0;
		STFRES_RAISE (STFRES_RANGE_VIOLATION);
		}
	}

STFResult STFProfile::Write4Bytes (uint32 position, uint32 data)
	{
	//lint --e{613}
	if (position + 3 <= size)
		{
		dataContainer[position + 0] = LBYTE0 (data);
		dataContainer[position + 1] = LBYTE1 (data);
		dataContainer[position + 2] = LBYTE2 (data);
		dataContainer[position + 3] = LBYTE3 (data);

		STFRES_RAISE_OK;
		}
	else
		{
		STFRES_RAISE (STFRES_RANGE_VIOLATION);
		}
	}

STFResult STFProfile::WriteString (uint32 position, STFString data, uint32 size)
	{
	char		*	ch;
	char		*	chOrig;
	//uint32		posOrig	= 0;

	chOrig = new char[size];
	ch = chOrig;
	//lint --e{613}
	//lint --e{661}
	//posOrig = position;
	
	data.Get(ch, size);

	while (*ch)
		{
		dataContainer[position++] = *ch;
		ch++;
		}

	dataContainer[position++] = *ch;

	delete[] chOrig;
	STFRES_RAISE_OK;
	}

STFResult STFProfile::GetProfileUsageSize (uint32 & size)
	{
	//Rework needed! Check for consecutive 12 bytes of data until end of category size.  
	uint32		position		= 0;
	uint32		elementSize	= 0;

	uint32		data1			= 0xffffffff;
	uint32		data2			= 0xffffffff;
	uint32		data3			= 0xffffffff;

	STFResult	res			= STFRES_OK; 

	bool lastElement	= false;

	while (!lastElement)
		{
		STFRES_REASSERT (GetElementSize (position, elementSize));

		res = Read4Bytes (position + elementSize + 0, data1);
		if (STFRES_SUCCEEDED (res))
			res = Read4Bytes (position + elementSize + 4, data2);
		if (STFRES_SUCCEEDED (res))
			res = Read4Bytes (position + elementSize + 8, data3);

		if ( (data1==0 && data2==0 && data3==0) || ( !(STFRES_SUCCEEDED(res)) ) )
			{
			lastElement = true;
			}

		position += elementSize; 
		}

	size = position;
	STFRES_RAISE_OK;
	}

STFResult STFProfile::GetElementSize (uint32 position, uint32 & size)
	{
	uint32		data			= 0;
	uint32		nameLength	= 0;
	uint32		valueLength	= 0;

	STFRES_REASSERT (Read4Bytes (position, data));

	if (XTBF(31, data)) //This is a node,
		{
		STFRES_REASSERT (Read4Bytes (position + 4, nameLength));
		size = nameLength + 8;
		}
	else // This is an entry
		{
		//nameLength	= (data << 2) >> 2;
		nameLength = data & 0x3fffffff;

		if (XTBF(30, data)) //value is string
			{
			STFRES_REASSERT (Read4Bytes (position + 4 + nameLength, valueLength));
			size = nameLength + valueLength + 8;
			}
		else //value is integer
			{
			size = nameLength + 8;
			}
		}

	STFRES_RAISE_OK;
	}

STFResult STFProfile::GetNameAt (uint32 position, STFString & name)
	{
	uint32 data	= 0;

	name = STFString("");

	STFRES_REASSERT (Read4Bytes (position, data));

	if (XTBF(31, data)) //This is a node,
		{
		name = STFString ((char*)(&dataContainer[position + 8]));
		}
	else // This is an entry
		{
		name = STFString ((char*)(&dataContainer[position + 4]));
		}

	STFRES_RAISE_OK;
	}

STFResult STFProfile::GetStringValueAt (uint32 position, STFString & value)
	{
	STFResult	res			= STFRES_OK;
	uint32		nameLength	= 0;
	uint32		data			= 0;
	
	STFRES_REASSERT (Read4Bytes (position, data));
	
	//nameLength = (data << 2) >> 2;
	nameLength = data & 0x3fffffff;

	if (XTBF(31, data)) //This is a node, not an entry.
		{
		res = STFRES_OBJECT_INVALID;
		}
	else
		{
		if (XTBF(30, data)) //value is string
			{
			value	= STFString ((char*)(&dataContainer[position + 8 + nameLength]));
			}
		else //value is integer
			{
			res = STFRES_OBJECT_INVALID;
			}
		}

	STFRES_RAISE(res);
	}

STFResult STFProfile::GetIntegerValueAt (uint32 position, uint32 & value)
	{
	STFResult	res			= STFRES_OK;
	uint32		data			= 0;
	uint32		nameLength	= 0;
	
	STFRES_REASSERT (Read4Bytes (position, data));
	//nameLength = (data << 2) >> 2;
	nameLength = data & 0x3fffffff;

	if (XTBF(31, data)) //This is a node, not an entry.
		{
		res = STFRES_OBJECT_INVALID;
		}
	else
		{
		if (XTBF(30, data)) //value is string
			{
			res = STFRES_OBJECT_INVALID;
			}
		else //value is integer
			{
			STFRES_REASSERT (Read4Bytes (position + 4 + nameLength, value));
			}
		}

	STFRES_RAISE(res);
	}

////////////////////////////
// STFSynchronizedProfile
////////////////////////////

STFSynchronizedProfile::STFSynchronizedProfile (void)	{}

STFSynchronizedProfile::STFSynchronizedProfile (uint32 size, uint8 * data) : STFProfile(size, data) {}

STFSynchronizedProfile::~STFSynchronizedProfile (void){}

STFResult STFSynchronizedProfile::Write (STFString key, STFString name, int32 value)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Write(key, name, value));
	}

STFResult STFSynchronizedProfile::Read (STFString key, STFString name, int32 & value, int32 deflt)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Read(key, name, value, deflt));
	}
		
STFResult STFSynchronizedProfile::Write (STFString key, STFString name, bool value)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Write(key, name, value));
	}
STFResult STFSynchronizedProfile::Read (STFString key, STFString name, bool & value, bool deflt)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Read(key, name, value, deflt));
	}

STFResult STFSynchronizedProfile::Write (STFString key, STFString name, uint32 value)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Write(key, name, value));
	}
STFResult STFSynchronizedProfile::Read (STFString key, STFString name, uint32 & value, uint32 deflt)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Read(key, name, value, deflt));
	}
		
STFResult STFSynchronizedProfile::Write (STFString key, STFString name, STFString value)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Write(key, name, value));
	}	
STFResult STFSynchronizedProfile::Read (STFString key, STFString name, STFString & value, STFString deflt)
	{
	STFAutoMutex autoMutex(&mutex);
	STFRES_RAISE(STFProfile::Read(key, name, value, deflt));
	}
