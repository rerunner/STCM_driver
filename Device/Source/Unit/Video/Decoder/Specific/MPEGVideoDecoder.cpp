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
														  16,	// Number of output packets in output connector,
														  8,	// Input connector queue size,
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


STFResult VirtualMPEGVideoDecoderUnit::DecodeData(const VDRDataRange & range, uint32 & offset)
	{
	uint8_t * buffer = range.GetStart() + offset;
	uint8_t * end = buffer + range.size - offset;

	mpeg2_buffer (decoder, buffer, end);

	while (1)
		{
		state = mpeg2_parse (decoder);
		switch (state)
			{
			case STATE_BUFFER:
				STFRES_RAISE_OK;
				break;
			case STATE_SEQUENCE:
				width = info->sequence->width;
				height = info->sequence->height;
				DP("Video size: width = %d, height = %d.\n", width, height);
				ysize = width * height;
				uvsize = info->sequence->chroma_height * info->sequence->chroma_width;

				// set up YV12 pixel array (12 bits per pixel)
				yPlaneSz = width * height;
				uvPlaneSz = width * height / 4;
				yPlane = (Uint8*)malloc(yPlaneSz);
				uPlane = (Uint8*)malloc(uvPlaneSz);
				vPlane = (Uint8*)malloc(uvPlaneSz);
				uvPitch = width / 2;

				mpeg2_custom_fbuf (decoder, 1);
				for (int i = 0; i < 3; i++)
					{
					/* For every frame buffer, allocate memory for y, u and v */
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
					// picture ready. memcopy to framebuffer. Note that u and v are reversed to get the correct color.

					// Copy the Y (Luma)
					memcpy (yPlane, ((struct fbuf_s *)info->display_fbuf->id)->yuv[0], ysize);

					// Copy the U (chroma)
					memcpy (uPlane, ((struct fbuf_s *)info->display_fbuf->id)->yuv[2], uvsize);

					// Copy the V (chroma)
					memcpy (vPlane, ((struct fbuf_s *)info->display_fbuf->id)->yuv[1], uvsize);

					SDL_UpdateYUVTexture(
						texture,
						NULL,
						yPlane,
						720,
						uPlane,
						uvPitch,
						vPlane,
						uvPitch);

					SDL_RenderClear(renderer);
					SDL_RenderCopy(renderer, texture, NULL, NULL);
					SDL_RenderPresent(renderer);
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

//////////////////////////////////////////////////////////////////////////

STFResult VirtualMPEGVideoDecoderUnit::ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset)
	{
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
    		framenum = 0;
			decoder = mpeg2_init ();
			if (decoder == NULL)
				{
				DP ("Could not allocate a decoder object.\n");
				assert(0);
				}
			info = mpeg2_info (decoder);

			// Make a screen to put our video
			screen = SDL_CreateWindow(
				"MPEG2 Video",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				720,
				576,
				0);

			renderer = SDL_CreateRenderer(screen, -1, 0);
			if (!renderer)
				{
				DP("SDL: could not create renderer - exiting\n");
				assert(0);
				}

			// Allocate a place to put our YUV image on that screen
			texture = SDL_CreateTexture(
				renderer,
				SDL_PIXELFORMAT_YV12,
				SDL_TEXTUREACCESS_STREAMING,
				720,
				576);
			if (!texture)
				{
				DP("SDL: could not create texture - exiting\n");
				assert(0);
				}
			// Immediately fake the signal that enough data was received to start
			inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
			break;

		case VDR_STRMCMD_DO:
			break;
		case VDR_STRMCMD_FLUSH:
			mpeg2_close (decoder);
			for (int i = 0; i < 3; i++)
			  {
			    free (fbuf[i].mbuf[0]);
			    free (fbuf[i].mbuf[1]);
			    free (fbuf[i].mbuf[2]);
			  }
			SDL_DestroyTexture(texture);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(screen);
			free(yPlane);
			free(uPlane);
			free(vPlane);
			break;
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;

		default:
			DP("*** Unhandled STRMCMD in VirtualLinuxMPEGVideoDecoderUnit::BeginStreamingCommand! ***\n");
		}

	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualThreadedStandardInOutStreamingUnitCollection::BeginStreamingCommand(command, param));
	}


STFResult VirtualMPEGVideoDecoderUnit::PreemptUnit(uint32 flags)
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


STFResult VirtualMPEGVideoDecoderUnit::Initialize(void)
	{
	STFRES_REASSERT(VirtualThreadedStandardInOutStreamingUnitCollection::Initialize());
	STFRES_REASSERT(SetThreadPriority((STFThreadPriority) physicalMPEGVideoDecoder->threadPriority));
	STFRES_REASSERT(SetThreadStackSize(physicalMPEGVideoDecoder->threadStackSize));
	STFRES_REASSERT(SetThreadName(physicalMPEGVideoDecoder->threadName));
	STFRES_RAISE_OK;
	}

