#ifndef CHAINLINK_H
#define CHAINLINK_H

///
/// @brief 		 Chain Links
///

///
/// Chain Links
/// 
/// Chain Links can be used to connect two independent Streaming Chains (i.e. possibly
/// being in a different streaming or activation state) by making use of a direct
/// physical-to-physical unit connection.
///
/// The physical Chain Link Input and the physical Chain Link Output unit are directly
/// connected, without another object in between. A Virtual unit of the link input
/// has to be made part of the "producing" streaming chain, and a virtual unit of the
/// link output must be part of the "receiving" streaming chain.
/// 
/// Data packets received by the link input are treated as follows:
///
/// If the receiving streaming chain is in the correct streaming state ("ready" or "streaming"),
/// a data packet will be forwarded from the link input to the link output physical unit
/// and injected into the target streaming chain.
///
/// If that is not the case (by the target chain either being passivated or in the wrong
/// streaming state), the data packet and the data ranges contained in it are immediately
/// released by the link input unit. However, in that case the link input will still generate
/// the required segment/group start/end and streaming startup messages.
///
/// Special attention has to be taken regarding flushing:
///
/// Flushing must always occur by the principle "receiving streaming chain first", so
/// that all memory blocks and data packets residing in the receiving streaming chain
/// are returned to the producing streaming chain before that is flushed itself.
///
/// Also, special attention has to be taken regarding memory pool allocators:
///
/// When the receiving streaming chain is passivated, it will cause sending of a
/// "NULL" pool allocator to the producing streaming chain. This clearly requires
/// that the producing streaming chain must be flushed and perhaps even passivated
/// before the receiving chain is passivated.

#include "Device/Interface/Unit/Datapath/IChainLink.h"
#include "VDR/Source/Base/VDRBase.h"
#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Unit/PhysicalUnit.h"


///////////////////////////////////////////////////////////////////////////////
// Physical Chain Link Input Unit
///////////////////////////////////////////////////////////////////////////////

// Forward declaration
class VirtualChainLinkInput;


class ChainLinkInput : public virtual IChainLinkInput,
							  public ExclusivePhysicalUnit
	{
	friend class VirtualChainLinkInput;

	protected:
		IChainLinkOutput * output;							/// The peer chain link output physical unit

		IVDRMemoryPoolAllocator	*	allocator;			/// Holds a memory pool allocator received 
																	/// from the downstream streaming chain

		VirtualChainLinkInput	*	curVirtualInput;	/// The currently active virtual chain link input unit

		/// Called from the Virtual Stream Mixer Input to signal activation.
		/// The memory pool allocator is then provided to the virtual unit.
		STFResult SetVirtualChainLinkInput(VirtualChainLinkInput * unit);

	public:
		ChainLinkInput(VDRUID unitID) : ExclusivePhysicalUnit(unitID)
			{
			output				= NULL;
			allocator			= NULL;
			curVirtualInput	= NULL;
			}
		
		virtual ~ChainLinkInput();

		//
		// IVDRBase functions
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// IChainLinkInput interface implementation
		//
		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator);
		virtual STFResult SendUpstreamNotification(VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult IsPushingChain(void);
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset);
		virtual STFResult GetStreamingState(VDRStreamingState & state);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};



///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Input Streaming Unit
///////////////////////////////////////////////////////////////////////////////

/// Virtual Chain Link Input Streaming Unit
class VirtualChainLinkInput : public VirtualStreamingUnit
	{
	friend class ChainLinkInput;

	protected:
		ChainLinkInput			*	physicalInput;

		UnqueuedInputConnector	inputConnector;
		IStreamingClock		*	streamingClock;


		/// Receive memory pool allocator from downstream streaming chain. 
		/// Called by the physical Chain Link Input unit.
		STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
			{
			STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
			}

		/// Called from the physical Link Input unit to receive upstream notifications.
		STFResult LinkInputNotification(VDRMID message, uint32 param1, uint32 param2);

	public:
		VirtualChainLinkInput(ChainLinkInput * physicalInput) : VirtualStreamingUnit(physicalInput),
																				  inputConnector(0, this)
			{
			this->physicalInput	= physicalInput;

			AddConnector((IStreamingConnector*)&inputConnector);		// => ID 0
			}

		//
		// IStreamingUnit Streaming Command Handling
		//
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);

		//
		// IStreamingUnit functions
		//
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult IsPushingChain(void);
		virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock);

		//
		// IVirtualUnit
		//
		virtual STFResult PreemptUnit (uint32 flags);

#if _DEBUG
		// Debugging facilities
		virtual STFString GetInformation(void)
			{
			return STFString("VirtualChainLinkInput ") + STFString(physical->GetUnitID(), 8, 16);
			}
#endif
	};



///////////////////////////////////////////////////////////////////////////////
// Physical Chain Link Output Unit
///////////////////////////////////////////////////////////////////////////////

// Forward declaration
class VirtualChainLinkOutput;

class ChainLinkOutput : public virtual IChainLinkOutput,
								public ExclusivePhysicalUnit
								
	{
	friend class VirtualChainLinkOutput;

	protected:
		IChainLinkInput			*	input;

		VirtualChainLinkOutput	*	curOutputUnit;

		uint32							priority;
		uint32							chainDelay;

		void SetVirtualChainLinkOutput(VirtualChainLinkOutput * unit)
			{
			this->curOutputUnit = unit;
			}
		
	public:
		ChainLinkOutput(VDRUID unitID) : ExclusivePhysicalUnit(unitID)
			{
			input				= NULL;
			curOutputUnit	= NULL;
			}

		virtual ~ChainLinkOutput();

		//
		// IVDRBase functions
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

		//
		// IChainLinkOutput interface implementation
		//
		virtual STFResult RegisterLinkInput(IChainLinkInput * input);
		virtual STFResult SendPacket(StreamingDataPacket * packet);
		virtual STFResult GetStreamingState(VDRStreamingState & state);
		virtual STFResult IsPushingChain(void);

#if _DEBUG
		// Debugging facilities
		virtual STFString GetInformation(void)
			{
			return STFString("ChainLinkOutput ") + STFString(GetUnitID(), 8, 16);
			}
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualChainLinkOutput : public VirtualStreamingUnit,
                               public virtual IStreamingClockClient
	{
	friend class ChainLinkOutput;

	private:
		ChainLinkOutput	*	physicalOutput;

	protected:
		
		//
		// "External" connectors
		//
		StreamingOutputConnector	outputConnector;
		IStreamingClock			*	streamingClock;
		uint32							clockID;
		bool								isPushingChainLink, isPullingChainLink;

		bool								firstPacket, discontinuityPending, insideGroup, insideSegment;
		STFHiPrec64BitTime			previousInputTime;
		STFHiPrec64BitTime			currentStreamTime, systemStartTime;
		STFHiPrec64BitDuration		currentSystemTimeOffset;

		STFInterlockedInt				pendingLock;		/// Protection of the pending packet processing
		StreamingDataPacket		*	pendingPacket, * targetPacket;
		volatile bool					packetBounced; 
		volatile bool					flushRequest;
		volatile bool					processRequest;
		volatile bool					stopRequest;
		uint16							segmentNumber, groupNumber;

		STFResult CalculatePacketTime(const	STFHiPrec64BitTime & inputTime, STFHiPrec64BitTime & outputTime);

		virtual STFResult SendTargetPacket(void);
		virtual STFResult ReleaseTargetPacket(void);
		virtual STFResult ReleasePendingPacket(void);
		virtual STFResult ProcessPendingPacket(void);
		virtual STFResult IsPushingChain(void);
	public:
		VirtualChainLinkOutput(ChainLinkOutput * physicalOutput);

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);
		virtual STFResult IsPushingChain(uint32 connectorID);
		virtual STFResult CompleteConnection(void);
		virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock);

		//
		// IStreamingUnit Streaming Command Handling
		//
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);

		//
		// IStreamingClockClient interface implementation
		//
		virtual STFResult SetStartupFrame(uint32 frameNumber, const STFHiPrec64BitTime & startTime);
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void);

		//
		// IVirtualUnit
		//
		virtual STFResult PreemptUnit(uint32 flags);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		// Debugging facilities
		virtual STFString GetInformation(void)
			{
			return STFString("VirtualChainLinkOutput ") + STFString(GetUnitID(), 8, 16);
			}
#endif
	};


#endif // CHAINLINK_H
