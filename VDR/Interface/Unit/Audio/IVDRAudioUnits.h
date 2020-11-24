#ifndef IVDRAUDIOUNITS_H
#define IVDRAUDIOUNITS_H

///
/// @brief      VDR Basic Audio Types and Tags
///

#include "VDR/Interface/Unit/IVDRGlobalUnitIDs.h"

//
// Error codes from Audio Units
//

/// An audio unit detected an audio stream type which it cannot handle
static const STFResult STFRES_UNSUPPORTED_AUDIO_STREAM_TYPE = 0x88844023;


//
// Public Unit IDs
//

/////////////////////////////////////////////////////////////////////////////

/// Elementary Audio Stream Player Streaming Proxy Unit
static const VDRUID VDRUID_AUDIO_PLAYER_PROXY = 0x00000012;


/////////////////////////////////////////////////////////////////////////////

/// PCM Audio Stream Mixer
static const VDRUID VDRUID_AUDIO_PCM_STREAM_MIXER = 0x0000000f;

/// Mixer Input Control for the Main Audio Input to the PCM Audio Mixer
static const VDRUID VDRUID_AUDIO_MIXER_INPUT_CONTROL_MAIN = 0x0000001d;

/// Mixer Input Control for the Auxiliary Audio Input to the PCM Audio Mixer
static const VDRUID VDRUID_AUDIO_MIXER_INPUT_CONTROL_AUX = 0x00000011;


/////////////////////////////////////////////////////////////////////////////

/// SPDIF Stream Mixer
static const VDRUID VDRUID_SPDIF_STREAM_MIXER = 0x0000001c;

/// Mixer Input Control for the Main  Input to the SPDIF Audio Mixer
static const VDRUID VDRUID_SPDIF_MIXER_INPUT_CONTROL_MAIN = 0x0000001e;


/////////////////////////////////////////////////////////////////////////////

/// Main Combined (PCM + SPDIF) Audio Renderer
static const VDRUID VDRUID_MAIN_AUDIO_RENDERER_PROXY = 0x0000004c;


/////////////////////////////////////////////////////////////////////////////

/// Main PCM Reader Streaming Proxy Unit
static const VDRUID VDRUID_MAIN_PCM_READER_PROXY = 0x00000019;


/////////////////////////////////////////////////////////////////////////////

/// Main PCM Renderer Streaming Proxy Unit
static const VDRUID VDRUID_MAIN_PCM_RENDERER_PROXY = 0x00000014;


/////////////////////////////////////////////////////////////////////////////

/// Main SPDIF Renderer Streaming Proxy Unit
static const VDRUID VDRUID_MAIN_SPDIF_RENDERER_PROXY = 0x0000001f;


/// Standard SPDIF Player Proxy Unit 
static const VDRUID VDRUID_STANDARD_SPDIF_PLAYER_PROXY = 0x0000004b;


/////////////////////////////////////////////////////////////////////////////

/// Generic Audio Synthesizer
static const VDRUID VDRUID_AUDIO_SYNTHESIZER = 0x00000010;


/// PCM Post Proc Proxy
static const VDRUID VDRUID_AUDIO_PCM_POST_PROCESSOR_CONTROL = 0x00000029;


//Jk Added for testing
static const VDRUID VDRUID_LOOPBACK_PROXY				= 0x00000156;

/// Chandan - MP3 encoder Proxy unit
static const VDRUID VDRUID_MP3_ENCODER_PROXY = 0x00000099;

/// Chandan - MP3 encoder- PCM Player unit
static const VDRUID VDRUID_PROVIDE_DATA_PROXY = 0x00010013;


#endif // #ifndef IVDRAUDIOUNITS_H
