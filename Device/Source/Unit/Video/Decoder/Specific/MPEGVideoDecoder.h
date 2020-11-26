#ifndef MPEGVIDEODECODER_H
#define MPEGVIDEODECODER_H

///
/// @brief
///

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
extern "C"
{
#include "GPL/libmpeg2-0.5.1/include/mpeg2.h"
#include <unistd.h>
}

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"
#include "VDR/Interface/Unit/Video/Decoder/IVDRVideoDecoderTypes.h"

#define ALIGN_16(p) ((void*)(((uintptr_t)(p) + 15) & ~((uintptr_t)15)))

#define MPEG2_VIDEOTYPE_CHANGED	MKFLAG(0)

///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

class MPEGVideoDecoderUnit : public SharedPhysicalUnit
	{
	friend class VirtualMPEGVideoDecoderUnit;

protected:
	enum
		{
		MPEG_VIDEO_DECODER_THREAD_PRIORITY = 0,
		MPEG_VIDEO_DECODER_THREAD_STACKSIZE = 1,
		MPEG_VIDEO_DECODER_THREAD_NAME = 2,
		MPEG_VIDEO_DECODER_DATABUFFER_COUNT = 3,
		MPEG_VIDEO_DECODER_DATABUFFER_SIZE = 4,
		MPEG_VIDEO_DECODER_MEMORYALIGNMENTFACTORINBYTES = 5,
		MPEG_VIDEO_DECODER_DATABUFFER_BLOCKSIZE = 6
		};
	IPhysicalUnit *allocatorPU;
	uint32 threadPriority;
	uint32 threadStackSize;
	char *threadName;
	uint32 mpegVideoDecoderDataBufferCount;
	uint32 mpegVideoDecoderDataBufferSize;
	uint32 mpegVideoDecoderMemoryAlignmentFactorInBytes;
	uint32 mpegVideoDecoderDataBufferBlockSize;

public:
	MPEGVideoDecoderUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {allocatorPU = NULL;}

	//
	// IPhysicalUnit interface implementation
	//
	virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

	virtual STFResult Create(uint64 * createParams);
	virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
	virtual STFResult Initialize(uint64 * depUnitsParams) {STFRES_RAISE_OK;}
	};


// Streaming Unit with an input and an output
class VirtualMPEGVideoDecoderUnit : public VirtualThreadedStandardInOutStreamingUnitCollection
	{
	struct fbuf_s
		{
		uint8_t * mbuf[3]; // Unaligned memory buffer pointers
		uint8_t * yuv[3]; // Aligned memory buffer pointers
		int used;
		} fbuf[3]; // Frame buffer structure

	struct fbuf_s * get_fbuf(void)
		{
		for (int i = 0; i < 3; i++)
			if (!fbuf[i].used)
				{
				fbuf[i].used = 1;
				return fbuf + i;
				}
		return NULL;
		}
protected:
	MPEGVideoDecoderUnit		*physicalMPEGVideoDecoder;
	IVDRMemoryPoolAllocator	*outputPoolAllocator;

	STFHiPrec64BitDuration		currentSystemTimeOffset;
	STFHiPrec64BitTime			currentStreamTime, systemStartTime;

	mpeg2dec_t					*decoder;
	const mpeg2_info_t			*info;
	mpeg2_state_t				state;
	size_t						size;
	int						framenum;

	uint8_t			*yPlane, *uPlane, *vPlane;
	size_t				yPlaneSz, uvPlaneSz;
	int				uvPitch;
	uint8_t			*frameBuffer;
	int				width, height, ysize, uvsize, dest_pitch;
	struct fbuf_s		*current_fbuf;

	STFHiPrec64BitTime		startTime;
	STFHiPrec64BitTime		endTime;
	STFHiPrec32BitDuration	timeDiff;

	SequenceHeaderExtension seqHeaderExtInfo; // See IVDRVideoDecoderTypes.h
	int segmentCount;
	uint32 dataPropertiesChanged;
	uint32 numObtainedBlocks; ///< Number of allocated memory blocks
	VDRMemoryBlock *memoryBlock;
	VDRDataRange	range[2]; // One decoding, one displaying
	int rangeCounter;

	virtual STFResult DecodeData(const VDRDataRange & range, uint32 & offset);
	virtual STFResult DeliverData(fbuf_s * infoBuffer);

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

	//
	// Range information parsing
	//
	virtual STFResult ParseDataDiscontinuity(void) {STFRES_RAISE_OK;}
	virtual STFResult ParseTimeDiscontinuity(void) {STFRES_RAISE_OK;}
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

	/// Returns if input data is currently being used for processing.
	virtual bool InputPending(void) {return false;}

public:
	/// Specific constructor.
	/// @param physical: Pointer to interface of corresponding physical unit
	VirtualMPEGVideoDecoderUnit(MPEGVideoDecoderUnit * physicalMPEGVideoDecoder);

	//
	// IStreamingUnit interface implementation
	//
	virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
#if _DEBUG
	//
	// IStreamingUnitDebugging functions
	//
	virtual STFString GetInformation(void)
	{
		return STFString("VirtualMPEGVideoDecoderUnit ") + STFString(physical->GetUnitID());
	}
#endif
	};


#endif // #ifndef MPEGVIDEODECODER_H
