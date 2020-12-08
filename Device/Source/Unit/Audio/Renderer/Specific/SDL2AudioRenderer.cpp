#include "SDL2AudioRenderer.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

///////////////////////////////////////////////////////////////////////////////
/// Physical Unit
///////////////////////////////////////////////////////////////////////////////
UNIT_CREATION_FUNCTION(CreateSDL2AudioRenderer, SDL2AudioRendererUnit)


STFResult SDL2AudioRendererUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
	{
	unit = (IVirtualUnit*)(new VirtualSDL2AudioRendererUnit(this));
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
STFResult VirtualSDL2AudioRendererUnit::Render(const VDRDataRange & range, uint32 & offset)
	{
	int flags = 0;
	int16_t int16_samples[256*6];
	ssize_t r;

	if (Preparing)
		{
		ConfigureRenderer();
		}

	STFRES_RAISE_OK; // Do NOT CHECK IN

	flags &= A52_CHANNEL_MASK | A52_LFE;

	float2s16_multi ((float*)range.GetStart() + offset, int16_samples, flags);

	r = 256 * sizeof (int16_t) * chans;

	if (SDL_QueueAudio(dev, int16_samples, (uint32) r) == 0)
		{
		if (Preparing)
			SDL_LockAudioDevice(dev);
		STFRES_RAISE_OK;
		}
	else
		{
		DP("Audio error = %s \n", SDL_GetError());
		return STFRES_OBJECT_FULL;
		}
	}

STFResult VirtualSDL2AudioRendererUnit::channels_multi ()
	{
	switch (codingMode)
		{
		case VDR_ACMOD_1_0:
			chans = 1;
			break;
		case VDR_ACMOD_DUALMONO:
		case VDR_ACMOD_2_0:
			chans = 2;
			break;
		case VDR_ACMOD_3_0:
		case VDR_ACMOD_2_1:
			chans = 3;
			break;
		case VDR_ACMOD_3_1:
		case VDR_ACMOD_2_2:
			chans = 4;
			break;
		case VDR_ACMOD_3_2:
		case VDR_ACMOD_2_3:
			chans = 5;
			break;
		case VDR_ACMOD_3_3:
		case VDR_ACMOD_2_4:
			chans = 6;
			break;
		case VDR_ACMOD_3_4:
			chans = 7;
			break;
		}
	chans = 2; // For now
	STFRES_RAISE_OK;
}

STFResult VirtualSDL2AudioRendererUnit::ConfigureRenderer()
	{
	if (dev == 0)
		{
		SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
		want.freq = sampleRate;
		want.format = AUDIO_S16;
		want.channels = 2;
		want.samples = 4096;
		want.callback = NULL; // MyAudioCallback; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

		dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

		inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);

		SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
		}
	STFRES_RAISE_OK;
	}

///
/// Parent class overrides
///
STFResult VirtualSDL2AudioRendererUnit::ParseRange(const VDRDataRange & range, uint32 & offset)
	{
	STFRES_RAISE(Render(range, offset));
	}


STFResult VirtualSDL2AudioRendererUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
	{
	switch (command)
		{
		case VDR_STRMCMD_BEGIN:
			dev = 0;
			Preparing = true;
			//inputConnector->SendUpstreamNotification(VDRMID_STRM_START_POSSIBLE, 0, 0);
			DP("Audio Preparing\n");
			break;
		case VDR_STRMCMD_DO:
			Preparing = false;
			SDL_UnlockAudioDevice(dev);
			DP("Audio started\n");
			break;
		case VDR_STRMCMD_FLUSH:
			/* Make sure that every single sample was played */
			break;
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;
		default:
			DP("*** Unhandled STRMCMD in VirtualSDL2AudioRendererUnit::BeginStreamingCommand! ***\n");
		}
	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::BeginStreamingCommand(command, param));
	}


STFResult VirtualSDL2AudioRendererUnit::ParseConfigure(TAG *& tags)
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

