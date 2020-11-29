#include "AC3Decoder.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

UNIT_CREATION_FUNCTION(CreatePhysicalAC3Decoder, PhysicalAC3DecoderUnit)

//
// Virtual Unit methods
//
STFResult VirtualAC3DecoderUnit::DecodeData(const VDRDataRange & range, uint32 & offset)
	{
	uint8 * end  = range.GetStart() + range.size;
	uint8 * current = range.GetStart() + offset;

	for (;;)
		{
		switch (decodeDataState)
			{
			case STDD_SEARCH_HEADER:
				//   Copy 7 bytes from Range Bytes read pointer at start of framebuffer.
				frameBufferPtr = frameBuffer;
				if ((end - current) < (7 - headerBytesFound))
					{
					//   If 7 bytes cannot be read (end of range), exit.
					memcpy((frameBuffer + headerBytesFound), current, (end - current));
					headerBytesFound += (int)(end - current);
					STFRES_RAISE_OK; // End of datarange reached.
					}
				else
					{
					memcpy((frameBuffer + headerBytesFound), current, (7 - headerBytesFound));
					current += (int)(7 - headerBytesFound);
					headerBytesFound = 7; // All Found.
					frameBufferPtr = frameBuffer + 7;
					decodeDataState = STDD_READ_HEADER;
					}
				// Intentional drop-through
			case STDD_READ_HEADER:
				headerBytesFound = 0;
				//   Read the header and find the length of the frame
				frameLength = a52_syncinfo(frameBuffer, &flags, &sampleRate, &bitRate);
				if (frameLength == 0)
					{
					// Move the search pointer one byte further
					current -= (int)6;
					decodeDataState = STDD_SEARCH_HEADER;
					break;
					}
				frameBufferPtr = frameBuffer + 7;
				frameLength -= (int)7; /* Remove the header */
				decodeDataState = STDD_READ_PAYLOAD;
				// Intentional drop-through
			case STDD_READ_PAYLOAD:
				//  copy range bytes in framebuffer until the whole frame is inside,
				//  keep track of RangeBytes read pointer.
				if ((end - current) < frameLength )
					{
					// if range bytes ( plus what is already copied from previous range)
					// is smaller than remaining length of frame, exit.
					memcpy(frameBufferPtr, current, (end - current));
					frameBufferPtr += (int)(end - current);
					frameLength -= (int)(end - current);
					STFRES_RAISE_OK; // End of datarange reached.
					}
				else
					{
					memcpy(frameBufferPtr, current, frameLength);
					current += (int)frameLength;
					frameBufferPtr += (int)frameLength; // Points now at frame end.
					decodeDataState = STDD_DECODE_FRAME;
					}
				// Intentional drop-through
			case STDD_DECODE_FRAME:
				//   If length of frame is reached, decode frame and clear framebuffer when done.
				level = 1;
				bias = 384;
				level *= gain;
				if (a52_frame(a52State, frameBuffer, &flags, &level, bias) != 0)
					{
					DP("a52_frame ERROR\n");
					ASSERT(0);
					}

				if (framenum == 0)
					{
					STFRES_REASSERT(outputFormatter.BeginSegment(segmentCount, false));
					STFRES_REASSERT(outputFormatter.PutDataDiscontinuity());
					}

				if (dataPropertiesChanged & AC3_ACMOD_CHANGED)
					{
					STFRES_REASSERT(outputFormatter.PutTag(SET_AUDIO_STREAM_SAMPLE_RATE(sampleRate)));
					DP("bit rate = %d\n", bitRate);
					STFRES_REASSERT(outputFormatter.PutTag(SET_AUDIO_STREAM_AUDIO_CODING_MODE(GetAudioChannelMode())));
					dataPropertiesChanged &= ~AC3_ACMOD_CHANGED;
					//Each call to PutTag() places one tag into the stream, the call to CompleteTags() finally places the tag done
					STFRES_REASSERT(outputFormatter.CompleteTags());
					}

				STFRES_REASSERT(outputFormatter.BeginGroup(framenum, false, true));

				// Can set dynamic range here optionally.
				a52_dynrng(a52State, NULL, NULL);
				for (blockCount=0; blockCount<6; blockCount++)
					{
					if (a52_block(a52State) != 0)
						{
						DP("a52_block ERROR, blockCount = %d\n", blockCount);
						ASSERT(0);
						}
					// convert samples to integer and queue to pcmplayer
					DeliverData(a52_samples (a52State));
					}
				STFRES_REASSERT(outputFormatter.CompleteGroup(false));
				framenum++;
				decodeDataState = STDD_SEARCH_HEADER;
				break;
			default:
				break;
			}
		}
	STFRES_RAISE_OK;
	}


STFResult VirtualAC3DecoderUnit::DeliverData(sample_t * _samples)
	{
	// Copy the sample into the memoryblock
	memcpy (range[rangeCounter%10].block->GetStart(), _samples, range[rangeCounter%10].block->GetSize());
	STFRES_REASSERT(outputFormatter.PutRange(range[rangeCounter%10]));
	outputFormatter.Commit(); // Needed?
	rangeCounter++;
	STFRES_RAISE_OK;
	}


///
/// Parent class overrides
///
STFResult PhysicalAC3DecoderUnit::Create(uint64 * createParams)
	{
	STFRES_ASSERT(GetNumberOfParameters(createParams) == 7, STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);

	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_THREAD_PRIORITY, threadPriority)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_THREAD_STACKSIZE, threadStackSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetStringParameter(createParams, AC3_DECODER_THREAD_NAME, threadName)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_DATABUFFER_COUNT, ac3DecoderDataBufferCount)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_DATABUFFER_SIZE, ac3DecoderDataBufferSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_MEMORYALIGNMENTFACTORINBYTES, ac3DecoderMemoryAlignmentFactorInBytes)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, AC3_DECODER_DATABUFFER_BLOCKSIZE, ac3DecoderDataBufferBlockSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_RAISE_OK;
	}

STFResult PhysicalAC3DecoderUnit::Connect(uint64 localID, IPhysicalUnit * source)
	{
	switch (localID)
		{
		case 0:
			allocatorPU = source;
			break;
		case 1: // message handling....
			break;
		default:
			STFRES_RAISE(STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
			break;
		}
	STFRES_RAISE_OK;
	}

STFResult PhysicalAC3DecoderUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualAC3DecoderUnit(this));
	if (unit)
		STFRES_REASSERT(unit->Connect(parent, root));
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
	STFRES_RAISE_OK;
	}

VirtualAC3DecoderUnit::VirtualAC3DecoderUnit(PhysicalAC3DecoderUnit * physicalAC3Decoder)
	: VirtualThreadedStandardInOutStreamingUnitCollection(physicalAC3Decoder,	1,	// Number of children in the virtual unit collection,
																2,	// Number of output packets in output connector,
																16,	// Input connector queue size,
																0,	// Input connector threshold,
																"AC3Decoder")	// Thread ID name,
	{
	this->physicalAC3Decoder = physicalAC3Decoder;
	}


STFResult VirtualAC3DecoderUnit::AllocateChildUnits (void)
	{
	IVirtualUnit* outputPoolAllocatorVU = NULL;
	STFResult res;

	if (NULL == physicalAC3Decoder->allocatorPU)
		STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);

	STFRES_REASSERT(physicalAC3Decoder->allocatorPU->CreateVirtual(outputPoolAllocatorVU, this, root != NULL ? root : this));
	if (NULL == outputPoolAllocatorVU)
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);

	if (STFRES_FAILED(res = AddLeafUnit(outputPoolAllocatorVU)))
		{
		outputPoolAllocatorVU->Release();
		STFRES_RAISE(res);
		}

	STFRES_REASSERT(outputPoolAllocatorVU->QueryInterface(VDRIID_VDR_MEMORYPOOL_ALLOCATOR, (void *&)outputPoolAllocator));
	if (NULL == outputPoolAllocator)
		STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);

	res = outputPoolAllocatorVU->Configure(
		SET_MEMPOOL_TOTALSIZE(physicalAC3Decoder->ac3DecoderDataBufferCount * physicalAC3Decoder->ac3DecoderDataBufferSize) +
		SET_MEMPOOL_BLOCKSIZE(physicalAC3Decoder->ac3DecoderDataBufferBlockSize) +
		SET_MEMPOOL_ALIGNMENT_FACTOR(physicalAC3Decoder->ac3DecoderMemoryAlignmentFactorInBytes) +
		SET_MEMPOOL_POOLNAME((char*) "decoded data buffers") +
		TAGDONE);

#if _DEBUG
	uint32 numBlocksGotten;
	res = outputPoolAllocatorVU->Configure(GET_MEMPOOL_NUMBER_OF_BLOCKS(numBlocksGotten) + TAGDONE);
	assert (STFRES_SUCCEEDED(res));
	assert (numBlocksGotten == physicalAC3Decoder->ac3DecoderDataBufferCount); // block count
#endif

	STFRES_REASSERT(outputPoolAllocatorVU->ActivateAndLock((VDRUALF_REALTIME_PRIORITY | VDRUALF_WAIT),
		STFHiPrec64BitTime(), STFHiPrec32BitDuration(0)));

	STFRES_RAISE_OK;
	}


STFResult VirtualAC3DecoderUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
	while (range < num)
		{
		STFRES_REASSERT(this->DecodeData(ranges[range], offset));
		range++;
		}
	STFRES_RAISE_OK;
	}


STFResult VirtualAC3DecoderUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			dataPropertiesChanged = AC3_ACMOD_CHANGED;
			groupNumber = 0;
			framenum = 0;
			frameLength = 0;
			headerBytesFound = 0;
			segmentCount = 0;
			a52State = a52_init(0); // No acceleration
			if (a52State == NULL)
				{
				DP ("Could not allocate a52 object.\n");
				return STFRES_NOT_ENOUGH_MEMORY;
				}
			gain = 1;
			for (int i = 0; i < 10; i++)
				{
				if ( STFRES_FAILED(outputPoolAllocator->GetMemoryBlocks(&memoryBlock, 0, 1, numObtainedBlocks, this)))
					{
					return STFRES_NOT_ENOUGH_MEMORY;
					}
				range[i].Init(memoryBlock, 0, memoryBlock->GetSize());
				}
			rangeCounter = 0;
			// Immediately fake the signal that enough data was received to start
			inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
			break;
		case VDR_STRMCMD_FLUSH:
			a52_free(a52State);
			break;
		case VDR_STRMCMD_DO:
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;
		default:
			DP("*** Unhandled STRMCMD in VirtualAC3DecoderUnit::BeginStreamingCommand! ***\n");
			break;
		}
	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualThreadedStandardInOutStreamingUnitCollection::BeginStreamingCommand(command, param));
	}


STFResult VirtualAC3DecoderUnit::PreemptUnit(uint32 flags)
	{
	STFRES_REASSERT(VirtualThreadedStandardInOutStreamingUnitCollection::PreemptUnit (flags));

	if (flags & (VDRUALF_PREEMPT_START_NEW | VDRUALF_PREEMPT_RESTART_PREVIOUS))
		{
		ResetThreadSignal();
		STFRES_REASSERT(StartThread());
		}

	if (flags & (VDRUALF_PREEMPT_STOP_PREVIOUS | VDRUALF_PREEMPT_STOP_NEW))
		{
		StopThread();
		Wait();
		}

	if (flags & (VDRUALF_PREEMPT_CHANGE | VDRUALF_PREEMPT_RESTORE))
		{
		// not sure about this yet
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualAC3DecoderUnit::Initialize(void)
	{
	STFRES_REASSERT(VirtualThreadedStandardInOutStreamingUnitCollection::Initialize());
	STFRES_REASSERT(SetThreadPriority((STFThreadPriority) physicalAC3Decoder->threadPriority));
	STFRES_REASSERT(SetThreadStackSize(physicalAC3Decoder->threadStackSize));
	STFRES_REASSERT(SetThreadName(physicalAC3Decoder->threadName));
	STFRES_RAISE_OK;
	}
