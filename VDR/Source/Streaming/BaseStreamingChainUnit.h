#ifndef BASESTREAMINGCHAINUNIT_H
#define BASESTREAMINGCHAINUNIT_H

///
/// @file       VDR/Source/Streaming/BaseStreamingChainUnit.h
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

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Unit/PhysicalUnit.h"

///////////////////////////////////////////////////////////////////////////////
// GenericStreamingChainUnit
///////////////////////////////////////////////////////////////////////////////


/// A generic streaming chain unit can be used to build a streaming chain
/// with just config information.  This chain does not process any information
/// in the chain unit, nor does it further configuration on the streaming units,
/// it simply provides connection and packet/message forwarding.
/// 
/// Configuration parameters for this unit are:
///	DWORD			number of inputs of the chain
///	DWORD			number of outputs of the chain
///	DWORD[]		connections
///
///	Each connection parameter consists of four entries, packed into a DWORD:
///		sourceUnitID, outputConnectorID, targetUnitID, inputConnectorID
///
///	Currently the ordering is MSB first, but this might probably change, when
///	the type of the parameter is changed to QUADBYTE ...
///
///	The unitID in this connection list, are the ordinal number of the child
///	units of this streaming chain unit.  The chain itself has the unit ID 255 
///   (0xff).
///
///	An example would be a simple DVDDemux
///
///	               ,------------.
///	               |            +-> C1-video
///	    dvdvob-C0->+ DVD Demux  +-> C2-audio
///	               |            +-> C3-spu
///	               `------------�
///	
///	is composed of a Replicator, three Splitters and three Unpackers
///	
///	            ,-------------------------------------------------------.
///	            |                                                       |
///	            |                       ,------------.  ,------------.  |
///	            |                    ,->+ V-Splitter +->+ V-Unpacker +->+-> C1-video
///	            |                   /   `------------�  `------------�  |
///	            |  ,------------.  /                                    |
///	            |  |            +-�     ,------------.  ,------------.  |
///	 dvdvob-C0->+->+ Replicator +------>+ A-Splitter +->+ A-Unpacker +->+-> C2-audio
///	            |  |            +-.     `------------�  `------------�  |
///	            |  `------------�  \                                    |
///	            |                   \   ,------------.  ,------------.  |
///	            |                    `->+ S-Splitter +->+ S-Unpacker +->+-> C3-spu
///	            |                       `------------�  `------------�  |
///	            |                                                       |
///	            `-------------------------------------------------------�
///
///	The config for this unit looks like this:
///
///	CREATE_UNIT (VDRUID_DVD_DEMULTIPLEXER, CreateGenericStreamingChainUnit),
///					DWORD_PARAM(1),	// num inputs
///					DWORD_PARAM(3),	// num outputs
///
///					// REPLICATOR
///					DWORD_PARAM(0xff000000),// Connect Chain, Out0 -> Unit0, In0					
///					DWORD_PARAM(0x00010100),// Connect Unit0, Out1 -> Unit1, In0
///					DWORD_PARAM(0x00020200),// Connect Unit0, Out2 -> Unit2, In0
///					DWORD_PARAM(0x00030300),// Connect Unit0, Out3 -> Unit3, In0
///
///					// SPLITTER to UNPACKER		
///					DWORD_PARAM(0x01010400),// Connect Unit1, Out0 -> Unit4, In0
///					DWORD_PARAM(0x02010500),// Connect Unit2, Out0 -> Unit5, In0
///					DWORD_PARAM(0x03010600),// Connect Unit3, Out0 -> Unit6, In0
///
///					// UNPACKER
///					DWORD_PARAM(0x0401ff01),// Connect Unit4, Out0 -> Chain, In0
///					DWORD_PARAM(0x0501ff02),// Connect Unit5, Out0 -> Chain, In1
///					DWORD_PARAM(0x0601ff03),// Connect Unit6, Out0 -> Chain, In2
///					
///					PARAMS_DONE,
///		VDRUID_DVD_DEMUX_REPLICATOR,				PARAMS_DONE,
///		VDRUID_DVD_DEMUX_VIDEO_SPLITTER,			PARAMS_DONE,
///		VDRUID_DVD_DEMUX_AUDIO_SPLITTER,			PARAMS_DONE,
///		VDRUID_DVD_DEMUX_SUBPICTURE_SPLITTER,	PARAMS_DONE,
///		VDRUID_DVD_DEMUX_VIDEO_UNPACKER,			PARAMS_DONE,
///		VDRUID_DVD_DEMUX_AUDIO_UNPACKER,			PARAMS_DONE,
///		VDRUID_DVD_DEMUX_SUBPICTURE_UNPACKER,	PARAMS_DONE,
///		MAPPING_DONE,
///
class PhysicalGenericStreamingChainUnit : public SharedPhysicalUnit
	{
	friend class VirtualGenericStreamingChainUnit;
	protected:
		/// Stores a single connection, to be used by VirtualUnit ininialize
		struct PGSCUConnection
			{
			uint8	fromUnit, outputID;
			uint8	toUnit, inputID;
			};

		typedef PGSCUConnection * PGSCUConnectionPtr;

		int						numChildUnits, totalChildUnits;	/// Actual and allocated number of child units
		IPhysicalUnitPtr	*	childUnits;								/// The child units provided by the config

		int						numChainInputs, numChainOutputs;	/// Number of connectors of this chain unit itself
		int						numConnections, totalConnections;/// Actual and allocated number of connections to make
		PGSCUConnection	*	connections;							/// Array of connections to make between the chain and the childs

	public:
		PhysicalGenericStreamingChainUnit(VDRUID unitID);
		~PhysicalGenericStreamingChainUnit(void);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

	};


/// A direct chain input connector is actually two connectors, an unqueued input connector
/// and a nested output connector.  These connectors have a direct link, and can therefore
/// bypass the chain unit when sending notifications and packets.  This connector can
/// be used, if the input of a streaming chain does not perform any processing, but simply
/// passes all traffic through to the nested output connector and vice versa.  This saves
/// two out of the three required calls, thus significantly shorting the call stack.
///
/// The usage of a direct chain connector can neither be detected by the unit with which
/// it is connected outside the chain, nor with the child unit with which it is connected
/// inside the chain.  It is therefore completely transparent and can be chosen purely based
/// on the processing requirements of the streaming chain unit itself.
///
class DirectChainInputConnector : public UnqueuedInputConnector
	{
	friend class DirectChainNestedOutputConnector;
	protected:
		class DirectChainNestedOutputConnector : public NestedOutputConnector
			{
			friend class DirectChainInputConnector;
			protected:
				DirectChainInputConnector	*	inputConnector;
			public:
				DirectChainNestedOutputConnector(uint32 id, VirtualStreamingChainUnit * unit, DirectChainInputConnector * inputConnector);

				virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2);
			};

		DirectChainNestedOutputConnector		nestedOutputConnector;
	public:
		DirectChainInputConnector(uint32 id, VirtualStreamingChainUnit * unit);

		virtual STFResult ReceivePacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamTagIDs(VDRTID * & ids);

		/// Get the output connector of this connector pair
		DirectChainNestedOutputConnector * GetChainOutputConnector(void) {return &nestedOutputConnector;}
	};


/// A direct chain output connector is actually two connectors, an unqueued nested input
/// connector and an output connector.  These connectors have a direct link, and can therefore
/// bypass the chain unit when sending notifications and packets.  This connector can
/// be used, if the nested input of a streaming chain does not perform any processing, but simply
/// passes all traffic through to the output connector and vice versa.  This saves
/// two out of the three required calls, thus significantly shorting the call stack.
///
/// The usage of a direct chain connector can neither be detected by the unit with which
/// it is connected outside the chain, nor with the child unit with which it is connected
/// inside the chain.  It is therefore completely transparent and can be chosen purely based
/// on the processing requirements of the streaming chain unit itself.
///
class DirectChainOutputConnector : public StreamingOutputConnector
	{
	friend class DirectChainNestedInputConnector;
	protected:
		class DirectChainNestedInputConnector : public UnqueuedNestedInputConnector
			{
			friend class DirectChainOutputConnector;
			protected:
				DirectChainOutputConnector	*	outputConnector;
			public:
				DirectChainNestedInputConnector(uint32 id, VirtualStreamingChainUnit * unit, DirectChainOutputConnector * outputConnector);

				virtual STFResult ReceivePacket(StreamingDataPacket * packet);
				virtual STFResult GetStreamTagIDs(VDRTID * & ids);
			};

		DirectChainNestedInputConnector		nestedInputConnector;
	public:
		DirectChainOutputConnector(uint32 id, VirtualStreamingChainUnit * unit);

		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2);

		/// Get the nested input connector of this connector pair
		DirectChainNestedInputConnector * GetChainInputConnector(void) {return &nestedInputConnector;}
	};


typedef DirectChainInputConnector	*	DirectChainInputConnectorPtr;
typedef DirectChainOutputConnector	*	DirectChainOutputConnectorPtr;


/// The virtual part of a generic streaming chain unit, see physical part for detailed
/// explanation.
class VirtualGenericStreamingChainUnit : public VirtualStreamingChainUnit
	{
	protected:
		PhysicalGenericStreamingChainUnit	*	physicalUnit;		/// The physical unit
		int												numChainInputs;	/// Number of actual outside inputs
		DirectChainInputConnectorPtr			*	chainInputs;		/// The input/nested output pairs
		int												numChainOutputs;	/// Number of actual outside outputs
		DirectChainOutputConnectorPtr			*	chainOutputs;		/// The nested input/output pairs

	public:
		VirtualGenericStreamingChainUnit(PhysicalGenericStreamingChainUnit * physical);
		~VirtualGenericStreamingChainUnit(void);

 		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult IsPushingChain(uint32 connectorID);

		//
		// IStreamingChainUnit functions
		//
		virtual STFResult NestedReceiveAllocator(uint32 nestedConnectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult NestedIsPushingChain(uint32 nestedConnectorID);

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
