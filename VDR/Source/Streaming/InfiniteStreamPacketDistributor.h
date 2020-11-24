///
/// @file       VDR/Source/Streaming/InfiniteStreamPacketDistributor.h
///
/// @brief      Distributes the packets of a stream round robin to an arbitrary number of ouptuts
///
/// @par OWNER: VDR Streaming Architecture Team
///
/// @author     Ulrich Mohr
///
/// @par SCOPE: INTERNAL Header File
///
/// @date       2004-07-09
///
/// &copy; 2003 ST Microelectronics. All Rights Reserved.
///

#ifndef INFINITESTREAMPACKETDISTRIBUTOR_H
#define INFINITESTREAMPACKETDISTRIBUTOR_H

#include "VDR/Source/Unit/PhysicalUnit.h"
#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/StreamingConnectors.h"



/// Physical Stream Replicator Unit
class StreamPacketDistributorStreamingUnit : public SharedPhysicalUnit
	{
	protected:
		uint32                                 numOutputs;

	public:
		StreamPacketDistributorStreamingUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};



/// Virtual Stream Packet Distributor Unit
/// This is the standard implementation of a Streaming Unit that that distributes streaming 
/// packets round robin to its outputs.
class VirtualStreamPacketDistributorStreamingUnit : public VirtualStreamingUnit
	{
	protected:
		UnqueuedInputConnector      inputConnector;

		StreamingOutputConnector ** outputConnectors;    /// 
		uint32                      numOutputs;          /// The number of outputs

		uint32 *                    packetCounters;      /// For debugging, we have a package counter for each output

		uint32                      nextOutputToServe;   /// the next output connector to server with a packet
		
		
	public:
		VirtualStreamPacketDistributorStreamingUnit(IPhysicalUnit * physical, uint32 numOutputs);
		
		virtual ~VirtualStreamPacketDistributorStreamingUnit(void);
		
		STFResult Initialize(void);

		STFResult InternalUpdate(void);
		
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);

		STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);

		// Special handling for messages requires overriding the following functions:
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void);
#endif
	};



#endif // INFINITESTREAMPACKETDISTRIBUTOR_H
