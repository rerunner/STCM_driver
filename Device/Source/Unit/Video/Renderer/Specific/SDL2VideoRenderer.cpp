#include "SDL2VideoRenderer.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

///////////////////////////////////////////////////////////////////////////////
/// Physical Unit
///////////////////////////////////////////////////////////////////////////////
UNIT_CREATION_FUNCTION(CreateSDL2VideoRenderer, SDL2VideoRendererUnit)


STFResult SDL2VideoRendererUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualSDL2VideoRendererUnit(this));
	if (unit)
		{
		STFRES_REASSERT(unit->Connect(parent, root));
		}
	else
		STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
	STFRES_RAISE_OK;
	}


///////////////////////////////////////////////////////////////////////////////
/// Virtual Unit
///////////////////////////////////////////////////////////////////////////////
STFResult VirtualSDL2VideoRendererUnit::Render(const VDRDataRange & range, uint32 & offset)
	{
	uint8 *buffer = range.GetStart() + offset;

	if (Preparing)
	{
		ConfigureRenderer();
		Preparing = false;
	}

	int ysize = seqHeaderExtInfo->horizontalSize * seqHeaderExtInfo->verticalSize;
	int uvsize = seqHeaderExtInfo->horizontalChromaSize * seqHeaderExtInfo->verticalChromaSize;
	int uvPitch = seqHeaderExtInfo->horizontalSize / 2;
	uint8 *yPlane = buffer; // size = ySize
	uint8 *vPlane = buffer + ysize; // size = uvsize
	uint8 *uPlane = vPlane + uvsize; // size = uvsize

	SDL_UpdateYUVTexture(
		texture,
		NULL,
		yPlane,
		seqHeaderExtInfo->horizontalSize,
		uPlane,
		uvPitch,
		vPlane,
		uvPitch);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	usleep(20 * 1000); // REMOVE THIS WHEN PRESENTATION TIME IS MATCHED WITH SYSTEM TIME
	STFRES_RAISE_OK;
	}


STFResult VirtualSDL2VideoRendererUnit::ConfigureRenderer()
	{
	//Open SDL2 video device
	// Make a screen to put our video
	DP("Video Renderer creating display, width = %d, height = %d.\n", seqHeaderExtInfo->horizontalSize,seqHeaderExtInfo->verticalSize);
	screen = SDL_CreateWindow(	"MPEG2 Video",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								seqHeaderExtInfo->horizontalSize,
								seqHeaderExtInfo->verticalSize,
								0);

	renderer = SDL_CreateRenderer(screen, -1, 0);
	if (!renderer)
		{
		DP("SDL: could not create renderer - exiting\n");
		assert(0);
		}

	// Allocate a place to put our YUV image on that screen
	texture = SDL_CreateTexture(	renderer,
									SDL_PIXELFORMAT_YV12,
									SDL_TEXTUREACCESS_STREAMING,
									seqHeaderExtInfo->horizontalSize,
									seqHeaderExtInfo->verticalSize);
	if (!texture)
		{
		DP("SDL: could not create texture - exiting\n");
		assert(0);
		}
	inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
	STFRES_RAISE_OK;
	}

///
/// Parent class overrides
///
STFResult VirtualSDL2VideoRendererUnit::ParseRange(const VDRDataRange & range, uint32 & offset)
	{
	STFRES_RAISE(Render(range, offset));
	}


STFResult VirtualSDL2VideoRendererUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			Preparing = true;
// SDL2 Init, this needs a better place...
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
				{
				DP("SDL2 global init failed.\n");
				}
			else
				{
				DP("SDL2 global init OK.\n");
				}
			break;
		case VDR_STRMCMD_DO:
			break;
		case VDR_STRMCMD_FLUSH:
			break;
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;
		default:
			DP("*** Unhandled STRMCMD in VirtualSDL2VideoRendererUnit::BeginStreamingCommand! ***\n");
		}
	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::BeginStreamingCommand(command, param));
	}


STFResult VirtualSDL2VideoRendererUnit::ParseConfigure(TAG *& tags)
	{
	TAG *& tp = tags;

	while (tp->id)
		{
		switch (tp->id)
			{
			case CSET_MPEG_VIDEO_SEQUENCE_PARAMETERS:
				seqHeaderExtInfo = VAL_MPEG_VIDEO_SEQUENCE_PARAMETERS(tp);
				DP("MPEG_VIDEO_SEQUENCE_PARAMETERS received\n");
				break;
			default:
				DP("Unknown tags!\n");
				break;
			}
		tp += tp->skip;
		}
	STFRES_RAISE_OK;
	}


