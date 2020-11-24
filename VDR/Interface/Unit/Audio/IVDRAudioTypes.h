#ifndef IVDRAUDIOTYPES_H
#define IVDRAUDIOTYPES_H

///
/// @brief      VDR Basic Audio Types and Tags
///

#define INCLUDE_OBSOLETE_AUDIO_TAGS	1 // set this to 0 asap

// Include definitions of Tags and Types for Audio Streams
#include "IVDRAudioStreamTypes.h"
#include "VDR/Interface/Base/IVDRMessage.h"


// 
// The Tags in this file MUST NOT be sent as properties of Audio Data Packets!
// They are Tags of (Streaming) Units.
//

/////////////////////////////////////////////////////////////////////////////
// General Audio Unit Tags and their types
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_GENERAL_MODE_PROPERTY = 0x00022000;


/////////////////////////////////////////////////////////////////////////////

// Audio General Mode

enum VDRAudioGeneralMode
	{
	VDR_AUDMOD_UNKNOWN,
	VDR_AUDMOD_CDDA,
	VDR_AUDMOD_DVD_VIDEO,
	VDR_AUDMOD_DVD_AUDIO,
	VDR_AUDMOD_SACD,
	VDR_AUDMOD_FILE
	};	

MKTAG (AUDIO_GENERAL_MODE,				VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x001, VDRAudioGeneralMode)


/////////////////////////////////////////////////////////////////////////////

// Application Mode

enum VDRAudioApplicationMode
	{
	VDR_AAM_UNKNOWN,				// "Don't care"
	VDR_AAM_KARAOKE,				// Playing a Karaoke disk
	VDR_AAM_SURROUND				// Playing a Surround disk
	};

// MKTAG (AUDIO_APPLICATION_MODE, VDRTID_AUDIO_STREAM_PROPERTY, 0x002, VDRAudioApplicationMode)
// if this actually used as a 'STREAM_PROPERTY' it should be moved to IVDRAudioStreamTypes.h
MKTAG (AUDIO_APPLICATION_MODE, VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x002, VDRAudioApplicationMode)	

/////////////////////////////////////////////////////////////////////////////


// CDDA Data format
// Uses Bitmask to indicate formats supported by CDDA decoder:
// ("audio only", "audio subchannel", "subchannel audio", 
//  "header subchannel audio", "header audio")

MKTAG (AUDIO_CDDA_DATA_FORMAT,		VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x003, uint32)

MKTAG (AUDIO_DECODE_LFE,				VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x004, bool)

// mute control

MKTAG (AUDIO_GLOBAL_MUTE,				VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x005, bool)

// test mode control

MKTAG (AUDIO_TEST_SPEAKER_SELECT,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x006, int32)

// voice enhance = center boost

MKTAG (AUDIO_VOICE_ENHANCE_EFFECT,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x007, bool)

// MPEG audio mode is read only: Stereo, Joint-stereo, Mono, or Dual Mono

MKTAG (AUDIO_STREAM_MPEG_MODE,		VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x008, VDRAudioMode)

// DVD Audio Tags

// typedef struct
//	{
//	uint32	coef[8];
//	} DVDAudioDownmixCoefficients;
//
// MKTAG (AUDIO_DOWNMIX_COEFFICIENT, VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x00A, DVDAudioDownmixCoefficients)
// This needs a specific interface, can only use scalars with tags...

MKTAG (AUDIO_DVDA_CHANNEL_ASSIGN,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x009, uint16)


/////////////////////////////////////////////////////////////////////////////

// Active stream ID (for Demux)
MKTAG (AUDIO_STREAM_ID,					VDRTID_AUDIO_GENERAL_MODE_PROPERTY,	0x00A, uint32)

// composite mute control (controlled by audio octopus, combines globalMute and trickModeMute)

MKTAG (AUDIO_COMPOSITE_MUTE,			VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x00B, bool)

// AC3-specific settings, the mode corresponds to enum eDDCompMode

enum AC3DynamicRangeMode
{
	AC3DNR_CUSTOM_MODE_0, // dialog normalization done by external analog circuit (not done by decoder)
	AC3DNR_CUSTOM_MODE_1, // digital?
	AC3DNR_LINE_OUT, // ??
	AC3DNR_RF_MODE // ignore AUDIO_AC3_LOW_DYNAMIC_RANGE and AUDIO_AC3_HIGH_DYNAMIC_RANGE
};

MKTAG (AUDIO_AC3_DYNAMIC_RANGE_MODE,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY,	0x010, AC3DynamicRangeMode)
MKTAG (AUDIO_AC3_LOW_DYNAMIC_RANGE,		VDRTID_AUDIO_GENERAL_MODE_PROPERTY,	0x011, uint32)
MKTAG (AUDIO_AC3_HIGH_DYNAMIC_RANGE,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY,	0x012, uint32)

//??? The following 4 are most likely obsolete...
//MKTAG (AUDIO_AC3_STREAM_ID,			VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x010, uint32)
//MKTAG (AUDIO_DTS_STREAM_ID,			VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x011, uint32)
//MKTAG (AUDIO_LPCM_STREAM_ID,			VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x012, uint32)
//MKTAG (AUDIO_MPEG_STREAM_ID,			VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x013, uint32)


// the application must explicitly gets the IVDRTagUnit interface for each input unit using unitSet->QueryUnitInterface()
// then send this tag to set the master volume of that stream (aux...) at the mixer input..
// 0 = MAX vol (no attenuation) -> linear attenuation in dB
// See also: AUDIO_CHANNEL_VOLUME below
MKTAG (AUDIO_MASTER_VOLUME,	VDRTID_AUDIO_GENERAL_MODE_PROPERTY, 0x018, uint32)



/////////////////////////////////////////////////////////////////////////////
// Tags and their types of units doing standard PCM processing (at decode time)
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_STANDARD_PCM_PROCESSING = 0x00024000;


/////////////////////////////////////////////////////////////////////////////
// Audio Outputs -- Main, VCR(Aux)

enum VDRAudioOutputs
	{
	VDR_AUDOUT_MAIN = 0,
	VDR_AUDOUT_AUX,
	VDR_NUM_AUDOUT_TYPES
	};


/////////////////////////////////////////////////////////////////////////////

// Virtual Surround ("Spatializer") mode

enum VDRAudioSpatializer
	{
	VDR_AUDSPAT_DEFAULT,
	VDR_AUDSPAT_NONE = VDR_AUDSPAT_DEFAULT,
	VDR_AUDSPAT_SRS_TS_3D,		// SRS True Surround or 3D sound, depending on source material
	VDR_AUDSPAT_VMAX_TS_3D		// VMAX True Surround or 3D sound, depending on source material
	// add more here...
	};


enum VDRTruBassSpeakerSize
	{
	VDR_TB_SPEAKER_DEFAULT = 0,
	VDR_TB_SPEAKER_LFRESPONSE_40HZ = VDR_TB_SPEAKER_DEFAULT,
	VDR_TB_SPEAKER_LFRESPONSE_60HZ,
	VDR_TB_SPEAKER_LFRESPONSE_100HZ,
	VDR_TB_SPEAKER_LFRESPONSE_150HZ,
	VDR_TB_SPEAKER_LFRESPONSE_200HZ,
	VDR_TB_SPEAKER_LFRESPONSE_250HZ,
	VDR_TB_SPEAKER_LFRESPONSE_300HZ,
	VDR_TB_SPEAKER_LFRESPONSE_400HZ
	};

enum VDRTruSurroundInputConfig
	{
	VDR_TSINCFG_PASSIVE_MATRIX,	// Lt/Rt directly into L/R channels
	VDR_TSINCFG_1_0,					// C 
	VDR_TSINCFG_2_0,					// L/R
	VDR_TSINCFG_3_0,					// L/C/R 
	VDR_TSINCFG_2_1,					// L/R/Cs 
	VDR_TSINCFG_3_1,					// L/C/R/Cs
	VDR_TSINCFG_2_2,					// L/R/Ls/Rs
	VDR_TSINCFG_3_2,					// L/C/R/Ls/Rs
	VDR_TSINCFG_3_3,					// L/C/R/Ls/Cs/Rs
	VDR_TSINCFG_3_2_BSDIGITAL,		// L/C/R/Ls/Rs compatible with BS Digital AAC content
	VDR_TSINCFG_PL2_MUSIC,			// L/C/R/Ls/Rs (PLII decoded in Music mode) 
	VDR_TSINCFG_CSII,					// CSII decoded material
	VDR_TSINCFG_AUTOMODE,				// TSXT copies pcmin->acmod to TSMode
	VDR_TSINCFG_DEFAULT = VDR_TSINCFG_AUTOMODE
	};


// left in until app is updated with indexed surround tags
MKTAG (AUDIO_SPATIALIZER_MODE,						VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x011, VDRAudioSpatializer)

// trusurround tags -- NOTE: uses indexed tags to support both MAIN and AUX (vcr) outs (see VDRAudioOutput for index values)
MKITG (AUDIO_TRUSURROUND_ENABLE,						VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x011, bool)
MKITG (AUDIO_TRUSURROUND_ENABLE_FOCUS,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x012, bool)
MKITG (AUDIO_TRUSURROUND_ENABLE_TRUBASS,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x013, bool)
MKITG (AUDIO_TRUSURROUND_ENABLE_HEADPHONE,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x014, bool)
MKITG (AUDIO_TRUSURROUND_TRUBASS_SPKRSIZE,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x015, VDRTruBassSpeakerSize)
MKITG (AUDIO_TRUSURROUND_FOCUS_AMOUNT,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x016, uint32)
MKITG (AUDIO_TRUSURROUND_TRUBASS_LEVEL,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x017, uint32)
MKITG (AUDIO_TRUSURROUND_INPUT_GAIN,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x018, uint32)
MKITG (AUDIO_TRUSURROUND_INPUT_CONFIG,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x019, VDRTruSurroundInputConfig)


// Prologic settings

enum VDRDolbyProLogicIIOutputConfig
	{
	VDR_DPLCFG_OFF = 0,
	VDR_DPLCFG_3_0 = 3,	// LCR three stereo
	VDR_DPLCFG_2_1,			// LRS phantom (= center not used)
	VDR_DPLCFG_3_1,			// LCRS
	VDR_DPLCFG_2_2,			// LRSS phantom
	VDR_DPLCFG_3_2,			// LCRSS
	VDR_DPLCFG_DEFAULT = VDR_DPLCFG_3_2
	};

enum VDRDolbyProLogicIIMode
	{
	VDR_DPLMODE_DEFAULT = 0,
	VDR_DPLMODE_PROLOGIC = VDR_DPLMODE_DEFAULT,
	VDR_DPLMODE_VIRTUAL,
	VDR_DPLMODE_MUSIC,
	VDR_DPLMODE_MOVIE,
	VDR_DPLMODE_MATRIX,
	VDR_DPLMODE_RESERVED_1,
	VDR_DPLMODE_RESERVED_2,
	VDR_DPLMODE_CUSTOM
	};

enum VDRDolbyProLogicIICenterWidth
	{
	VDR_DPLCNTRWIDTH_DEFAULT = 0,
	VDR_DPLCNTRWIDTH_0 = VDR_DPLCNTRWIDTH_DEFAULT,
	VDR_DPLCNTRWIDTH_1,
	VDR_DPLCNTRWIDTH_2,
	VDR_DPLCNTRWIDTH_3,
	VDR_DPLCNTRWIDTH_4,
	VDR_DPLCNTRWIDTH_5,
   VDR_DPLCNTRWIDTH_6,
	VDR_DPLCNTRWIDTH_7
	};

enum VDRDolbyProLogicIIDimension
	{
	VDR_DPLDIM_DEFAULT = 0,
	VDR_DPLDIM_MOST_SURR=VDR_DPLDIM_DEFAULT,	// -3
	VDR_DPLDIM_MORE_SURR,	// -2
	VDR_DPLDIM_SURR,			// -1
	VDR_DPLDIM_NEUTRAL,		// 0
	VDR_DPLDIM_CNTR,			// +1
	VDR_DPLDIM_MORE_CNTR,	// +2
	VDR_DPLDIM_MOST_CNTR		// +3
	};


MKTAG (AUDIO_PROLOGICII_ENABLE,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x030, bool)
MKTAG (AUDIO_PROLOGICII_CONFIG,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x031, VDRDolbyProLogicIIOutputConfig)
MKTAG (AUDIO_PROLOGICII_MODE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x032, VDRDolbyProLogicIIMode)
MKTAG (AUDIO_PROLOGICII_CENTERWIDTH,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x033, VDRDolbyProLogicIICenterWidth)
MKTAG (AUDIO_PROLOGICII_DIMENSION,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x034, VDRDolbyProLogicIIDimension)
MKTAG (AUDIO_PROLOGICII_PANORAMA,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x035, bool)
MKTAG (AUDIO_PROLOGICII_AUTOBALANCE,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x036, bool)
MKTAG (AUDIO_PROLOGICII_INV_POLARITY,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x037, bool)
MKTAG (AUDIO_PROLOGICII_SURROUND_FILTER,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x038, bool)
MKTAG (AUDIO_PROLOGICII_PCMSCALE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x039, uint32)


/////////////////////////////////////////////////////////////////////////////


/*JK added for Mp3 encoding*/


enum SamplingRateFreq
	{
	SRF44100HZ,
	SRF48000HZ
	};

enum BitRate
	{
	KBPERSEC32,
	KBPERSEC48,
	KBPERSEC64,
	KBPERSEC96,
	KBPERSEC128,
	KBPERSEC192,
	KBPERSEC256,
	KBPERSEC320
	};


enum ChannelMode
	{
		Stereo,
		JointStereo,
		DualChannel,
		Mono_Ch
	};

enum MP3_Emphasis
	{
		No_Emphasis,
		MS_50_OR_15MS,
		RESERVED,
		CCIT_J_17
	};
typedef struct _MP3EncoderParam
	{

	bool errorProtection;
	BitRate  lowBitRate;
	BitRate  upperBitRate;
	SamplingRateFreq samplingRate;
	bool padding;
	ChannelMode channelMode ;
	bool copyright;
	bool original;
	MP3_Emphasis emphasis;
	}MP3EncoderParam;

enum CodecType
	{
		MPEG1Layer3,
		MPEG2Layer3,
		MPEG2_5Layer3,
		OggVorbis,
		ACC,
		ATRAC,
		AC3,
		WMAVer7,
		WMAVer8,
		WMAVer9
	};
typedef struct _MP3EncoderProperties
	{
	
	uint16 maxPreDelayTime;
	uint32 encoderInfo;
	CodecType codecType;
	}MP3EncoderProperties;


//static const VDRTID VDRTID_MP3_ENCODER = 0x0004900e;

static const VDRTID VDRTID_MP3_ENCODER = 0x00031000;

// Sample rate in Hz
MKTAG (MP3_ERROR_PROTECTION,  	VDRTID_MP3_ENCODER, 0x08a,bool) 

MKTAG (MP3_UPPER_BIT_RATE, 		VDRTID_MP3_ENCODER, 0x08b, BitRate )

MKTAG (MP3_LOWER_BIT_RATE, 	VDRTID_MP3_ENCODER, 0x08c, BitRate )

MKTAG (MP3_PADDING, 			VDRTID_MP3_ENCODER, 0x08d, bool)

MKTAG (MP3_CHANNEL_MODE, 		VDRTID_MP3_ENCODER, 0x08e, ChannelMode)

MKTAG (MP3_COPYRIGHT, 			VDRTID_MP3_ENCODER, 0x08f, bool)

MKTAG (MP3_ORIGINAL, 			VDRTID_MP3_ENCODER, 0x084, bool)

MKTAG (MP3_EMPHASIS, 			VDRTID_MP3_ENCODER, 0x085, MP3_Emphasis)

MKTAG (MP3_SAMPLING_RATE_FREQ, VDRTID_MP3_ENCODER, 0x086, SamplingRateFreq )

//Jk ends 


// output configuration and downmix

enum VDRAudioDownmixMode // was "DownmixMode2Channels" with 55xx
	{
	VDR_DMM2C_STEREO,										// pure stereo
	VDR_DMM2C_PROLOGIC_COMPATIBLE						// Pro Logic compatible
	};

enum VDRAudioDualModeConfig // was "AC3DualModeConfig" with 55xx
	{
	VDR_ADM_DEFAULT,		// default is stereo
	VDR_ADM_STEREO = VDR_ADM_DEFAULT,
	VDR_ADM_CHANNEL1,		// channel 1 on both L/R
	VDR_ADM_CHANNEL2,		// channel 2 on both L/R
	VDR_ADM_MIX				// mix channel 1 and 2 to mono, output on both L/R
	};

enum VDRAudioSpeakerConfig // was "AC3SpeakerConfig" in 55xx
	{
	VDR_AC3SC_20_SURROUND_COMPATIBLE,
	VDR_AC3SC_10,
	VDR_AC3SC_20_NORMAL,
	VDR_AC3SC_30,
	VDR_AC3SC_21,
	VDR_AC3SC_31,
	VDR_AC3SC_22,
	VDR_AC3SC_32,
	VDR_AC3SC_KARAOKE,	// This may only be available for MPEG-2 audio, but it is added to the AC3 settings to avoid introducing new tags
	VDR_AC3SC_MODE_ID	// To configure MODE_ID
	};


// needed in addition to the stream tag so the MacroProcessor can be notified via unit tag
enum VDRDeemphMode
	{
	VDR_DEEMPH_50_15_US,
   VDR_DEEMPH_CCITT_J_17
	};

enum VDRDeemphEnable
	{
	VDR_DEEMPH_DISABLED,
   VDR_DEEMPH_ENABLED,
	VDR_DEEMPH_AUTO_ENABLED
	};

// NOTE: AUDIO_DOWNMIXMODE and AUDIO_SPEAKER_CONFIG use indexed tags to support both MAIN and AUX (vcr) outs (see VDRAudioOutput for index values)
MKITG (AUDIO_DOWNMIXMODE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x042, VDRAudioDownmixMode)
MKITG (AUDIO_SPEAKER_CONFIG,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x044, VDRAudioSpeakerConfig)

MKTAG (AUDIO_DUAL_MODE_CONFIG,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x046, VDRAudioDualModeConfig)
MKTAG (AUDIO_AC3EX_ENABLE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x047, bool)
MKTAG (AUDIO_DEEMPH_ENABLE,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x048, VDRDeemphEnable)	// note that this is not a bool!
MKTAG (AUDIO_DEEMPH_MODE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x049, VDRDeemphMode)

MKTAG (AUDIO_DOWNMIX_STEREO_UPMIX,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x051, bool)
MKTAG (AUDIO_DOWNMIX_MONO_UPMIX,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x052, bool)
MKTAG (AUDIO_DOWNMIX_MEAN_SURROUND,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x053, bool)
MKTAG (AUDIO_DOWNMIX_SECOND_STEREO,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x054, bool)
MKTAG (AUDIO_DOWNMIX_NORMALIZE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x055, bool)
MKTAG (AUDIO_DOWNMIX_NORM_IDX,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x056, uint32)
MKTAG (AUDIO_DOWNMIX_DIALOG_ENHANCE,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x057, bool)
#if WATERMARKING_ENABLED
	// TAG from NAV to enable Watermarking in Decoder postprocessing
	MKTAG (AUDIO_WATERMARKING_ENABLE,VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x058, bool)
#endif


/////////////////////////////////////////////////////////////////////////////
// ST Omni Surround Settings

enum VDROmniSurroundInputMode
	{
	VDR_OSIM_MATRIX,	/*0  Lt, Rt */
	VDR_OSIM_1_0,		/* 1 C , will not be used, for mono channel processing, please refer to ST WideSurround */
	VDR_OSIM_2_0,		/* 2 L, R */
	VDR_OSIM_3_0,		/* 3 L, R, C */
	VDR_OSIM_2_1,		/* 4 L, R, Ls, but the Rs buffer is needed during the virtualization process */
	VDR_OSIM_3_1,		/* 5 L, R, C, Ls. also for plologic I decoded, but Rs buffer is needed during the virtualization process */
	VDR_OSIM_2_2,		/* 6 L, R, Ls, Rs */
	VDR_OSIM_3_2,		/* 7 L, R, C, Ls, Rs, also for plologic II decoded */
	VDR_OSIM_2_3,		/* 8 L, R, Ls, Rs, Csl */
	VDR_OSIM_3_3,		/* 9 L, R, C, Ls, Rs, Csl */
	VDR_OSIM_2_4,		/* 10 L, R, Ls, Rs, Csl Csr */
	VDR_OSIM_3_4,		/* 11 L, R, C, Ls, Rs, Csl, Csr */
	VDR_OSIM_AUTO		/* 12 Auto mode added */
	};

enum VDROmniSurroundMode
	{
	VDR_OSM_BYPASS,			/*Bypass mode, mainly used for dialog enhancement alone purpose */
	VDR_OSM_DOWNMIX,			/*Dowmmix mode, mainly used for dialog enhancement alone for two channel output */
	VDR_OSM_FRONTBYPASS,		/*Front channel bypass mode */
	VDR_OSM_ACTIVATE			/* Front channel widening mode */
	};

enum VDROmniSurroundLfeMode
	{
	VDR_OSLM_DISABLE,	/* LFE not processed */
	VDR_OSLM_ENABLE,		/* LFE channel exist at the output mainly used for stereo output with subwoofer */
	VDR_OSLM_DOWNMIX		/* LFE channel mixed into the stereo output channels */
	};

enum VDROmniSurroundDialogLevel
	{
	VDR_OSDL_0,    /*Lowest*/
	VDR_OSDL_1,
	VDR_OSDL_2,
	VDR_OSDL_3     /*Highest*/
	};

// NOTE: uses indexed tags to support both MAIN and AUX (vcr) outs (see VDRAudioOutput for index values)
MKITG (AUDIO_OMNISURROUND_ENABLE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x060, bool)
MKITG (AUDIO_OMNISURROUND_INPUT_MODE,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x061, VDROmniSurroundInputMode)
MKITG (AUDIO_OMNISURROUND_SURROUND_MODE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x062, VDROmniSurroundMode)
MKITG (AUDIO_OMNISURROUND_ENABLE_DIALOG_MODE,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x063, bool)
MKITG (AUDIO_OMNISURROUND_LFE_MODE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x064, VDROmniSurroundLfeMode)
MKITG (AUDIO_OMNISURROUND_DIALOG_LEVEL,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x065, VDROmniSurroundDialogLevel)


/////////////////////////////////////////////////////////////////////////////
// ST Wide Surround Settings

enum VDRWideSurroundMode
	{
	VDR_WSM_SIMULATED,
	VDR_WSM_MUSIC,
	VDR_WSM_MOVIE
	};

enum VDRWideSurroundBassFreq
	{
	VDR_WSBF_40HZ,
	VDR_WSBF_90HZ,
	VDR_WSBF_120HZ,
	VDR_WSBF_160HZ
	};

enum VDRWideSurroundMidFreq
	{
	VDR_WSMF_200HZ,
	VDR_WSMF_400HZ,
	VDR_WSMF_500HZ,
	VDR_WSMF_600HZ
	};

enum VDRWideSurroundTrebFreq
	{
	VDR_WSTF_2KHZ,
	VDR_WSTF_4KHZ,
	VDR_WSTF_5KHZ,
	VDR_WSTF_6KHZ
	};

// NOTE: uses indexed tags to support both MAIN and AUX (vcr) outs (see VDRAudioOutput for index values)
MKITG (AUDIO_WIDESURROUND_ENABLE,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06a, bool)
MKITG (AUDIO_WIDESURROUND_GAIN,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06b, uint32)
MKITG (AUDIO_WIDESURROUND_MODE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06c, VDRWideSurroundMode)
MKITG (AUDIO_WIDESURROUND_BASS_FREQ,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06d, VDRWideSurroundBassFreq)
MKITG (AUDIO_WIDESURROUND_MID_FREQ,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06e, VDRWideSurroundMidFreq)
MKITG (AUDIO_WIDESURROUND_TREB_FREQ,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x06f, VDRWideSurroundTrebFreq)


/////////////////////////////////////////////////////////////////////////////
// Circle Surround II Settings
enum VDRCircleSurroundIIMode
	{
   VDR_CSIIM_CINEMA,
   VDR_CSIIM_MUSIC,
   VDR_CSIIM_MONO
	};

enum VDRCircleSurroundIIOutputMode
	{
   VDR_CSIIOM_STEREO,
   VDR_CSIIOM_MULTICHANNEL
	};

MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE,						VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x070, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_PHANTOM,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x071, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_CTR_FB,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x072, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_IS525,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x073, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_CTR_RR,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x074, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_CTR_RR_FB,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x075, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_TRUBASS_SUB,		VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x076, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_TRUBASS_FRONT,	VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x077, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_ENABLE_FOCUS,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x078, bool)
MKTAG (AUDIO_CIRCLESURROUNDII_TRUBASS_SPKRSIZE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x079, VDRTruBassSpeakerSize)
MKTAG (AUDIO_CIRCLESURROUNDII_OUTMODE,						VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x07a, VDRCircleSurroundIIOutputMode)
MKTAG (AUDIO_CIRCLESURROUNDII_MODE,							VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x07b, VDRCircleSurroundIIMode)
MKTAG (AUDIO_CIRCLESURROUNDII_INPUT_GAIN,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x07c, uint32)
MKTAG (AUDIO_CIRCLESURROUNDII_TRUBASS_LEVEL,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x07d, uint32)
MKTAG (AUDIO_CIRCLESURROUNDII_FOCUS_AMOUNT,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x07e, uint32)


/////////////////////////////////////////////////////////////////////////////
// Channel Synthesizer Settings

enum VDRChannelSynthMode
	{
	VDR_CHSYN_MODE_MOVIE,
	VDR_CHSYN_MODE_MUSIC
	};

enum VDRChannelSynthOutAcMode
	{
	VDR_CHSYN_OUTACMODE_30,
	VDR_CHSYN_OUTACMODE_21,
	VDR_CHSYN_OUTACMODE_31,
	VDR_CHSYN_OUTACMODE_22,
	VDR_CHSYN_OUTACMODE_32
	};

MKTAG (AUDIO_CHANNELSYNTH_ENABLE,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x080, bool)
MKTAG (AUDIO_CHANNELSYNTH_MODE,					VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x081, VDRChannelSynthMode)
MKTAG (AUDIO_CHANNELSYNTH_OUTMODE,				VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x082, VDRChannelSynthOutAcMode)
MKTAG (AUDIO_CHANNELSYNTH_ENABLE_LFE,			VDRTID_AUDIO_STANDARD_PCM_PROCESSING, 0x083, bool)



/////////////////////////////////////////////////////////////////////////////
// Tags and their types of units doing PCM (post) processing at render time
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_RENDERER_PCM_PROCESSING = 0x00043000;

/////////////////////////////////////////////////////////////////////////////

// PCM Output Configuration (= Bass Redirection Scheme)

enum VDRPCMOutputConfig
	{
	VDR_PCMOCFG_DEFAULT,	// ALL, scaled, is default
	VDR_PCMOCFG_ALL = VDR_PCMOCFG_DEFAULT,
	VDR_PCMOCFG_LSW,
	VDR_PCMOCFG_LLR,	
	VDR_PCMOCFG_SLP,	
	VDR_PCMOCFG_SUM,		// subwoofer = sum of all input channels
	VDR_PCMOCFG_BYP,		// Bypass
	VDR_PCMOCFG_LSW_WF	// same as config 1 without filters
	};

enum VDRBassMgmtType
	{
	VDR_BASSMGT_OFF,	
	VDR_BASSMGT_DOLBY_1,
	VDR_BASSMGT_DOLBY_2,
	VDR_BASSMGT_DOLBY_3,
	VDR_BASSMGT_SIMPLIFIED,
	VDR_BASSMGT_DOLBY_CERT,
	VDR_BASSMGT_PHILIPS,
	VDR_BASSMGT_RESERVED1,
	VDR_BASSMGT_RESERVED2,
	VDR_BASSMGT_RESERVED3
	};

MKTAG (AUDIO_BASSMGMT_TYPE,					VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x001, VDRPCMOutputConfig)
MKTAG (AUDIO_BASSMGMT_LFE,						VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x002, bool)
MKTAG (AUDIO_BASSMGMT_BOOST,					VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x003, bool)
MKTAG (AUDIO_BASSMGMT_SURROUND_FILTERS,	VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x004, bool)
MKTAG (AUDIO_BASSMGMT_WORDSIZE,				VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x005, uint32)
MKTAG (AUDIO_BASSMGMT_LIMITER_INDEX,		VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x006, uint32)


enum VDRAudioChannels
	{
	VDR_AUDIO_LEFT,
	VDR_AUDIO_RIGHT,
	VDR_AUDIO_CENTER,
	VDR_AUDIO_LFE,
	VDR_AUDIO_LEFT_SURROUND,
	VDR_AUDIO_RIGHT_SURROUND,
	VDR_AUDIO_CENTER_SURROUND_LEFT,
	VDR_AUDIO_CENTER_SURROUND_RIGHT
	};


// speaker delays - unit is milliseconds
MKITG (AUDIO_CHANNEL_DELAY, VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x010, uint32)

// total speaker delay for all speakers combined - unit is samples
MKTAG (AUDIO_TOTAL_SPEAKER_DELAY_SAMPLES,	VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x011, uint32)

//// speaker volumes	--	attenuation in db
// See also: AUDIO_MASTER_VOLUME aboce
MKITG (AUDIO_CHANNEL_VOLUME,						VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x02F, uint32)

#if INCLUDE_OBSOLETE_AUDIO_TAGS // this needs to be removed
// speaker volumes	--	attenuation in db
MKTAG (AUDIO_VOLUME_CENTER,						VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x020, uint32)
MKTAG (AUDIO_VOLUME_LEFT_SURROUND,				VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x021, uint32)
MKTAG (AUDIO_VOLUME_LEFT,							VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x022, uint32)
MKTAG (AUDIO_VOLUME_RIGHT_SURROUND,				VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x023, uint32)
MKTAG (AUDIO_VOLUME_RIGHT,							VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x024, uint32)
MKTAG (AUDIO_VOLUME_SUBWOOFER,					VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x025, uint32)
MKTAG (AUDIO_VOLUME_CENTER_SURROUND_LEFT,		VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x026, uint32)
MKTAG (AUDIO_VOLUME_CENTER_SURROUND_RIGHT,	VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x027, uint32)
#endif // INCLUDE_OBSOLETE_AUDIO_TAGS

MKTAG (AUDIO_VOLUME_DAMP,				VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x026, bool)


// EQ 
enum VDREQGainBands
	{
	EQ_GAIN_BAND_0,
	EQ_GAIN_BAND_1,
	EQ_GAIN_BAND_2,
	EQ_GAIN_BAND_3,
	EQ_GAIN_BAND_4,
	EQ_GAIN_BAND_5,
	EQ_GAIN_BAND_6,
	EQ_GAIN_BAND_7
	};

MKTAG (AUDIO_EQ_ENABLE,			VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x030, bool)

// eq	--	gain in db; use VDREQGainBands as index for tags
MKITG (AUDIO_EQ_GAIN,			VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x031, uint32)


// DC Offset Remove
MKTAG (AUDIO_DCREMOVE,			VDRTID_AUDIO_RENDERER_PCM_PROCESSING, 0x03A, bool)

/////////////////////////////////////////////////////////////////////////////
// Tags and their types of units doing audio decoding information
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_STATUS_INFORMATION = 0x00044000;

//Get only tags for getting decoder status information
MKTAG (AUDIO_STATUS_AUDIO_CODING_MODE, VDRTID_AUDIO_STATUS_INFORMATION, 0x001, VDRAudioCodingMode)
MKTAG (AUDIO_STATUS_LFE_PRESENCE, VDRTID_AUDIO_STATUS_INFORMATION, 0x002, bool)
MKTAG (AUDIO_STATUS_SAMPLING_RATE, VDRTID_AUDIO_STATUS_INFORMATION, 0x003, uint32)



/////////////////////////////////////////////////////////////////////////////
// Karaoke Tags and their types
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_KARAOKE_PROPERTY = 0x0002c000;

enum KaraokeConfig // was "AC3KaraokeConfig" in 55xx
	{
	AC3KARA_AWARE,
	AC3KARA_DEFAULT,
	AC3KARA_OFF = AC3KARA_DEFAULT,
	AC3KARA_MULTICHANNEL	= 3,
	AC3KARA_CAPABLE_NO_VOCAL,
	AC3KARA_CAPABLE_V1,
	AC3KARA_CAPABLE_V2,
	AC3KARA_CAPABLE_BOTH_VOCAL
	};

enum KaraokeVoiceEffectType
	{  
	KARAOKE_VOICE_EFFECT_NONE = 0,
	KARAOKE_VOICE_EFFECT_ECHO = 1,
	KARAOKE_VOICE_EFFECT_CHORUS = 2,
	KARAOKE_VOICE_EFFECT_REVERB = 3
	};

MKTAG (AUDIO_KARAOKE_CONFIG,								VDRTID_KARAOKE_PROPERTY, 0x001, KaraokeConfig)
MKTAG (AUDIO_KARAOKE_DUET_THRESHOLD,					VDRTID_KARAOKE_PROPERTY, 0x002, int32)
MKTAG (AUDIO_KARAOKE_ECHO_FEEDBACK,						VDRTID_KARAOKE_PROPERTY, 0x003, int32)
MKTAG (AUDIO_KARAOKE_ECHO_VOLUME,						VDRTID_KARAOKE_PROPERTY, 0x004, int32)
MKTAG (AUDIO_KARAOKE_ENABLE,								VDRTID_KARAOKE_PROPERTY, 0x005, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_DUET,						VDRTID_KARAOKE_PROPERTY, 0x006, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_MUSIC_CHANNEL_MUTE,		VDRTID_KARAOKE_PROPERTY, 0x007, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_PITCH_SHIFT,				VDRTID_KARAOKE_PROPERTY, 0x008, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_VOICE_CANCELLATION,		VDRTID_KARAOKE_PROPERTY, 0x009, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_VOICE_CHANNEL_MUTE,		VDRTID_KARAOKE_PROPERTY, 0x00A, bool)
MKTAG (AUDIO_KARAOKE_ENABLE_TEMPO_CONTROL,			VDRTID_KARAOKE_PROPERTY, 0x00B, bool)
MKTAG (AUDIO_KARAOKE_LEFT_CHANNEL_MUSIC_VOLUME,		VDRTID_KARAOKE_PROPERTY, 0x00C, int32)
MKTAG (AUDIO_KARAOKE_LEFT_CHANNEL_VOICE_VOLUME,		VDRTID_KARAOKE_PROPERTY, 0x00D, int32)
MKTAG (AUDIO_KARAOKE_MIKE_PORT_SELECT,					VDRTID_KARAOKE_PROPERTY, 0x00E, int32)
MKTAG (AUDIO_KARAOKE_MUSIC_GAIN,							VDRTID_KARAOKE_PROPERTY, 0x00F, int32)
MKTAG (AUDIO_KARAOKE_MUSIC_IN_SHIFT,					VDRTID_KARAOKE_PROPERTY, 0x010, int32)
MKTAG (AUDIO_KARAOKE_MUSIC_OUT_SHIFT,					VDRTID_KARAOKE_PROPERTY, 0x011, int32)
MKTAG (AUDIO_KARAOKE_PITCH_SHIFT,						VDRTID_KARAOKE_PROPERTY, 0x012, int32)
MKTAG (AUDIO_KARAOKE_RIGHT_CHANNEL_MUSIC_VOLUME,	VDRTID_KARAOKE_PROPERTY, 0x013, int32)
MKTAG (AUDIO_KARAOKE_RIGHT_CHANNEL_VOICE_VOLUME,	VDRTID_KARAOKE_PROPERTY, 0x014, int32)
MKTAG (AUDIO_KARAOKE_VOICE_CANCELLATION,				VDRTID_KARAOKE_PROPERTY, 0x015, int32)
MKTAG (AUDIO_KARAOKE_VOICE_EFFECT,						VDRTID_KARAOKE_PROPERTY, 0x016, KaraokeVoiceEffectType)
MKTAG (AUDIO_KARAOKE_VOICE_EFFECT_BALANCE,			VDRTID_KARAOKE_PROPERTY, 0x017, int32)
MKTAG (AUDIO_KARAOKE_VOICE_EFFECT_DELAY,				VDRTID_KARAOKE_PROPERTY, 0x018, int32)
MKTAG (AUDIO_KARAOKE_VOICE_GAIN,							VDRTID_KARAOKE_PROPERTY, 0x019, int32)
MKTAG (AUDIO_KARAOKE_VOICE_IN_SHIFT,					VDRTID_KARAOKE_PROPERTY, 0x01A, int32)
MKTAG (AUDIO_KARAOKE_VOICE_OUT_SHIFT,					VDRTID_KARAOKE_PROPERTY, 0x01B, int32)


/////////////////////////////////////////////////////////////////////////////
// SPDIF Configuration Tags for each audio type:
// User preference for SPDIF (bitstream, PCM, or OFF)
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_SPDIF_PROPERTY = 0x0002d000;

// This can be specified for each audio type
enum VDRSPDIFOutputConfiguration
	{
	VDR_SPDIFOCFG_PCM,									// PCM output
	VDR_SPDIFOCFG_COMPRESSED,							// Compressed output
	VDR_SPDIFOCFG_PCM_MUTE_NULL,						// Mute in PCM mode with NULL output
	VDR_SPDIFOCFG_COMPRESSED_MUTE_PAUSE_BURSTS,	// Mute in Compressed mode using Pause Bursts
	VDR_SPDIFOCFG_OFF										// SPDIF output physically off
	};

MKTAG (AUDIO_AC3_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x001, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_CDDA_DTS_SPDIF_CONFIG,	VDRTID_SPDIF_PROPERTY, 0x002, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_DTS_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x003, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_LPCM_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x004, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_MPEG1_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x005, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_MPEG2_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x006, VDRSPDIFOutputConfiguration)
MKTAG (AUDIO_WMA_SPDIF_CONFIG,		VDRTID_SPDIF_PROPERTY, 0x007, VDRSPDIFOutputConfiguration)


// Do we need seperate config for each audio type or a common one like this is enough???
MKITG (AUDIO_SPDIF_CONFIG,				VDRTID_SPDIF_PROPERTY, 0x008, VDRSPDIFOutputConfiguration)

// SPDIF configuration for SPDIF Player - usually not used by applications (these
// will set the audio type specific SPDIF configuration flags.
MKTAG (AUDIO_SPDIF_PLAYER_CONFIG,	VDRTID_SPDIF_PROPERTY, 0x100, VDRSPDIFOutputConfiguration)


/////////////////////////////////////////////////////////////////////////////
// PCM Reader Configuration Tags 
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_FORMAT_PROPERTY = 0x00025000;

MKTAG (AUDIO_SAMPLE_RATE,      VDRTID_AUDIO_FORMAT_PROPERTY, 0x001, uint32)
MKTAG (AUDIO_BITS_PER_SAMPLE,  VDRTID_AUDIO_FORMAT_PROPERTY, 0x002, uint32)
MKTAG (AUDIO_SAMPLE_WORD_SIZE, VDRTID_AUDIO_FORMAT_PROPERTY, 0x003, uint32)
MKTAG (AUDIO_FRAME_DURATION,   VDRTID_AUDIO_FORMAT_PROPERTY, 0x004, uint32) //in milliseconds
MKTAG (PRE_DELAY_TIME,   VDRTID_AUDIO_FORMAT_PROPERTY, 0x005, uint32) //in milliseconds
MKTAG (MP3_ENCODER_STATUS,   VDRTID_AUDIO_FORMAT_PROPERTY, 0x006, bool) //in milliseconds

/////////////////////////////////////////////////////////////////////////////
// PCM Player Configuration Tags 
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_PLAYER = 0x00049000;

// Sample rate in Hz
MKTAG (AUDIO_PLAYER_SAMPLE_RATE,      VDRTID_AUDIO_PLAYER, 0x001, uint32)

// Desired oversampling (256, 384, 512, etc.)
MKTAG (AUDIO_PLAYER_OVERSAMPLING,     VDRTID_AUDIO_PLAYER, 0x002, uint32)

//Decide whether to PCM player FDMA to use SRC or not
MKTAG (AUDIO_PLAYER_FDMA_USE_SRC,     VDRTID_AUDIO_PLAYER, 0x003, uint32)

//Decide whether to PCM player FDMA to use 2 channel or 10 channel
MKTAG (AUDIO_PLAYER_FDMA_CHANNLE_2_OR_NOT,     VDRTID_AUDIO_PLAYER, 0x004, uint32)

//Decide to configure the Audio Mixer to perfom Pink Noise Test
MKTAG (AUDIO_PLAYER_PINK_NOISE_GENERATOR,     VDRTID_AUDIO_PLAYER, 0x005, uint32)

/////////////////////////////////////////////////////////////////////////////
//	CDDA Data Format Tags and their Types
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_AUDIO_CDDA_PROPERTY = 0x00036000;

// CDDA Data format

enum CDDADataFormat
	{
	VDR_CDDADF_AUDIO_ONLY,								// Only audio data
	VDR_CDDADF_DEFAULT = VDR_CDDADF_AUDIO_ONLY,	// Only audio data
	VDR_CDDADF_AUDIO_SUBCHANNEL,			//For each block first audio data (2352 bytes), then subchannel (98 bytes)
	VDR_CDDADF_SUBCHANNEL_AUDIO,			// For each block first subchannel (98 bytes), then audio data (2352 bytes)
	VDR_CDDADF_HEADER_SUBCHANNEL_AUDIO,	
	VDR_CDDADF_HEADER_AUDIO				

	};

MKTAG (CDDA_DATA_FORMAT, VDRTID_AUDIO_CDDA_PROPERTY, 0x001, CDDADataFormat)

////////////////////////////////////////////////////////////////////////////
//
//	Audio Decoder Notification Messages
//
#if WATERMARKING_ENABLED
	static const VDRMID VDRMID_AUDIO_WATERMARKS_OCCURRED = 0x00078000;	// param1 may contain the error quantity
#endif


#endif

