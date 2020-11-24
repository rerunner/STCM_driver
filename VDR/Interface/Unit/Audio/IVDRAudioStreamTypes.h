#ifndef IVDRAUDIOSTREAMTYPES_H
#define IVDRAUDIOSTREAMTYPES_H

///
/// @brief VDR Audio Stream Types and Tags
///

#include "VDR/Interface/Streaming/IVDRStreamTypes.h"

#ifndef ST20LITE
#pragma warning(disable:4800)
#endif

/////////////////////////////////////////////////////////////////////////////
//	Audio Stream Tags and their Types
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_STREAM_PROPERTY = 0x0001f000;


enum VDRAudioStreamType
	{
	VDR_AUDIOTYPE_UNDEFINED,		// No audio type
	VDR_AUDIOTYPE_ANONYMOUS,		// Anonymous audio stream (must be ignored by all processing - used for pass-through)
	VDR_AUDIOTYPE_MPEG,				// All forms of MPEG audio (1/2/3/4)
	VDR_AUDIOTYPE_AC3,
	VDR_AUDIOTYPE_LPCM,				// includes CDDA (set # of bits and sample rate accordingly)
	VDR_AUDIOTYPE_DTS,
	VDR_AUDIOTYPE_SDDS,
	VDR_AUDIOTYPE_MLP,
	VDR_AUDIOTYPE_LPCM_DVDA,		// DVD Audio with LPCM
	VDR_AUDIOTYPE_CDDA,				// CDDA without Subchannels
	VDR_AUDIOTYPE_CDDA_SUB,			// CDDA with Subchannels
	VDR_AUDIOTYPE_CDDA_DTS,			// CDDA with DTS information
	VDR_AUDIOTYPE_CDDA_DTS_SUB,	// CDDA with DTS information and subchannel info
	VDR_AUDIOTYPE_WMA,
	VDR_AUDIOTYPE_AAC,
	VDR_AUDIOTYPE_OGG_VORBIS,
	VDR_AUDIOTYPE_REAL_AUDIO,
	VDR_AUDIOTYPE_RAW_PCM,			// Raw PCM (continuous PCM stream)
	
	VDR_AUDIOTYPE_LAST
	};

// Audio Type of stream
MKTAG (AUDIO_STREAM_TYPE,	VDRTID_AUDIO_STREAM_PROPERTY, 0x001, VDRAudioStreamType)


/////////////////////////////////////////////////////////////////////////////


//
// The following Tags are mainly used to describe the properties of PCM streams.
// However, some of them may also be applicable to streams of other type (this is legal to do).
//

// Audio Coding Mode (Assigns channels to specific speakers)

enum VDRAudioCodingMode
	{
	VDR_ACMOD_DUALMONO,
	VDR_ACMOD_1_0,
	VDR_ACMOD_2_0,
	VDR_ACMOD_3_0,
	VDR_ACMOD_2_1,
	VDR_ACMOD_3_1,
	VDR_ACMOD_2_2,
	VDR_ACMOD_3_2,
	VDR_ACMOD_2_3,
	VDR_ACMOD_3_3,
	VDR_ACMOD_2_4,
	VDR_ACMOD_3_4
	// Add coding modes for newer versions of Dolby Digital and other types here...
	};

MKTAG (AUDIO_STREAM_AUDIO_CODING_MODE,	VDRTID_AUDIO_STREAM_PROPERTY, 0x002, VDRAudioCodingMode)


/////////////////////////////////////////////////////////////////////////////

// Service Mode

// Explains purpose of stream from a service point of view

enum VDRAudioServiceMode
	{
	VDR_AUDIO_SERVICE_COMPLETE_MAIN,
	VDR_AUDIO_SERVICE_MUSIC_AND_EFFECTS,
	VDR_AUDIO_SERVICE_VISUALLY_IMPAIRED,
	VDR_AUDIO_SERVICE_HEARING_IMPAIRED,
	VDR_AUDIO_SERVICE_DIALOGUE,
	VDR_AUDIO_SERVICE_COMMENTARY,
	VDR_AUDIO_SERVICE_EMERGENCY,
	VDR_AUDIO_SERVICE_VOICE_OVER,
	VDR_AUDIO_SERVICE_KARAOKE
	};

MKTAG (AUDIO_STREAM_SERVICE_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x003, VDRAudioServiceMode)


/////////////////////////////////////////////////////////////////////////////

// Sample Rate	of stream
MKTAG (AUDIO_STREAM_SAMPLE_RATE,	VDRTID_AUDIO_STREAM_PROPERTY, 0x004, uint32)


/////////////////////////////////////////////////////////////////////////////

// Bits per sample
MKTAG (AUDIO_STREAM_BITS_PER_SAMPLE, VDRTID_AUDIO_STREAM_PROPERTY, 0x005, uint16)


/////////////////////////////////////////////////////////////////////////////

// Number of channels
MKTAG (AUDIO_STREAM_NUMBER_OF_CHANNELS, VDRTID_AUDIO_STREAM_PROPERTY, 0x006, uint16)


/////////////////////////////////////////////////////////////////////////////

// Emphasis

enum VDRAudioEmphasis
	{
	VDR_AUDIO_EMPHASIS_NONE,
	VDR_AUDIO_EMPHASIS_50_15_MS,
	VDR_AUDIO_EMPHASIS_RESERVED,
	VDR_AUDIO_EMPHASIS_CCIT_J17
	};

MKTAG (AUDIO_STREAM_EMPHASIS,	VDRTID_AUDIO_STREAM_PROPERTY, 0x007, VDRAudioEmphasis)


/////////////////////////////////////////////////////////////////////////////

// Mute info

enum VDRAudioMute
	{
	VDR_AUDIO_MUTE_OFF,
	VDR_AUDIO_MUTE_ON			// Indicates stream content must be muted at the output
	};

MKTAG (AUDIO_STREAM_MUTE, VDRTID_AUDIO_STREAM_PROPERTY, 0x008, VDRAudioMute)


/////////////////////////////////////////////////////////////////////////////

// Dynamic Range Control

enum VDRAudioDynamicRangeControl
	{
	VDR_DYNRG_COMPRESSED,	// Dynamic range sclae factor taken from MPEG-2 AC3 stream
	VDR_DYNRG_MAXIMUM,		// Always use full dynamic range
	VDR_DYNRG_ARBITRARY		// Set arbitrary value for dynamic range using specific TAGs
	};

MKTAG (AUDIO_STREAM_DYNAMIC_RANGE_CONTROL,	VDRTID_AUDIO_STREAM_PROPERTY, 0x00a, VDRAudioDynamicRangeControl)

// High Dynamic Range (0 - 10000)
MKTAG (AUDIO_STREAM_HIGH_DYNAMIC_RANGE,		VDRTID_AUDIO_STREAM_PROPERTY, 0x00b, uint16)

// Low Dynamic Range (0 - 10000)
MKTAG (AUDIO_STREAM_LOW_DYNAMIC_RANGE,			VDRTID_AUDIO_STREAM_PROPERTY, 0x00c, uint16)



/////////////////////////////////////////////////////////////////////////////

// Compression Control

// ??? Not clear yet.

MKTAG (AUDIO_STREAM_COMPRESSION_CONTROL, VDRTID_AUDIO_STREAM_PROPERTY, 0x00d, uint32)


/////////////////////////////////////////////////////////////////////////////

// Frame duration of stream in 108MHz clock settings

MKTAG (AUDIO_STREAM_FRAME_DURATION,	VDRTID_AUDIO_STREAM_PROPERTY, 0x00e, uint32)

/////////////////////////////////////////////////////////////////////////////

// Dolby Surround Mode

enum VDRDolbySurroundMode
	{
	VDR_DSURMOD_NOT_INDICATED,
	VDR_DSURMOD_NOT_DOLBY_SURROUND_ENCODED,
	VDR_DSURMOD_DOLBY_SURROUND_ENCODED
	};

MKTAG (AUDIO_STREAM_DOLBY_SURROUND_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x010, VDRDolbySurroundMode)


/////////////////////////////////////////////////////////////////////////////

enum VDRAudioLFEInfo
	{
	VDR_LFE_NOT_PRESENT,
	VDR_LFE_PRESENT
	};

// This Stream Tag is meant to indicate to a Down Stream post-processing unit
// whether a LFE channel is present in the decoded audio or not.  That's why 
// it is called AUDIO_LFE_INFO :)

MKTAG (AUDIO_LFE_INFO, VDRTID_AUDIO_STREAM_PROPERTY, 0x011, VDRAudioLFEInfo)


/////////////////////////////////////////////////////////////////////////////

// Room Type

enum VDRAudioRoomType
	{
	VDR_ROOMTYPE_UNKNOWN,
	VDR_ROOMTYPE_LARGE,
	VDR_ROOMTYPE_SMALL
	};

MKTAG (AUDIO_STREAM_ROOM_TYPE, VDRTID_AUDIO_STREAM_PROPERTY, 0x012, VDRAudioRoomType)


/////////////////////////////////////////////////////////////////////////////

// Multilingual Info

enum VDRAudioMultilingualInfo
	{
	VDR_MULTI_LINGUAL_FS_SAME,	// Multilingual channels have same sampling frequency as main channels
	VDR_MULTI_LINGUAL_FS_HALF	// Multilingual channels have half of main channels' sampling frequency
	};

MKTAG (AUDIO_STREAM_MULTILINGUAL_INFO, VDRTID_AUDIO_STREAM_PROPERTY, 0x013, VDRAudioMultilingualInfo)


/////////////////////////////////////////////////////////////////////////////

// Number of Multilingual channels

MKTAG (AUDIO_STREAM_MULTILINGUAL_CHANNELS, VDRTID_AUDIO_STREAM_PROPERTY, 0x014, uint16)




//
// Specific Tags of AC3 streams (provided if needed by a processing Streaming unit after decoding)
//

/////////////////////////////////////////////////////////////////////////////

// Mix Level
MKTAG (AUDIO_STREAM_MIX_LEVEL, VDRTID_AUDIO_STREAM_PROPERTY, 0x080, uint32)


/////////////////////////////////////////////////////////////////////////////

// Center Mix Level
MKTAG (AUDIO_STREAM_CENTER_MIX_LEVEL, VDRTID_AUDIO_STREAM_PROPERTY, 0x081, uint32)


/////////////////////////////////////////////////////////////////////////////

// Surround Mix Level
MKTAG (AUDIO_STREAM_SURROUND_MIX_LEVEL, VDRTID_AUDIO_STREAM_PROPERTY, 0x082, uint32)

// Copy Right Protected
MKTAG (AUDIO_COPYRIGHT_PROTECTED, VDRTID_AUDIO_STREAM_PROPERTY, 0x083, bool)

// Original Bit Stream
MKTAG (AUDIO_ORIGINAL_BIT_STREAM, VDRTID_AUDIO_STREAM_PROPERTY, 0x084, bool)


//
// Specific Tags of MPEG streams (provided if needed by a processing Streaming unit after decoding)
//

/////////////////////////////////////////////////////////////////////////////

// According to ISO/IEC 11172-3

enum VDRAudioMode
	{
	VDR_AUDIO_MODE_STEREO,			// ISO: 0 (%00)
	VDR_AUDIO_MODE_JOINT_STEREO,	// ISO: 1 (%01)
	VDR_AUDIO_MODE_DUAL_CHANNEL,	// ISO: 2 (%10)
	VDR_AUDIO_MODE_SINGLE_CHANNEL	// ISO: 3 (%11)
	};


/////////////////////////////////////////////////////////////////////////////

// MPEG Mode

MKTAG (AUDIO_STREAM_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0a0, VDRAudioMode)


/////////////////////////////////////////////////////////////////////////////

// MPEG Mode Extension (refer to ISO/IEC 11172-3 for more info on possible values)

MKTAG (AUDIO_STREAM_MODE_EXTENSION,	VDRTID_AUDIO_STREAM_PROPERTY, 0x0a1, uint16)


/////////////////////////////////////////////////////////////////////////////

// MPEG Dematrix Procedure (refer to ISO/IEC 11172-3 for more info on possible values)

MKTAG (AUDIO_STREAM_DEMATRIX_PROCEDURE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0a2, uint16)

// Bits per sample in Group 2 (DVD Audio only)

MKTAG (AUDIO_STREAM_BITS_PER_SAMPLE_GROUP2, VDRTID_AUDIO_STREAM_PROPERTY, 0x0A3, uint16)

// MPEG Multichannel Extension mode

MKTAG (AUDIO_STREAM_MPEG_EXTENSION_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0A4, bool)

// MPEG Low Sample Rate

MKTAG (AUDIO_STREAM_MPEG_LOW_SAMPLE_RATE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0A5, bool)

// MPEG audio layer

enum MPEGAudioLayer
	{
	MPAUDLAYER_AAC,
	MPAUDLAYER_3,								// = MP3
	MPAUDLAYER_2,
	MPAUDLAYER_1
	};

MKTAG (AUDIO_STREAM_MPEG_LAYER, VDRTID_AUDIO_STREAM_PROPERTY, 0x0B0, MPEGAudioLayer)

// MPEG audio layer

enum MPEGAudio
	{
	MPEGAUDIO_1,
	MPEGAUDIO_2
	};

MKTAG (AUDIO_STREAM_MPEG_AUDIO_TYPE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0B1, MPEGAudio)

// Number of channels in Group 2 (DVD Audio only)

MKTAG (AUDIO_STREAM_NUMBER_OF_CHANNELS_GROUP2, VDRTID_AUDIO_STREAM_PROPERTY, 0x0B5, uint16)

// Sample rate in Group 2 (DVD Audio only)

MKTAG (AUDIO_STREAM_SAMPLE_RATE_GROUP2, VDRTID_AUDIO_STREAM_PROPERTY, 0x0B8, uint32)

// Bit Shift in Group 2 (DVD Audio only)
MKTAG (AUDIO_STREAM_BIT_SHIFT_GROUP2, VDRTID_AUDIO_STREAM_PROPERTY, 0x0B9, uint32)

// this may be temp.  For now acc has more output audio downmix/channel modes than we support so use this 
// tag to transmit this info from the acc decoder to the acc mixer
MKTAG (AUDIO_STREAM_AUDIO_CODING_MODE_ACC_EXT, VDRTID_AUDIO_STREAM_PROPERTY, 0x0BA, uint32)


/////////////////////////////////////////////////////////////////////////////
/// LPCM RAW mode
/// This flag is intented to tell the LPCM FA that the data he receives will be
/// raw PCM data (rather than CDDA or other PCM data with header)

MKTAG (AUDIO_STREAM_LPCM_RAW_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x0BB, bool)


//DTS 

//Tag specific to DTS , To configure the Sample blocks in Frame send to LX in Transform Command
MKTAG (AUDIO_DTS_NBSAMPLES_PER_FRAME,	VDRTID_AUDIO_STREAM_PROPERTY, 0x0BC, uint32)

//To specify DTS decoder after sync locked
MKTAG (AUDIO_DTS_SYNC_LOCKED,	VDRTID_AUDIO_STREAM_PROPERTY, 0x0BD, uint32)


#endif	// #ifndef IVDRAUDIOSTREAMTYPES_H

