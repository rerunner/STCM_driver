///
/// @brief      Demultiplexes DVD streams into elementary streams
///

#include "DVDStreamDemux.h"

#include "VDR/Source/Construction/IUnitConstruction.h"
#include "STF/Interface/STFDebug.h"


UNIT_CREATION_FUNCTION(CreateDVDStreamDemux, DVDStreamDemuxUnit)

// Connector IDs used in this Streaming Unit

#define EXT_STREAM_IN_ID					0
#define EXT_VIDEO_OUT_ID					1
#define EXT_AUDIO_OUT_ID					2
#define EXT_SUBPICTURE_OUT_ID				3

#define CHAIN_STREAM_OUT_ID				0
#define CHAIN_VIDEO_IN_ID					1
#define CHAIN_AUDIO_IN_ID					2
#define CHAIN_SUBPICTURE_IN_ID			3

STFResult DVDStreamDemuxUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualDVDStreamDemuxUnit(this));

	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	STFRES_RAISE_OK;
	}


STFResult DVDStreamDemuxUnit::Create(uint64 * createParams)
	{
	if (createParams[0] != PARAMS_DONE)
		STFRES_RAISE(STFRES_INVALID_PARAMETERS);

	STFRES_RAISE_OK;
	}


STFResult DVDStreamDemuxUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			streamReplicator = source;
			break;

		case 1:
			videoSplitter = source;
			break;

		case 2:
			audioSplitter = source;
			break;

		case 3:
			subpictureSplitter = source;
			break;

		case 4:
			videoUnpacker = source;
			break;

		case 5:
			audioUnpacker = source;
			break;

		case 6:
			subpictureUnpacker = source;
			break;

		default:
			STFRES_RAISE(STFRES_RANGE_VIOLATION);
		}

	STFRES_RAISE_OK;
	}


STFResult DVDStreamDemuxUnit::Initialize(uint64 * depUnitsParams)
	{
	STFRES_RAISE_OK;
	}


VirtualDVDStreamDemuxUnit::VirtualDVDStreamDemuxUnit(DVDStreamDemuxUnit * physical)
	: VirtualStreamingChainUnit(physical, 7),

	  inputConnector   		(  	EXT_STREAM_IN_ID,			this),
	  videoOutConnector		(16,	EXT_VIDEO_OUT_ID,			this),
	  audioOutConnector		(16,	EXT_AUDIO_OUT_ID,			this),
	  subpictureOutConnector( 1,	EXT_SUBPICTURE_OUT_ID,	this),

	  replicatorOutConnector(16,	CHAIN_STREAM_OUT_ID,		this),
	  videoInConnector      (		CHAIN_VIDEO_IN_ID,		this),
	  audioInConnector      (		CHAIN_AUDIO_IN_ID,		this),
	  subpictureInConnector	(		CHAIN_SUBPICTURE_IN_ID,	this)
	{
	this->physical = physical;

	// Add external connectors to StreamingUnit connector list
	AddConnector(&inputConnector);			// => ID 0
	AddConnector(&videoOutConnector);		// => ID 1
	AddConnector(&audioOutConnector);		// => ID 2
	AddConnector(&subpictureOutConnector);	// => ID 3

	streamReplicator = NULL;     
	videoSplitter = NULL;  
	audioSplitter = NULL;  
	subpictureSplitter = NULL;
	videoUnpacker = NULL;  
	audioUnpacker = NULL;  
	subpictureUnpacker = NULL;
	}


VirtualDVDStreamDemuxUnit::~VirtualDVDStreamDemuxUnit()
	{
	if (streamReplicator) streamReplicator->Release();
	if (videoSplitter) videoSplitter->Release();  
	if (audioSplitter) audioSplitter->Release();  
	if (subpictureSplitter) subpictureSplitter->Release();
	if (videoUnpacker) videoUnpacker->Release();  
	if (audioUnpacker) audioUnpacker->Release();  
	if (subpictureUnpacker) subpictureUnpacker->Release();
	}

STFResult VirtualDVDStreamDemuxUnit::AllocateChild(IPhysicalUnit * physical, IStreamingUnit * & streaingUnit)
	{
	IVirtualUnit			* tempVU;

	// Create AC3 Decoder unit
	STFRES_REASSERT(physical->CreateVirtual(tempVU, this, root ? root : this));
	STFRES_REASSERT(VirtualUnitCollection::AddLeafUnit(tempVU));	// Add to virtual unit collection

	STFRES_REASSERT(tempVU->QueryInterface(VDRIID_STREAMING_UNIT, (void*&) streaingUnit));
	STFRES_REASSERT(StreamingChainUnit::AddStreamingSubUnit(streaingUnit));	// Add to register of Streaming sub-units

	STFRES_RAISE_OK;
	}

STFResult VirtualDVDStreamDemuxUnit::AllocateChildUnits(void)
	{
	STFRES_REASSERT(AllocateChild(physical->streamReplicator, streamReplicator));
	STFRES_REASSERT(AllocateChild(physical->videoSplitter, videoSplitter));
	STFRES_REASSERT(AllocateChild(physical->audioSplitter, audioSplitter));
	STFRES_REASSERT(AllocateChild(physical->subpictureSplitter, subpictureSplitter));
	STFRES_REASSERT(AllocateChild(physical->videoUnpacker, videoUnpacker));
	STFRES_REASSERT(AllocateChild(physical->audioUnpacker, audioUnpacker));
	STFRES_REASSERT(AllocateChild(physical->subpictureUnpacker, subpictureUnpacker));

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDStreamDemuxUnit::Initialize(void)
	{
	IStreamingConnector * tempInConnector;
	IStreamingConnector * tempOutConnector;
	STFResult res = STFRES_OK;

	STFRES_REASSERT(VirtualStreamingChainUnit::Initialize());

	//
	// Make connections 
	//

	STFRES_REASSERT(streamReplicator->FindConnector(0, tempInConnector));
	STFRES_REASSERT(PlugConnectors(tempInConnector, &replicatorOutConnector));

	STFRES_REASSERT(streamReplicator->FindConnector(1, tempOutConnector));
	STFRES_REASSERT(videoSplitter->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

	STFRES_REASSERT(streamReplicator->FindConnector(2, tempOutConnector));
	STFRES_REASSERT(audioSplitter->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

	STFRES_REASSERT(streamReplicator->FindConnector(3, tempOutConnector));
	STFRES_REASSERT(subpictureSplitter->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

	STFRES_REASSERT(videoSplitter->FindConnector(1, tempOutConnector));
	STFRES_REASSERT(videoUnpacker->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

	STFRES_REASSERT(audioSplitter->FindConnector(1, tempOutConnector));
	STFRES_REASSERT(audioUnpacker->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

	STFRES_REASSERT(subpictureSplitter->FindConnector(1, tempOutConnector));
	STFRES_REASSERT(subpictureUnpacker->FindConnector(0, tempInConnector));
 	STFRES_REASSERT(PlugConnectors(tempInConnector, tempOutConnector));

 	STFRES_REASSERT(videoUnpacker->FindConnector(1, tempOutConnector));
 	STFRES_REASSERT(PlugConnectors(&videoInConnector, tempOutConnector));

 	STFRES_REASSERT(audioUnpacker->FindConnector(1, tempOutConnector));
 	STFRES_REASSERT(PlugConnectors(&audioInConnector, tempOutConnector));

 	STFRES_REASSERT(subpictureUnpacker->FindConnector(1, tempOutConnector));
 	STFRES_REASSERT(PlugConnectors(&subpictureInConnector, tempOutConnector));

	STFRES_RAISE(res);
	}


STFResult VirtualDVDStreamDemuxUnit::InternalUpdate(void)
	{
	STFRES_RAISE_OK;
	}


STFResult VirtualDVDStreamDemuxUnit::ReceivePacket(uint32 connectorID, StreamingDataPacket * packet)
	{
	STFRES_RAISE(replicatorOutConnector.SendPacket(packet));
	}


STFResult VirtualDVDStreamDemuxUnit::SignalPacketArrival(uint32 connectorID, uint32 numPackets)
	{
	// We are not queueing, so there is nothing we could actually do
	STFRES_RAISE_OK;
	}


STFResult VirtualDVDStreamDemuxUnit::UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2)
	{
#if 1	// default implementation
	switch (connectorID)
		{
		case EXT_VIDEO_OUT_ID:
			STFRES_RAISE(videoInConnector.SendUpstreamNotification(message, param1, param2));
			break;

		case EXT_AUDIO_OUT_ID:
			STFRES_RAISE(audioInConnector.SendUpstreamNotification(message, param1, param2));
			break;

		case EXT_SUBPICTURE_OUT_ID:
			STFRES_RAISE(subpictureInConnector.SendUpstreamNotification(message, param1, param2));
			break;
		default:
			DP("DVD Demux: message from unsupported output connector\n");
		}
#else	// temporal fix for testing
	if (message == VDRMID_STRM_SEGMENT_END)
		DP("VDRMID_STRM_SEGMENT_END\n");

	switch (connectorID)
		{
		case EXT_VIDEO_OUT_ID:
			if (message == VDRMID_STRM_END_PRESENTATION)
				STFRES_RAISE_OK;

			STFRES_RAISE(videoInConnector.SendUpstreamNotification(message, param1, param2));
			break;

		case EXT_AUDIO_OUT_ID:
			if (message == VDRMID_STRM_SEGMENT_END)
				message = VDRMID_STRM_END_PRESENTATION;
			STFRES_RAISE(audioInConnector.SendUpstreamNotification(message, param1, param2));
			break;

		case EXT_SUBPICTURE_OUT_ID:
			STFRES_RAISE(subpictureInConnector.SendUpstreamNotification(message, param1, param2));
			break;
		default:
			DP("DVD Demux: message from unsupported output connector\n");
		}
#endif

	// else give it directly to our input connector (unsupported extOut connector)
	STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
	}


STFResult VirtualDVDStreamDemuxUnit::ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator)
	{
	switch (connectorID)
		{
		case EXT_VIDEO_OUT_ID:
			STFRES_RAISE(videoInConnector.ProvideAllocator(allocator));
			break;

		case EXT_AUDIO_OUT_ID:
			STFRES_RAISE(audioInConnector.ProvideAllocator(allocator));
			break;

		case EXT_SUBPICTURE_OUT_ID:
			STFRES_RAISE(subpictureInConnector.ProvideAllocator(allocator));
			break;
		}

	// else give it to our input connector
	STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
	}


STFResult VirtualDVDStreamDemuxUnit::NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2)
	{
	//
	// Forward the request to the upstream filter via the input connector
	//
	STFRES_RAISE(inputConnector.SendUpstreamNotification(message, param1, param2));
	}
	

STFResult VirtualDVDStreamDemuxUnit::NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet)
	{
	// Check current stream type here!

	switch (nestedConnectorID)
		{	  
		case CHAIN_VIDEO_IN_ID:
			STFRES_RAISE(videoOutConnector.SendPacket(packet));
			break;

		case CHAIN_AUDIO_IN_ID:
			STFRES_RAISE(audioOutConnector.SendPacket(packet));
			break;

		case CHAIN_SUBPICTURE_IN_ID:
			STFRES_RAISE(subpictureOutConnector.SendPacket(packet));
			break;
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualDVDStreamDemuxUnit::NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets)
	{
	STFRES_RAISE_OK;
	}


// If an allocator is not used by the decoding subunits, it is forwarded further upstream
STFResult VirtualDVDStreamDemuxUnit::NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator)
	{
	STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
	}

#if _DEBUG
STFString VirtualDVDStreamDemuxUnit::GetInformation(void)
	{
	return STFString("DVDStreamDemux ") + STFString(physical->GetUnitID());
	}
#endif
