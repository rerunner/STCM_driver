#ifndef ICHAINLINK_H
#define ICHAINLINK_H

///
/// @brief      Chain Link Input & Output Interfaces
///
/// Chain Link Input & Output Interfaces
/// Chain Links are needed to connect independent Streaming Chains.
///


#include "VDR/Source/Streaming/IStreaming.h"


///////////////////////////////////////////////////////////////////////////////
// Chain Link Input Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_CHAIN_LINK_INPUT = 0x8000008f;


/// Interface of a Chain Link Input Phyiscal Unit
class IChainLinkInput : public virtual IVDRBase
	{
	public:
		/// Receive allocator from downstream Streaming Chain
		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator) = 0;

		/// Receive upstream notifications from downstream Streaming Chain
		virtual STFResult SendUpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;

		/// Check, whether this chain is a pushing chain
		virtual STFResult IsPushingChain(void) = 0;

		/// Retrieve state of source streaming chain.
		/// Returns STFRES_OBJECT_NOT_CURRENT if there is no activated virtual chain link input.
		virtual STFResult GetStreamingState(VDRStreamingState & state) = 0;

		/// Return the current stream time offset in the upstream chain
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Chain Link Output Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_CHAIN_LINK_OUTPUT = 0x80000090;


/// Interface of a Chain Link Output Physical Unit
class IChainLinkOutput : public virtual IVDRBase
	{
	public:
		/// Register peer Link Input Unit
		virtual STFResult RegisterLinkInput(IChainLinkInput * input) = 0;

		/// Send single Streaming Data Packet
		virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;

		/// Retrieve state of target streaming chain.
		/// Returns STFRES_OBJECT_NOT_CURRENT if there is no activated virtual chain link output.
		virtual STFResult GetStreamingState(VDRStreamingState & state) = 0;

		/// Check, whether this chain is a pushing chain
		virtual STFResult IsPushingChain(void) = 0;
	};


#endif	// #ifndef ICHAINLINK_H
