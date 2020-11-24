#ifndef AC3DECODER_H
#define AC3DECODER_H

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"
#include "VDR/Interface/Unit/Audio/IVDRAudioStreamTypes.h"

extern "C" 
{
/* temporary (really!) AC3 library includes */
#include "GPL/a52dec-0.7.4/include/a52.h"
#include "GPL/a52dec-0.7.4/include/mm_accel.h"
}

#define AC3_ACMOD_CHANGED	MKFLAG(0)

class PhysicalAC3DecoderUnit : public SharedPhysicalUnit
{
	friend class VirtualAC3DecoderUnit;

protected:
	enum
	{
		AC3_DECODER_THREAD_PRIORITY = 0,
		AC3_DECODER_THREAD_STACKSIZE = 1,
		AC3_DECODER_THREAD_NAME = 2,
		AC3_DECODER_DATABUFFER_COUNT = 3,
		AC3_DECODER_DATABUFFER_SIZE = 4,
		AC3_DECODER_MEMORYALIGNMENTFACTORINBYTES = 5,
		AC3_DECODER_DATABUFFER_BLOCKSIZE = 6
	};
	IPhysicalUnit *allocatorPU;
	uint32 threadPriority;
	uint32 threadStackSize;
	char *threadName;
	uint32 ac3DecoderDataBufferCount;
	uint32 ac3DecoderDataBufferSize;
	uint32 ac3DecoderMemoryAlignmentFactorInBytes;
	uint32 ac3DecoderDataBufferBlockSize;

public:
	PhysicalAC3DecoderUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {allocatorPU = NULL;}

	//
	// IPhysicalUnit interface implementation
	//
	virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	virtual STFResult Create(uint64 * createParams);
	virtual STFResult Connect(uint64 localID, IPhysicalUnit* source);
	virtual STFResult Initialize(uint64 * depUnitsParams) {STFRES_RAISE_OK;}
};


// Streaming Unit with an input and an output
class VirtualAC3DecoderUnit : public VirtualThreadedStandardInOutStreamingUnitCollection
{
private:
	a52_state_t * a52State; // State of the AC3 decoder
	int sampleRate;
	int flags;
	int bitRate;
	uint8 frameBuffer[3840];
	uint8 *frameBufferPtr;
	uint32 numObtainedBlocks; ///< Number of allocated memory blocks
	VDRMemoryBlock *memoryBlock;
	VDRDataRange	range[10];
	int rangeCounter;
	uint32 dataPropertiesChanged; ///< Check when data proporties change, 

        enum StreamingDecodeDataState
	{
		STDD_SEARCH_HEADER,
		STDD_READ_HEADER,
		STDD_READ_PAYLOAD,
		STDD_DECODE_FRAME
	} decodeDataState;

	sample_t level, bias;
	int blockCount;
	int headerBytesFound;
	int frameLength;
	int segmentCount;

	sample_t gain;
	int framenum;

	virtual STFResult DecodeData(const VDRDataRange & range, uint32 & offset);
	virtual STFResult DeliverData(sample_t * _samples);
	virtual VDRAudioCodingMode GetAudioChannelMode()
	{
		int copyFlags = flags;
		if (copyFlags&A52_LFE)
			copyFlags&=~A52_LFE;
		if (copyFlags&A52_ADJUST_LEVEL)
			copyFlags&=~A52_ADJUST_LEVEL;

		if (copyFlags == A52_MONO)
			return VDR_ACMOD_1_0;
		else if (copyFlags == A52_STEREO)
			return VDR_ACMOD_2_0;
		else if (copyFlags == A52_3F)
			return VDR_ACMOD_3_0;
		else if (copyFlags == A52_2F1R)
			return VDR_ACMOD_1_0;
		else if (copyFlags == A52_3F1R)
			return VDR_ACMOD_3_1;
		else if (copyFlags == A52_2F2R)
			return VDR_ACMOD_2_2;
		else if (copyFlags == A52_3F2R)
			return VDR_ACMOD_3_2;
		else
			return VDR_ACMOD_2_0;
	}

protected:
	PhysicalAC3DecoderUnit *physicalAC3Decoder;
	IVDRMemoryPoolAllocator *outputPoolAllocator;

	STFHiPrec64BitDuration	currentSystemTimeOffset;
	STFHiPrec64BitTime		currentStreamTime,
	systemStartTime;

	//
	// IVirtualUnit
	//
	virtual STFResult PreemptUnit(uint32 flags);

	//
	// VirtualUnitCollection pure virtual function overrides
	//
	virtual STFResult AllocateChildUnits (void);
	virtual STFResult Initialize(void);

	//
	// Data range parsing
	//
	virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
	virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
	virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{	if (requestNotification)
			inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_START, groupNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseEndGroup(uint16 groupNumber, bool requestNotification)
	{	if (requestNotification)
			inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_END, groupNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{	if (requestNotification)
		inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_START, segmentNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{	if (requestNotification)
			inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_END, segmentNumber, 0);
		STFRES_RAISE_OK;
	}

	virtual bool InputPending(void)
	{
		return false;
	}

public:
	VirtualAC3DecoderUnit(PhysicalAC3DecoderUnit * physicalAC3Decoder);
	~VirtualAC3DecoderUnit(void) {};

#if _DEBUG
	//
	// IStreamingUnitDebugging functions
	//
	virtual STFString GetInformation(void)
	{
		return STFString("VirtualAC3DecoderUnit ") + STFString(physical->GetUnitID());
	}
#endif
};

#endif
