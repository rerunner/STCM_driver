///
/// @file       VDR/Source/Streaming/BaseStreamingChainUnit.cpp
///
/// @brief      Base Implementations for generic Streaming Chain Units
///
/// @author     U. Sigmund
///
/// @date       2004-01-23
///
/// @par OWNER: VDR Architecture Team
///
/// @par SCOPE: INTERNAL Header File
///
/// Implementation of a generic streaming chain unit, configured in
/// the global board config.
///
/// &copy: 2003 ST Microelectronics. All Rights Reserved.
///

#include "BaseStreamingChainUnit.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

UNIT_CREATION_FUNCTION(CreateGenericStreamingChainUnit, PhysicalGenericStreamingChainUnit)

PhysicalGenericStreamingChainUnit::PhysicalGenericStreamingChainUnit(VDRUID unitID)
	 : SharedPhysicalUnit(unitID) 
	{
	//
	// We assume four children as a start, more will be allocated if required
	//
	totalChildUnits = 4;
	numChildUnits = 0;
	childUnits = new IPhysicalUnitPtr[totalChildUnits];
	totalConnections = 16;
	numConnections = 0;
	connections = new PGSCUConnection[totalConnections];
	}

PhysicalGenericStreamingChainUnit::~PhysicalGenericStreamingChainUnit(void)
	{
	delete[] childUnits;
	delete[] connections;
	}

STFResult PhysicalGenericStreamingChainUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualGenericStreamingChainUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult PhysicalGenericStreamingChainUnit::Create(uint64 * createParams)
	{
	int							i;
	PGSCUConnection		*	tempConnections;
        //lint --e{613}
	// get number of connectors
	STFRES_ASSERT(createParams[0] == PARAMS_DWORD, STFRES_INVALID_PARAMETERS);
	numChainInputs = createParams[1];
	STFRES_ASSERT(createParams[2] == PARAMS_DWORD, STFRES_INVALID_PARAMETERS);
	numChainOutputs = createParams[3];

	// parse list of connections
	createParams += 4;
	while (createParams[0] == PARAMS_DWORD)
		{
		// Check if we have enough space left for the next connection
		if (numConnections == totalConnections)
			{
			// If not double array size
			totalConnections *= 2;
			tempConnections = new PGSCUConnection[totalConnections];
			for(i=0; i<numConnections; i++)
				tempConnections[i] = connections[i];
			delete[] connections;
			connections = tempConnections;
			}

		// Extract the various IDs from the DWORD
		connections[numConnections].fromUnit = ((uint32)(createParams[1]) >> 24) & 0xff;
		connections[numConnections].outputID = ((uint32)(createParams[1]) >> 16) & 0xff;
		connections[numConnections].toUnit   = ((uint32)(createParams[1]) >>  8) & 0xff;
		connections[numConnections].inputID  = ((uint32)(createParams[1])      ) & 0xff;

		// Proceed to the next connection
		numConnections++;
		createParams += 2;
		}

	STFRES_RAISE_OK;
	}

STFResult PhysicalGenericStreamingChainUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	IPhysicalUnitPtr	*	tempChildUnits;
	int						i;

	// Check if we have enough space left for the next child unit
	if (localID >= totalChildUnits)
		{
		// If not, double the array size
		totalChildUnits += localID;
		tempChildUnits = new IPhysicalUnitPtr[totalChildUnits];
		for(i=0; i<numChildUnits; i++)
			tempChildUnits[i] = childUnits[i];
		delete[] childUnits;
		childUnits = tempChildUnits;
		}

	// Because childs may not come in order, we clear all entries up
	// to the new child unit ID
	while (localID >= numChildUnits)
		childUnits[numChildUnits++] = NULL;

	// place child
	childUnits[localID] = source;

	STFRES_RAISE_OK;
	}

STFResult PhysicalGenericStreamingChainUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}



VirtualGenericStreamingChainUnit::VirtualGenericStreamingChainUnit(PhysicalGenericStreamingChainUnit * physical)
	: VirtualStreamingChainUnit(physical, physical->numChildUnits)
	{
	int	i;

	physicalUnit = physical;
        //lint --e{613}
	// Allocate inputs if they exist
	numChainInputs = physical->numChainInputs;
	if (numChainInputs <= 0)
		chainInputs = NULL;
	else
		chainInputs = new DirectChainInputConnectorPtr[numChainInputs];

	for(i=0; i<numChainInputs; i++)
		{
		chainInputs[i] = new DirectChainInputConnector(i, this); 
		AddConnector(chainInputs[i]);
		}

	// Allocate outputs if they exist
	numChainOutputs = physical->numChainOutputs;
	if (numChainOutputs <= 0)
		chainOutputs = NULL;
	else
		chainOutputs = new DirectChainOutputConnectorPtr[numChainOutputs];

	for(i=0; i<numChainOutputs; i++)
		{
		chainOutputs[i] = new DirectChainOutputConnector(i + numChainInputs, this); 
		AddConnector(chainOutputs[i]);
		}
	}

VirtualGenericStreamingChainUnit::~VirtualGenericStreamingChainUnit(void)
	{
	int	i;
	//lint --e{613}
	// should have unconnected before...

	for(i=0; i<numChainInputs; i++)
		delete chainInputs[i];
	for(i=0; i<numChainOutputs; i++)
		delete chainOutputs[i];
	delete[] chainInputs;
	delete[] chainOutputs;
	}

STFResult VirtualGenericStreamingChainUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	// Forward the allocator
	STFRES_RAISE(chainOutputs[connectorID - numChainInputs]->GetChainInputConnector()->ProvideAllocator(allocator));
	}

STFResult VirtualGenericStreamingChainUnit::NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator)
	{
	//lint --e{613}
	// Forward the allocator
	STFRES_RAISE(chainInputs[nestedConnectorID]->ProvideAllocator(allocator));
	}

STFResult VirtualGenericStreamingChainUnit::IsPushingChain(uint32 connectorID)
	{
	//lint --e{613}
	// Forward the allocator
	STFRES_RAISE(chainOutputs[connectorID - numChainInputs]->GetChainInputConnector()->IsPushingChain());
	}

STFResult VirtualGenericStreamingChainUnit::NestedIsPushingChain(uint32 nestedConnectorID)
	{
	//lint --e{613}
	// Forward the allocator
	STFRES_RAISE(chainInputs[nestedConnectorID]->IsPushingChain());
	}


STFResult VirtualGenericStreamingChainUnit::AllocateChildUnits(void)
	{
	int	i;
	IVirtualUnit			*	tempVU;
	IStreamingUnit			*	tempSU;

	//
	// Allocate all child units, and hand the handles over to the virtual unit
	// collection and the streaming chain.  Thus we keep no references to our
	// childs.
	//
	for(i=0; i<physicalUnit->numChildUnits; i++)
		{
		STFRES_REASSERT(physicalUnit->childUnits[i]->CreateVirtual(tempVU, this, root ? root : this));
		STFRES_REASSERT(VirtualUnitCollection::AddLeafUnit(tempVU));	// Add to virtual unit collection

		STFRES_REASSERT(tempVU->QueryInterface(VDRIID_STREAMING_UNIT, (void*&) tempSU));
		STFRES_REASSERT(StreamingChainUnit::AddStreamingSubUnit(tempSU));	// Add to register of Streaming sub-units
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualGenericStreamingChainUnit::Initialize(void)
	{
	int																	i;
	PhysicalGenericStreamingChainUnit::PGSCUConnection		con;
	IStreamingConnector											*	tempInConnector;
	IStreamingConnector											*	tempOutConnector;
        //lint --e{613}
	//
	// Initialize childs
	//
	STFRES_REASSERT(VirtualStreamingChainUnit::Initialize());

	//
	// Iterate over all required connections
	//
	for(i=0; i<physicalUnit->numConnections; i++)
		{
		con = physicalUnit->connections[i];
         
		// Check type of connection
		if (con.fromUnit == 0xff)
			{
			//
			// It is an input to the chain, we thus need to find the input connector of
			// the child unit, that will receive the input.
			//
			STFRES_REASSERT(streamingSubUnits[con.toUnit]->FindConnector(con.inputID, tempInConnector));
			STFRES_REASSERT(PlugConnectors(tempInConnector, chainInputs[con.outputID]->GetChainOutputConnector()));
			}
		else if (con.toUnit == 0xff)
			{
			//
			// It is an output of the chain, we thus need to find the output connector of
			// the child unit, that will provide the data.
			//
			STFRES_REASSERT(streamingSubUnits[con.fromUnit]->FindConnector(con.outputID, tempOutConnector));
			STFRES_REASSERT(PlugConnectors(chainOutputs[con.inputID - numChainInputs]->GetChainInputConnector(), tempOutConnector));
			}
		else
			{
			//
			// It is an internal connection, so wec just let the StreamingChain do its job
			//
			STFRES_REASSERT(PlugSubUnitConnectors(con.fromUnit, con.outputID, con.toUnit, con.inputID));
			}
		}

	STFRES_RAISE_OK;
	}

STFResult	VirtualGenericStreamingChainUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}

#if _DEBUG
STFString VirtualGenericStreamingChainUnit::GetInformation(void)
	{
	return STFString("GenericStreamingChainUnit ") + STFString(physical->GetUnitID(), 8, 16);
	}
#endif


DirectChainInputConnector::DirectChainNestedOutputConnector::DirectChainNestedOutputConnector(uint32 id, VirtualStreamingChainUnit * unit, DirectChainInputConnector * inputConnector)
	: NestedOutputConnector(0, id, unit)
	{
	this->inputConnector = inputConnector;
	}

STFResult DirectChainInputConnector::DirectChainNestedOutputConnector::UpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	// Forward notification to partner connector
	STFRES_RAISE(inputConnector->SendUpstreamNotification(message, param1, param2));
	}

DirectChainInputConnector::DirectChainInputConnector(uint32 id, VirtualStreamingChainUnit * unit)
	: UnqueuedInputConnector(id, unit), nestedOutputConnector(id, unit, this)
	{
	}

STFResult DirectChainInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{
	// Forward packet to partner connector
	STFRES_RAISE(nestedOutputConnector.SendPacket(packet));
	}

STFResult DirectChainInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(nestedOutputConnector.GetStreamTagIDs(ids));
	}


DirectChainOutputConnector::DirectChainNestedInputConnector::DirectChainNestedInputConnector(uint32 id, VirtualStreamingChainUnit * unit, DirectChainOutputConnector * outputConnector)
	: UnqueuedNestedInputConnector(id, unit)
	{
	this->outputConnector = outputConnector;
	}

STFResult DirectChainOutputConnector::DirectChainNestedInputConnector::ReceivePacket(StreamingDataPacket * packet)
	{								  
	// Forward packet to partner connector
	STFRES_RAISE(outputConnector->SendPacket(packet));
	}

STFResult DirectChainOutputConnector::DirectChainNestedInputConnector::GetStreamTagIDs(VDRTID * & ids)
	{
	STFRES_RAISE(outputConnector->GetStreamTagIDs(ids));
	}


DirectChainOutputConnector::DirectChainOutputConnector(uint32 id, VirtualStreamingChainUnit * unit)
	: StreamingOutputConnector(0, id, unit), nestedInputConnector(id, unit, this)
	{
	}

STFResult DirectChainOutputConnector::UpstreamNotification(VDRMID message, uint32 param1, uint32 param2)
	{
	// Forward notification to partner connector
	STFRES_RAISE(nestedInputConnector.SendUpstreamNotification(message, param1, param2));
	}
