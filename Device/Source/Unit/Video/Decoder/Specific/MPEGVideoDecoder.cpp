///
/// @brief Implementations for Threaded Video Decoder Streaming Unit
///

#include "MPEGVideoDecoder.h"
#include "STF/Interface/STFDebug.h"
#include "STF/Interface/STFTimer.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

#define DBG_VDR_CMD_STRING(c)	((c==VDR_STRMCMD_NONE)?"NONE": \
	(c==VDR_STRMCMD_BEGIN)?"BEGIN": \
	(c==VDR_STRMCMD_DO)?"DO": \
	(c==VDR_STRMCMD_STEP)?"STEP": \
	(c==VDR_STRMCMD_FLUSH)?"FLUSH": \
	"???")

#define DEBUG_STREAMING_CMD 0

#if _DEBUG && DEBUG_STREAMING_CMD
	#define DP_STREAMCMD(unit, str, cmd) DebugPrint("%s : "str" (%s)\n", (char*) unit->GetInformation(), DBG_VDR_CMD_STRING(cmd))
#else
	#define DP_STREAMCMD(unit, str, cmd) do {} while(0);
#endif


///////////////////////////////////////////////////////////////////////////////
// Physical Unit methods
///////////////////////////////////////////////////////////////////////////////

UNIT_CREATION_FUNCTION(CreateMPEGVideoDecoder, MPEGVideoDecoderUnit)


STFResult MPEGVideoDecoderUnit::Create(uint64 * createParams)
	{
	STFRES_ASSERT(GetNumberOfParameters(createParams) == 7, STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);

	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_THREAD_PRIORITY, threadPriority)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_THREAD_STACKSIZE, threadStackSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetStringParameter(createParams, MPEG_VIDEO_DECODER_THREAD_NAME, threadName)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_DATABUFFER_COUNT, mpegVideoDecoderDataBufferCount)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_DATABUFFER_SIZE, mpegVideoDecoderDataBufferSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_MEMORYALIGNMENTFACTORINBYTES, mpegVideoDecoderMemoryAlignmentFactorInBytes)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_ASSERT(STFRES_SUCCEEDED(GetDWordParameter(createParams, MPEG_VIDEO_DECODER_DATABUFFER_BLOCKSIZE, mpegVideoDecoderDataBufferBlockSize)), STFRES_BOARDCONSTRUCTION_INVALID_CONFIGURATION);
	STFRES_RAISE_OK;
	}

STFResult MPEGVideoDecoderUnit::Connect(uint64 localID, IPhysicalUnit * source)
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

STFResult MPEGVideoDecoderUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualMPEGVideoDecoderUnit(this));
	if (unit)
		STFRES_REASSERT(unit->Connect(parent, root));
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
// Virtual Unit methods
///////////////////////////////////////////////////////////////////////////////

VirtualMPEGVideoDecoderUnit::VirtualMPEGVideoDecoderUnit(MPEGVideoDecoderUnit * physicalMPEGVideoDecoder)
	: VirtualThreadedStandardInOutStreamingUnitCollection(physicalMPEGVideoDecoder,
														  1,	// Number of children in the virtual unit collection,
														  3,	// Number of output packets in output connector,
														  4,	// Input connector queue size,
														  0,	// Input connector threshold,
														  "MPEGVideoDecoder")	// Thread ID name,
	{
	this->physicalMPEGVideoDecoder = physicalMPEGVideoDecoder;
	}


STFResult VirtualMPEGVideoDecoderUnit::AllocateChildUnits (void)
	{
	IVirtualUnit* outputPoolAllocatorVU = NULL;
	STFResult res;

	if (NULL == physicalMPEGVideoDecoder->allocatorPU)
		STFRES_RAISE(STFRES_OBJECT_NOT_ALLOCATED);

	STFRES_REASSERT(physicalMPEGVideoDecoder->allocatorPU->CreateVirtual(outputPoolAllocatorVU, this, root != NULL ? root : this));
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
		SET_MEMPOOL_TOTALSIZE(physicalMPEGVideoDecoder->mpegVideoDecoderDataBufferCount * physicalMPEGVideoDecoder->mpegVideoDecoderDataBufferSize) +
		SET_MEMPOOL_BLOCKSIZE(physicalMPEGVideoDecoder->mpegVideoDecoderDataBufferBlockSize) +
		SET_MEMPOOL_ALIGNMENT_FACTOR(physicalMPEGVideoDecoder->mpegVideoDecoderMemoryAlignmentFactorInBytes) +
		SET_MEMPOOL_POOLNAME((char*) "decoded data buffers") +
		TAGDONE);

#if _DEBUG
	uint32 numBlocksGotten;
	res = outputPoolAllocatorVU->Configure(GET_MEMPOOL_NUMBER_OF_BLOCKS(numBlocksGotten) + TAGDONE);
	assert (STFRES_SUCCEEDED(res));
	assert (numBlocksGotten == physicalMPEGVideoDecoder->mpegVideoDecoderDataBufferCount); // block count
#endif

	STFRES_REASSERT(outputPoolAllocatorVU->ActivateAndLock((VDRUALF_REALTIME_PRIORITY | VDRUALF_WAIT),
		STFHiPrec64BitTime(), STFHiPrec32BitDuration(0)));

	STFRES_RAISE_OK;
	}


STFResult VirtualMPEGVideoDecoderUnit::PrepareDecoder()
	{
	preparing = false;
	rangeCounter = 0;
	//Signal upstream that the decoder is ready to start
	inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
	STFRES_RAISE_OK;
	}

STFResult VirtualMPEGVideoDecoderUnit::DecodeData(const VDRDataRange & range, uint32 & offset)
	{
	uint8_t * buffer;
	uint8_t * end;
	STFResult res;

	if (preparing)
		{
		PrepareDecoder();
		}

	buffer = range.GetStart() + offset;
	end = buffer + range.size - offset;

	mpeg2_buffer (decoder, buffer, end);

	while (!flushRequest)
		{
		state = mpeg2_parse (decoder);
		switch (state)
			{
			case STATE_BUFFER:
				mpeg2_buffer (decoder, buffer, end);
				STFRES_RAISE_OK;
				break;
			case STATE_SEQUENCE:
				width = info->sequence->width;
				height = info->sequence->height;
				DP("Video size: width         = %d, height         = %d.\n", width, height);
				DP("          : picture_width = %d, picture_height = %d.\n", info->sequence->picture_width, info->sequence->picture_height);
				DP("          : display_width = %d, display_height = %d.\n", info->sequence->display_width, info->sequence->display_height);
				DP("          : pixel_width   = %d, pixel_height   = %d.\n", info->sequence->pixel_width, info->sequence->pixel_height);
				/*	picture_width, picture_height:  --> The actual video size in pixels.
					display_width, display_height:	--> The display values refer to the dimensions that the player should render the video at.
					pixel_width, pixel_height:		--> The pixel values refer to the actual number of samples stored per frame.

					aspect ratio = (picture_width * pixel_width) / (picture_height * pixel_height)

					The aspect_ratio_information may have the following values (with all others being reserved or forbidden):
					
					    0001  1:1
					    0010  4:3
					    0011  16:9
					    0100  2.21:1

					See also https://www.mir.com/DMG/aspect.html
				*/
				if (((info->sequence->picture_width * info->sequence->pixel_width) / (info->sequence->picture_height * info->sequence->pixel_height)) == (4/3))
					{
					seqHeaderExtInfo.aspectRatioInformation = 0x0010;
					DP("Aspect Ration = 4/3\n");
					}
				else if (((info->sequence->picture_width * info->sequence->pixel_width) / (info->sequence->picture_height * info->sequence->pixel_height)) == (16/9))
					{
					seqHeaderExtInfo.aspectRatioInformation = 0x0011;
					DP("Aspect Ration = 16/9\n");
					}
				else if (((info->sequence->picture_width * info->sequence->pixel_width) / (info->sequence->picture_height * info->sequence->pixel_height)) == (2.21/1))
					{
					seqHeaderExtInfo.aspectRatioInformation = 0x0100;
					DP("Aspect Ration = 2.21/1\n");
					}
				else if (((info->sequence->picture_width * info->sequence->pixel_width) / (info->sequence->picture_height * info->sequence->pixel_height)) == (1/1))
					{
					seqHeaderExtInfo.aspectRatioInformation = 0x0001;
					DP("Aspect Ration = 1/1\n");
					}
				else
					{
					seqHeaderExtInfo.aspectRatioInformation = 0x0010;
					DP("Aspect Ration = NOT VALID, force 4/3\n");
					}

				// The MPEG-2 system is driven by a 27 MHz clock.
				seqHeaderExtInfo.frameRateExtensionN = 27000000;
				seqHeaderExtInfo.frameRateExtensionD = info->sequence->frame_period;

				ysize = width * height;
				uvsize = info->sequence->chroma_height * info->sequence->chroma_width;

				// set up YV12 pixel array (12 bits per pixel)
				yPlaneSz = width * height;
				uvPlaneSz = width * height / 4;

				uvPitch = width / 2;
				// Store the relevant information
				seqHeaderExtInfo.horizontalSize = info->sequence->width;
				seqHeaderExtInfo.verticalSize = info->sequence->height;
				seqHeaderExtInfo.horizontalChromaSize = info->sequence->chroma_width;
				seqHeaderExtInfo.verticalChromaSize = info->sequence->chroma_height;

				mpeg2_custom_fbuf (decoder, 1);
				for (int i = 0; i < 3; i++)
					{
					// For every frame buffer, allocate memory for y, u and v
					fbuf[i].mbuf[0] = (uint8_t *) malloc(info->sequence->width*info->sequence->height + 15);
					fbuf[i].mbuf[1] = (uint8_t *) malloc(info->sequence->chroma_width*info->sequence->chroma_height + 15);
					fbuf[i].mbuf[2] = (uint8_t *) malloc(info->sequence->chroma_width*info->sequence->chroma_height + 15);
					for (int j = 0; j < 3; j++)
						{
						fbuf[i].yuv[j] = (uint8_t*) ALIGN_16(fbuf[i].mbuf[j]);
						}
					fbuf[i].used = 0;
					}
				for (int i = 0; i < 2; i++)
					{
					current_fbuf = get_fbuf ();
					mpeg2_set_buf (decoder, current_fbuf->yuv, current_fbuf);
					}
				mpeg2_skip (decoder, 0);
				break;
			case STATE_PICTURE:
				VirtualMPEGVideoDecoderUnit::current_fbuf = get_fbuf();
				mpeg2_set_buf (decoder, VirtualMPEGVideoDecoderUnit::current_fbuf->yuv,
					VirtualMPEGVideoDecoderUnit::current_fbuf);
				break;
			case STATE_END:
				// Intended fallthrough.
			case STATE_INVALID_END:
				// Intended fallthrough.
			case STATE_SLICE:
				if (info->display_fbuf)
					{
					// picture ready.
					//STFRES_REASSERT(DeliverData());
					res =  DeliverData();
					while (res != STFRES_OK)
						res =  DeliverData();
					}
				if (info->discard_fbuf)
					{
					((struct fbuf_s *)info->discard_fbuf->id)->used = 0;
					}
				break;
			default:
				break;
			}
		}
	STFRES_RAISE_OK;
	}


STFResult VirtualMPEGVideoDecoderUnit::DeliverData()
	{
	//
	// This method needs a state machine for re-entry purpose (retry after object full)
	//
	if (!(info->display_fbuf))
		STFRES_RAISE_OK;

	switch (deliverState)
		{
		case MPEGVIDEO_DELIVER_SEGMENT_START:
			// Deliver segment start
			if (framenum == 0)
				{
				STFRES_REASSERT(outputFormatter.BeginSegment(segmentCount, false));
				STFRES_REASSERT(outputFormatter.PutDataDiscontinuity());
				}
			if (dataPropertiesChanged & MPEG2_VIDEOTYPE_CHANGED)
				{
				STFRES_REASSERT(outputFormatter.PutTag(SET_MPEG_VIDEO_SEQUENCE_PARAMETERS(&seqHeaderExtInfo)));
				dataPropertiesChanged &= ~MPEG2_VIDEOTYPE_CHANGED;
				// Each call to PutTag() places one tag into the stream,
				// the call to CompleteTags() finally places the tag done
				STFRES_REASSERT(outputFormatter.CompleteTags());
				}
			deliverState = MPEGVIDEO_DELIVER_BEGIN_GROUP;

		case MPEGVIDEO_DELIVER_BEGIN_GROUP:
			// Deliver group start
			STFRES_REASSERT(outputFormatter.BeginGroup(framenum, false, true));
			deliverState = MPEGVIDEO_DELIVER_START_TIME;

		case MPEGVIDEO_DELIVER_START_TIME:
			// Deliver start time
			STFRES_REASSERT(outputFormatter.PutStartTime(presentationTime));
			startTime += STFHiPrec32BitDuration(40000, STFTU_MICROSECS); //PAL HACK
			deliverState = MPEGVIDEO_DELIVER_GET_MEMORYBLOCKS;

		case MPEGVIDEO_DELIVER_GET_MEMORYBLOCKS:
			// picture ready.
			// Get a memory block from the output formatter
			STFRES_REASSERT(outputPoolAllocator->GetMemoryBlocks(&memoryBlock, 0, 1, numObtainedBlocks, this));
			decodedPictureRange[rangeCounter%3].Init(memoryBlock, 0, memoryBlock->GetSize());
			// memcopy to framebuffer. Note that u and v are reversed to
			// get the correct color.
			// Copy the sample into the memoryblock
			memcpy (decodedPictureRange[rangeCounter%3].block->GetStart(), ((struct fbuf_s *)info->display_fbuf->id)->yuv[0], ysize);
			memcpy (decodedPictureRange[rangeCounter%3].block->GetStart() + ysize, ((struct fbuf_s *)info->display_fbuf->id)->yuv[1], uvsize);
			memcpy (decodedPictureRange[rangeCounter%3].block->GetStart() + ysize + uvsize, ((struct fbuf_s *)info->display_fbuf->id)->yuv[2], uvsize);
			deliverState = MPEGVIDEO_DELIVER_RANGE;

		case MPEGVIDEO_DELIVER_RANGE:
			STFRES_REASSERT(outputFormatter.PutRange(decodedPictureRange[rangeCounter%3]));
			deliverState = MPEGVIDEO_DELIVER_END_TIME;

		case MPEGVIDEO_DELIVER_END_TIME:
			// Deliver end time
			STFRES_REASSERT(outputFormatter.PutEndTime(presentationTime));
			deliverState = MPEGVIDEO_DELIVER_GROUP_END;

		case MPEGVIDEO_DELIVER_GROUP_END:
			// Deliver group end
			if (groupEndPending)
				{
				STFRES_REASSERT(outputFormatter.CompleteGroup(groupEndNotification));
				groupEndPending = false;
				}
			framenum++;
			rangeCounter++;
			outputFormatter.Commit(); // Don't wait, just push to the output
			deliverState = MPEGVIDEO_DELIVER_BEGIN_GROUP;

		default:
			break;
		}

	STFRES_RAISE_OK;
	}


//////////////////////////////////////////////////////////////////////////

STFResult VirtualMPEGVideoDecoderUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
//	if (!preparing)
//		STFRES_REASSERT(DeliverData());// First check if pending data needs to be send.
	while (range < num)
		{
		STFRES_REASSERT(this->DecodeData(ranges[range], offset));
		range++;
		}
	STFRES_RAISE_OK;
	}


STFResult VirtualMPEGVideoDecoderUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	DP_STREAMCMD(this, "BeginStreamingCommand", command);
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			dataPropertiesChanged = MPEG2_VIDEOTYPE_CHANGED;
			groupNumber = 0;
			segmentCount = 0;
			framenum = 0;
			preparing = true;
			deliverState = MPEGVIDEO_DELIVER_SEGMENT_START;
			break;
		case VDR_STRMCMD_DO:
			break;
		case VDR_STRMCMD_FLUSH:
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;

		default:
			DP("*** Unhandled STRMCMD in VirtualMPEGVideoDecoderUnit::BeginStreamingCommand! ***\n");
		}

	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualThreadedStandardInOutStreamingUnitCollection::BeginStreamingCommand(command, param));
	}


STFResult VirtualMPEGVideoDecoderUnit::PreemptUnit(uint32 flags)
	{
	STFRES_REASSERT(VirtualThreadedStandardInOutStreamingUnitCollection::PreemptUnit (flags));

	if (flags & (VDRUALF_PREEMPT_START_NEW | VDRUALF_PREEMPT_RESTART_PREVIOUS))
		{
		decoder = mpeg2_init ();
		if (decoder == NULL)
			{
			DP ("Could not allocate a decoder object.\n");
			assert(0);
			}
		info = mpeg2_info (decoder);

		ResetThreadSignal();
		STFRES_REASSERT(StartThread());
		}

	if (flags & (VDRUALF_PREEMPT_STOP_PREVIOUS | VDRUALF_PREEMPT_STOP_NEW))
		{
		//cleanup
		mpeg2_close (decoder);
		for (int i = 0; i < 3; i++)
			{
			free (fbuf[i].mbuf[0]);
			free (fbuf[i].mbuf[1]);
			free (fbuf[i].mbuf[2]);
			}

		StopThread();
		Wait();
		}

	if (flags & (VDRUALF_PREEMPT_CHANGE | VDRUALF_PREEMPT_RESTORE))
		{
		// not sure about this yet
		}

	STFRES_RAISE_OK;
	}


STFResult VirtualMPEGVideoDecoderUnit::Initialize(void)
	{
	STFRES_REASSERT(VirtualThreadedStandardInOutStreamingUnitCollection::Initialize());
	STFRES_REASSERT(SetThreadPriority((STFThreadPriority) physicalMPEGVideoDecoder->threadPriority));
	STFRES_REASSERT(SetThreadStackSize(physicalMPEGVideoDecoder->threadStackSize));
	STFRES_REASSERT(SetThreadName(physicalMPEGVideoDecoder->threadName));
	STFRES_RAISE_OK;
	}

