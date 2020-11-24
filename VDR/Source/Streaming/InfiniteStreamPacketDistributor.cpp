///
/// @brief      Distributes the packets of a stream round robin to an arbitrary number of ouptuts
///

#include "VDR/Source/Construction/IUnitConstruction.h"
#include "VDR/Source/Streaming/InfiniteStreamPacketDistributor.h"

UNIT_CREATION_FUNCTION(CreateStreamDistributionStreamingUnit, StreamPacketDistributorStreamingUnit)

STFResult StreamPacketDistributorStreamingUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualStreamPacketDistributorStreamingUnit(this, numOutputs));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}

STFResult StreamPacketDistributorStreamingUnit::Create(uint64 * createParams)
	{
	if (GetNumberOfParameters(createParams) != 1)
		STFRES_RAISE(STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);

	STFRES_REASSERT(GetDWordParameter(createParams, 0, numOutputs));

	STFRES_RAISE_OK;
	}

STFResult StreamPacketDistributorStreamingUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	STFRES_RAISE(STFRES_RANGE_VIOLATION);
	}


STFResult StreamPacketDistributorStreamingUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}




VirtualStreamPacketDistributorStreamingUnit::VirtualStreamPacketDistributorStreamingUnit(IPhysicalUnit * physical, uint32 numOutputs) : 
	VirtualStreamingUnit(physical), 
	inputConnector(0, this)
	{	
	outputConnectors = NULL;
	packetCounters = NULL;

	this->numOutputs = numOutputs;
	}

VirtualStreamPacketDistributorStreamingUnit::~VirtualStreamPacketDistributorStreamingUnit(void) 
	{
	if (outputConnectors)
		{
		for (uint32 i = 0; i < numOutputs; i++)
			delete outputConnectors[i];

		delete[] outputConnectors;
		outputConnectors = NULL;
		}

	delete [] packetCounters;
	packetCounters = NULL;
	}

STFResult VirtualStreamPacketDistributorStreamingUnit::Initialize(void) 
	{
	if (numOutputs == 0)
		STFRES_REASSERT(STFRES_INVALID_CONFIGURE_STATE);

	STFRES_REASSERT(this->AddConnector(&inputConnector));
	
	outputConnectors = new StreamingOutputConnector*[numOutputs];
	if (outputConnectors == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	packetCounters = new uint32[numOutputs];
	if (packetCounters == NULL)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	for (uint32 j = 0; j < numOutputs; j++)
		{
		outputConnectors[j] = NULL;
		}

	for (uint32 i = 0; i < numOutputs; i++)
		{
		outputConnectors[i] = new StreamingOutputConnector(0, i+1, this);
		if (outputConnectors[i] == NULL)
			STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
		packetCounters[i] = 0;
		STFRES_REASSERT(this->AddConnector(outputConnectors[i]));
		}

	nextOutputToServe = 0;
	
	STFRES_RAISE_OK;
	}

STFResult VirtualStreamPacketDistributorStreamingUnit::InternalUpdate(void) 
	{
	STFRES_RAISE_OK;
	}

STFResult VirtualStreamPacketDistributorStreamingUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets) 
	{ 
	STFRES_RAISE(STFRES_UNIMPLEMENTED); 
	}

STFResult VirtualStreamPacketDistributorStreamingUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
	}

STFResult VirtualStreamPacketDistributorStreamingUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFResult result = STFRES_OK;		
	//lint --e{613}
	for (uint32 i = 0; i < numOutputs; i++)
		{
		uint32 nextOutputToServeIndex = nextOutputToServe % numOutputs;

		result = outputConnectors[nextOutputToServeIndex]->SendPacket(packet);
		if (STFRES_SUCCEEDED(result))
			packetCounters[nextOutputToServeIndex]++;

		nextOutputToServe++;
		if (STFRES_SUCCEEDED(result))
			STFRES_RAISE(result);
		}

	STFRES_RAISE(result);
	}

// Special handling for messages requires overriding the following functions:
STFResult VirtualStreamPacketDistributorStreamingUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
	}


#if _DEBUG
//
// IStreamingUnitDebugging functions
//
STFString VirtualStreamPacketDistributorStreamingUnit::GetInformation(void)
	{
	return "PDU" + STFString((uint32) this, 8);
	}
#endif



