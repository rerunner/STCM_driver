///
/// @brief      Demultiplexes DVD streams into elementary streams
///

#ifndef DVDSTREAMDEMUX_H
#define DVDSTREAMDEMUX_H

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Unit/PhysicalUnit.h"

class DVDStreamDemuxUnit : public SharedPhysicalUnit
	{
	friend class VirtualDVDStreamDemuxUnit;
	protected:
 		IPhysicalUnit	*	streamReplicator;
		IPhysicalUnit	*	audioSplitter, * videoSplitter, * subpictureSplitter;
		IPhysicalUnit	*	audioUnpacker, * videoUnpacker, * subpictureUnpacker;
	public:
		DVDStreamDemuxUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

 		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};

class VirtualDVDStreamDemuxUnit : public VirtualStreamingChainUnit
	{
	private:
		DVDStreamDemuxUnit	*	physical;
	protected:
		//
		// "External" connectors
		//
		UnqueuedInputConnector			inputConnector;				///< Input connector with incoming dvd program stream
		StreamingOutputConnector		videoOutConnector;			///< Elementary stream video
		StreamingOutputConnector		audioOutConnector;			///< Elementary stream audio
		StreamingOutputConnector		subpictureOutConnector;		///< Elementary stream subpicture

		//
		// "Internal" connectors (i.e. in/out of subchain)
		//
		NestedOutputConnector			replicatorOutConnector;		///< Nested output connector to replicator
		UnqueuedNestedInputConnector	videoInConnector;				///< Nested input connector from video splitter
		UnqueuedNestedInputConnector	audioInConnector;				///< Nested input connector from audio splitter
		UnqueuedNestedInputConnector	subpictureInConnector;		///< Nested input connector from subpicture splitter

		IStreamingUnit					*	streamReplicator;
		IStreamingUnit					*	videoSplitter;
		IStreamingUnit					*	audioSplitter;
		IStreamingUnit					*	subpictureSplitter;
		IStreamingUnit					*	videoUnpacker;
		IStreamingUnit					*	audioUnpacker;
		IStreamingUnit					*	subpictureUnpacker;

		STFResult AllocateChild(IPhysicalUnit * physical, IStreamingUnit * & streaingUnit);

	public:
		VirtualDVDStreamDemuxUnit(DVDStreamDemuxUnit * physical);
		~VirtualDVDStreamDemuxUnit(void);

 		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);

		//
		// IStreamingChainUnit functions
		//
		virtual STFResult NestedUpstreamNotification(uint32 nestedConnectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult NestedReceivePacket(uint32 nestedConnectorID, StreamingDataPacket * packet);
		virtual STFResult NestedSignalPacketArrival(uint32 nestedConnectorID, uint32 numPackets);
		virtual STFResult NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator);

		//
		// VirtualUnitCollection pure virtual function overrides
		//
		virtual STFResult AllocateChildUnits(void);

		//! Initialisation of the unit and its subunits
		virtual STFResult Initialize(void);
  		virtual STFResult	InternalUpdate(void);

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};

#endif
