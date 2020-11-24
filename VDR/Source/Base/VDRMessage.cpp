
//
// PURPOSE:		Base Message Interface Implementation
//									  

/*! \file
	\brief Base Message Interface. Defines functions to register Message Sinks at various driver components.
*/

#include "VDRMessage.h"

MessageSinkRegistration::MessageSinkRegistration(STFMessageDispatcher * dispatcher) : DispatchedSTFMessageSink(dispatcher)
	{
	numSinks		= 0;
	totalSinks	= 4;

	registeredSinks = new RegisteredMessageSinkPtr[totalSinks];
	}


MessageSinkRegistration::~MessageSinkRegistration()
	{
	int i = 0;
	
	while (i < numSinks)
		{
		delete registeredSinks[i];
		i++;
		}

	delete[] registeredSinks;
	}


STFResult MessageSinkRegistration::ReceiveMessage(STFMessage & message)
	{
	int	numberOfSends = 0;
	int	i;

	// Send messages to registered message sinks

	sinkListLock.Enter();

	i = 0;
	while (i < numSinks)
		{
		if (registeredSinks[i]->range == 0 || 
			 // Is the message in the requested range for the registerd sink?
			 ((message.message >= registeredSinks[i]->message) && 
			  (message.message <= (registeredSinks[i]->message + registeredSinks[i]->range))))
			{
			if (!STFRES_IS_ERROR(registeredSinks[i]->sink->SendMessage(message, false)))
				numberOfSends++;
			}

		i++;
		}

	sinkListLock.Leave();

	// If there was no receiver for the message, we must complete it now to possibly awaken
	// the sender.
	if (numberOfSends == 0)
		message.Complete();

	STFRES_RAISE_OK;
	}


STFResult MessageSinkRegistration::RegisterMessageSink(STFMessageSink * sink)
	{
	STFRES_RAISE(RegisterFilteredMessageSink(sink, 0, 0));
	}


STFResult MessageSinkRegistration::RegisterFilteredMessageSink(STFMessageSink * sink, uint32 message, uint32 range)
	{
	RegisteredMessageSinkPtr * tempSinks;
	//lint --e{661}
	if (sink)
		{
		int i = 0;

		sinkListLock.Enter();

		// No more entries in list?
		if (numSinks == totalSinks)
			{
			// So double the size of the array!
			tempSinks = registeredSinks;
			totalSinks *= 2;

			registeredSinks = new RegisteredMessageSinkPtr[totalSinks];

			for (i = 0; i < numSinks; i++)
				registeredSinks[i] = tempSinks[i];

			delete[] tempSinks;
			}

		registeredSinks[numSinks] = new RegisteredMessageSink(sink, message, range);

		numSinks++;

		sinkListLock.Leave();
		}
	else
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}


STFResult MessageSinkRegistration::UnregisterMessageSink(STFMessageSink * sink)
	{
	if (numSinks > 0)
		{
		sinkListLock.Enter();

		int i = 0;

		// Search for the sink in the list of registered sinks
		while (i < numSinks && registeredSinks[i]->sink != sink)
			i++;

		if (i < numSinks)
			{
			delete registeredSinks[i];
			
			numSinks--;

			// Compact the array
			registeredSinks[i] = registeredSinks[numSinks];

			sinkListLock.Leave();
			}
		else
			{
			sinkListLock.Leave();

			STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
			}
		}
	else
		STFRES_RAISE(STFRES_OBJECT_EMPTY);

	STFRES_RAISE_OK;
	}


STFResult MessageSinkRegistration::SetDispatcher(STFMessageDispatcher * dispatcher)
	{
	if (this->dispatcher && dispatcher && dispatcher != this->dispatcher)
		dispatcher->AbortDispatcher();

	this->dispatcher = dispatcher;

	STFRES_RAISE_OK;
	}


STFResult MessageSinkRegistration::QueryInterface(VDRIID iid, void *& ifp)
	{
	VDRQI_BEGIN
		VDRQI_IMPLEMENT(VDRIID_VDR_BASE,								IVDRBase);
		VDRQI_IMPLEMENT(VDRIID_VDR_MESSAGE_SINK_REGISTRATION,	IVDRMessageSinkRegistration);
	VDRQI_END_BASE;

	STFRES_RAISE_OK;
	}




//
// Message collector unit implementation 
//


MessageCollector::MessageCollector(STFMessageDispatcher * dispatcher) : MessageSinkRegistration(dispatcher)
	{	
	this->registeredMessageSinkRegistrationsTable = NULL;
	this->maximumNumberOfHandledRegistrations = 0;
	}

MessageCollector::~MessageCollector()
	{
	if (registeredMessageSinkRegistrationsTable != NULL)
		{
		Cleanup();
		delete[] registeredMessageSinkRegistrationsTable;
		}
	}

STFResult MessageCollector::Initialize(uint64 maxMessageSinkRegistrations)
	{
	ASSERT(registeredMessageSinkRegistrationsTable == NULL);
	
	STFRES_ASSERT(maxMessageSinkRegistrations > 0, STFRES_INVALID_PARAMETERS);

	this->maximumNumberOfHandledRegistrations = maxMessageSinkRegistrations;
	
	this->registeredMessageSinkRegistrationsTable = new MessageSinkRegistrationInformation[this->maximumNumberOfHandledRegistrations];
	if (this->registeredMessageSinkRegistrationsTable != NULL)
		{
		for (uint32 i = 0; i < this->maximumNumberOfHandledRegistrations; i++)
			{
			this->registeredMessageSinkRegistrationsTable[i].sinkRegistration = NULL;
			this->registeredMessageSinkRegistrationsTable[i].valid = false;
			}
		}
	else
		{
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		}

	STFRES_RAISE_OK;
	}


STFResult MessageCollector::Cleanup()
	{
	//lint --e{613}
	ASSERT(registeredMessageSinkRegistrationsTable != NULL);

	for (uint32 i = 0; i < this->maximumNumberOfHandledRegistrations; i++)
		{
		this->registeredMessageSinkRegistrationsTable[i].valid = false;
		this->registeredMessageSinkRegistrationsTable[i].sinkRegistration->Release();
		this->registeredMessageSinkRegistrationsTable[i].sinkRegistration = NULL;
		}

	STFRES_RAISE_OK;
	}

STFResult MessageCollector::FindNextFreeElementInRegistrationTable(uint32 & indexOfFreeElement)
	{
	//lint --e{613}
	ASSERT(this->registeredMessageSinkRegistrationsTable != NULL);

	for (uint32 i = 0; i < this->maximumNumberOfHandledRegistrations; i++)
		{
		if (this->registeredMessageSinkRegistrationsTable[i].valid == false)
			{
			indexOfFreeElement = i;
			STFRES_RAISE_OK;
			}
		}

	indexOfFreeElement = 0xffffffff;
	STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}


STFResult MessageCollector::FindEntryByMessageSinkRegistration(IVDRMessageSinkRegistration * searchedSinkRegistration, uint32 & indexOfFoundElement)
	{
	//lint --e{613}
	ASSERT(this->registeredMessageSinkRegistrationsTable != NULL);

	indexOfFoundElement = 0xffffffff;

	for (uint32 i = 0; i < this->maximumNumberOfHandledRegistrations; i++)
		{
		if ( this->registeredMessageSinkRegistrationsTable[i].valid && 
			 (this->registeredMessageSinkRegistrationsTable[i].sinkRegistration == searchedSinkRegistration) )
			{
			indexOfFoundElement = i;
			STFRES_RAISE_OK;
			}
		}

	STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
	}


STFResult MessageCollector::RegisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration)
	{
	uint32 indexOfNextFree = 0xffffffff;

	// check object state and parameters
	STFRES_ASSERT(this->registeredMessageSinkRegistrationsTable, STFRES_OBJECT_INVALID);		
	
	STFRES_ASSERT(sinkRegistration != NULL, STFRES_INVALID_PARAMETERS);
	
	// find free entry
	STFRES_ASSERT(STFRES_SUCCEEDED(FindNextFreeElementInRegistrationTable(indexOfNextFree)), STFRES_OBJECT_FULL);

	// enter the sink registration
	this->registeredMessageSinkRegistrationsTable[indexOfNextFree].valid = true;
	this->registeredMessageSinkRegistrationsTable[indexOfNextFree].sinkRegistration = sinkRegistration;		

	STFResult res = this->registeredMessageSinkRegistrationsTable[indexOfNextFree].sinkRegistration->RegisterMessageSink(this);
	if (STFRES_FAILED(res))
		{
		this->registeredMessageSinkRegistrationsTable[indexOfNextFree].sinkRegistration = NULL;
		this->registeredMessageSinkRegistrationsTable[indexOfNextFree].valid = false;		
		}

	sinkRegistration->AddRef();

	STFRES_RAISE_OK;
	}


STFResult MessageCollector::UnregisterMessageSinkRegistration(IVDRMessageSinkRegistration * sinkRegistration)
	{
	uint32 indexOfSinkRegistration = 0xffffffff;

	// check object state and parameters
	STFRES_ASSERT(this->registeredMessageSinkRegistrationsTable, STFRES_OBJECT_INVALID);
	STFRES_ASSERT(sinkRegistration != NULL, STFRES_INVALID_PARAMETERS);	

	// find a matching entry and check it
	STFRES_REASSERT(FindEntryByMessageSinkRegistration(sinkRegistration, indexOfSinkRegistration));

	ASSERT(this->registeredMessageSinkRegistrationsTable[indexOfSinkRegistration].sinkRegistration != NULL);
		
	// remove from the list and release
	this->registeredMessageSinkRegistrationsTable[indexOfSinkRegistration].sinkRegistration = NULL;
	this->registeredMessageSinkRegistrationsTable[indexOfSinkRegistration].valid = false;

	sinkRegistration->Release();

	STFRES_RAISE_OK;
	}


STFResult MessageCollector::QueryInterface(VDRIID iid, void * & ifp)
	{
	VDRQI_BEGIN		
		VDRQI_IMPLEMENT(VDRIID_MESSAGE_COLLECTOR,	IVDRMessageCollector);
	VDRQI_END(MessageSinkRegistration);

	STFRES_RAISE_OK;
	}

