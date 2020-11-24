///
/// @brief      Extracts elementary streams from DVD PES streams
///

#ifndef DVDPESSTREAMUNPACKER_H
#define DVDPESSTREAMUNPACKER_H

#include "DVDPESSplitter.h"

class DVDPESStreamUnpackerUnit : public SharedPhysicalUnit
	{
	protected:
		uint32	rangesThreshold;
	public:
		DVDPESStreamUnpackerUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
	};

class VirtualDVDPESStreamUnpackerUnit : public VirtualNonthreadedStandardInOutStreamingUnit
	{
	protected:
		DataRangeStreamQueue		streamQueue;

		enum DVDPESState
			{
			DVDPESS_PARSE_PACKHEADER_0,
			DVDPESS_PARSE_PACKHEADER_1,
			DVDPESS_PARSE_PACKHEADER_2,
			DVDPESS_PARSE_PACKHEADER_3,
			DVDPESS_PARSE_PAYLOAD,
			DVDPESS_ANALYZE_PACKET,
			DVDPESS_DELIVER_PACKET,
			DVDPESS_DELIVER_RANGES,
			DVDPESS_DELIVER_END_TIME,
			DVDPESS_DELIVER_GROUP_END,
			DVDPESS_DELIVER_TAGS,
			DVDPESS_DELIVER_GROUP_START,
			DVDPESS_DELIVER_START_TIME
			} state;

		uint32	packetSize;

		STFResult ParsePESPacket(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		STFResult AnalyzePESPacket(void);
		STFResult DeliverPESPacket(void);

		virtual STFResult ParseBeginConfigure(void);
		virtual STFResult ParseConfigure(TAG *& tags);
		virtual STFResult ParseCompleteConfigure(void);

		//
		// Range Processing
		//
		virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);

		/// Reset Streaming Data Packet parser
		virtual STFResult ResetParser(void);

		virtual STFResult ResetInputProcessing(void);

		/// For processing Flush operations in a derived class
		virtual STFResult ProcessFlushing(void);

 		/// Returns if input data is currently being used for processing
		virtual bool InputPending(void);
	public:
		VirtualDVDPESStreamUnpackerUnit(DVDPESStreamUnpackerUnit * physical, uint32 rangesThreshold);

		STFResult InternalUpdate(void) {STFRES_RAISE_OK;}

#if _DEBUG
		virtual STFString GetInformation(void);
#endif
	};

#endif
