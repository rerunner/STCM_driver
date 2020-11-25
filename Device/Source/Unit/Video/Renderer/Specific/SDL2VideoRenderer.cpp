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
	STFRES_RAISE_OK;
	}


STFResult VirtualSDL2VideoRendererUnit::ConfigureRenderer()
	{
	//Open SDL2 video device
	// Make a screen to put our video
	screen = SDL_CreateWindow(	"MPEG2 Video",
								SDL_WINDOWPOS_UNDEFINED,
								SDL_WINDOWPOS_UNDEFINED,
								WindowProperties.width,
								WindowProperties.height,
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
									ScreenProperties.width,
									ScreenProperties.height);
	if (!texture)
		{
		DP("SDL: could not create texture - exiting\n");
		assert(0);
		}
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
	int error;
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			Preparing = true;
			// Immediately fake the signal that enough data was received to start
			inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
			break;
		case VDR_STRMCMD_DO:
			break;
		case VDR_STRMCMD_FLUSH:
			/* Make sure that every single sample was played */
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
			case CSET_AUDIO_STREAM_AUDIO_CODING_MODE:
				codingMode = VAL_AUDIO_STREAM_AUDIO_CODING_MODE(tp);
				DP("AUDIO_STREAM_AUDIO_CODING_MODE = %d\n", codingMode);
				channels_multi (); // Convert codingMode to pulse audio channels
				break;
			case CSET_AUDIO_STREAM_SAMPLE_RATE:
				sampleRate = VAL_AUDIO_STREAM_SAMPLE_RATE(tp);
				DP("AUDIO_STREAM_SAMPLE_RATE = %d\n", sampleRate);
				break;
			default:
				DP("Unknown tags!\n");
				break;
			}
		tp += tp->skip;
		}
	STFRES_RAISE_OK;
	}


