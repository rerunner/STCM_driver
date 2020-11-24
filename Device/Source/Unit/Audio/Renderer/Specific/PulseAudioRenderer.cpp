#include "PulseAudioRenderer.h"
#include "STF/Interface/STFTimer.h"
#include "STF/Interface/STFDebug.h"
#include "VDR/Source/Construction/IUnitConstruction.h"

///////////////////////////////////////////////////////////////////////////////
/// Physical Unit
///////////////////////////////////////////////////////////////////////////////
UNIT_CREATION_FUNCTION(CreatePulseAudioRenderer, PulseAudioRendererUnit)


STFResult PulseAudioRendererUnit::CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent, IVirtualUnit * root)
{
	unit = (IVirtualUnit*)(new VirtualPulseAudioRendererUnit(this));
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
STFResult VirtualPulseAudioRendererUnit::Render(const VDRDataRange & range, uint32 & offset)
{
	int flags = 0;
	int16_t int16_samples[256*6];
	int error;
	ssize_t r;

	if (Preparing)
	{
		ConfigureRenderer();
		Preparing = false;
	}
	//flags |= A52_ADJUST_LEVEL;
	//flags |= A52_STEREO;

	flags &= A52_CHANNEL_MASK | A52_LFE;

	float2s16_multi ((float*)range.GetStart() + offset, int16_samples, flags);

	r = 256 * sizeof (int16_t) * chans;
	if (pa_simple_write(s, int16_samples, (size_t) r, &error) < 0) 
	{
		DP("pa_simple_write() failed: %s\n", pa_strerror(error));
	}
	STFRES_RAISE_OK;
}

STFResult VirtualPulseAudioRendererUnit::channels_multi ()
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

STFResult VirtualPulseAudioRendererUnit::ConfigureRenderer()
{
	int error;
	// Pulse Audio start
	/* The Sample format to use */
	static const pa_sample_spec ss = {
		.format = PA_SAMPLE_S16LE,
		.rate = sampleRate,
		.channels = 2
	};
	s = NULL;
	/* Create a new playback stream */
	if (!(s = pa_simple_new(NULL, "AC3", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) 
	{
		DP("pa_simple_new() failed: %s\n", pa_strerror(error));
	}
	STFRES_RAISE_OK;
}

///
/// Parent class overrides
///
STFResult VirtualPulseAudioRendererUnit::ParseRange(const VDRDataRange & range, uint32 & offset)
{
	STFRES_RAISE(Render(range, offset));
}


STFResult VirtualPulseAudioRendererUnit::BeginStreamingCommand(VDRStreamingCommand command, int32 param)
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
			if (pa_simple_drain(s, &error) < 0) 
			{
				DP("pa_simple_drain() failed: %s\n", pa_strerror(error));
			}
			if (s)
				pa_simple_free(s);
			s = NULL;
			break;
		case VDR_STRMCMD_STEP:
		case VDR_STRMCMD_NONE:
			break;
		default:
			DP("*** Unhandled STRMCMD in VirtualPulseAudioRendererUnit::BeginStreamingCommand! ***\n");
	}
	// Now call our parent class to complete the command handling
	STFRES_RAISE(VirtualNonthreadedStandardStreamingUnit::BeginStreamingCommand(command, param));
}


STFResult VirtualPulseAudioRendererUnit::ParseConfigure(TAG *& tags)
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

