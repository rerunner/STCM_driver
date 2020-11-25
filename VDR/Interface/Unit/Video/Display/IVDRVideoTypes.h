///
/// @brief      Video related types.
///

#ifndef VDRVIDEOTYPES_H
#define VDRVIDEOTYPES_H

#include "VDR/Interface/Unit/IVDRTags.h"

enum MovingImageStandard
	{
	MIS_INVALID,
	MIS_UNKNOWN,
	MIS_480_29_97,
	MIS_576_25,
	MIS_NUMBER
	};

enum ColorStandard
	{
	CS_INVALID,
	CS_UNKNOWN,
	CS_NTSC,
	CS_PAL,
	CS_SECAM,
	CS_NUMBER
	};


enum VideoStandard
	{
	VSTD_INVALID,
	VSTD_UNKNOWN,
	VSTD_NTSC,
	VSTD_PAL,
	VSTD_SECAM,
	VSTD_HDTV,
	VSTD_NUMBER
	};

/// @brief Enum BitmapPolarity contains information which field the bitmap contains

enum VideoBitmapContent
	{
	VBC_TOP_FIELD,		//< Top field
	VBC_BOTTOM_FIELD,	//< Bottom field
	VBC_FULL			//< both fields or frame (depending on interlaced/progressive encoding)
	};

enum NormVideoStandard
	{
	NVS_INVALID,
	NVS_UNKNOWN,
	NVS_PAL_BDGHI,		// PAL Europe
	NVS_PAL_N,			// PAL 358 for Paraguay, Uruguay (IRE7.5)
	NVS_PAL_NC,			// PAL 358 for Argentina (no pedestal)
	NVS_PAL_I_60,		// PAL 60 of PAL_BDGHI
	NVS_PAL_M,			// PAL 60 of PAL_N and for Brazil
	NVS_PAL_MC,			// PAL 60 of PAL_NC
	NVS_NTSC_M,			// NTSC USA
	NVS_NTSC_MC,		// NTSC Japan (no pedestal)
	NVS_NTSC_M_443,	// NTSC for US Army Europe
	NVS_NUMBER
	};


enum VideoSource
	{
	VSRC_INVALID,
	VSRC_UNKNOWN,
	VSRC_COMPOSITE,
	VSRC_SVIDEO,
	VSRC_SERIAL_DIGITAL,
	VSRC_RGB,
	VSRC_NUMBER
	};


//Obsolete

enum VideoPresentationMode
	{
	VPRM_INVALID,
	VPRM_UNKNOWN,
	VPRM_4_BY_3_NORMAL,
	VPRM_4_BY_3_FULL,
	VPRM_4_BY_3_LETTERBOXED,
	VPRM_16_BY_9_ANAMORPHIC,
	VPRM_14_BY_9_LETTERBOXED_CENTER, // ETS-300 aspect ratio
	VPRM_14_BY_9_LETTERBOXED_TOP,    // defined by DVD-VR VR5-78
	VPRM_16_BY_9_LETTERBOXED_TOP,
	VPRM_LARGE_16_BY_9_LETTERBOXED_CENTER,
	VPRM_14_BY_9_ANAMORPHIC,
	VPRM_RESERVED,
	VPRM_NUMBER
	};


enum MPEGPresentationMode
	{
	MP2PM_INVALID,
	MP2PM_UNKNOWN,
	MP2PM_FULLSIZE_4_BY_3,	///< Picture to be displayed at full size, and source material is 4:3
	MP2PM_FULLSIZE_16_BY_9,	///< Picture to be displayed at full size, and source material is 16:9 (anamorphic)
	MP2PM_LETTERBOXED,		///< Picture to be displayed letterboxed, and source material is 16:9 (anamorphic)
	MP2PM_PANSCAN,				///< Picture to be displayed in pan scan, and source material is 16:9 (anamorphic)
	MP2PM_NUMBER
	};


enum PALVideoSubStandard
	{
	PALVSSTD_INVALID,
	PALVSSTD_UNKNOWN,
	PALVSSTD_DEFAULT,
	PALVSSTD_BDGHI,
	PALVSSTD_N,
	PALVSSTD_NC,
	PALVSSTD_NUMBER
	};

enum NTSCVideoSubStandard
	{
	NTSCVSSTD_INVALID,
	NTSCVSSTD_UNKNOWN,
	NTSCVSSTD_DEFAULT,
	NTSCVSSTD_NORMAL,
	NTSCVSSTD_PALM,
	NTSCVSSTD_443,
	NTSCVSSTD_JAPAN,
	NTSCVSSTD_PAL60,
	NTSCVSSTD_NUMBER
	};

// Enable/disable progressive scan output 
enum VideoProscanMode
	{
	VIDPROS_INVALID,
	VIDPROS_UNKNOWN,
	VIDPROS_DEFAULT,
	VIDPROS_OFF = VIDPROS_DEFAULT,	// Progressive scan off
	VIDPROS_ON,								// Progressive scan on
	VIDPROS_NUMBER
	};

enum VideoRectangleMode
	{
	VRM_INVALID,
	VRM_UNKNOWN,

	VRM_DEFAULT,
	VRM_FULL_SIZE = VRM_DEFAULT,
	VRM_APPLY_RECTANGLE,

	VRM_NUMBER
	};
	
enum VideoCopyMode
	{
	VCPMD_INVALID,
	VCPMD_UNKNOWN,
	VCPMD_DEFAULT,	// depends on implementation (e.g. "do not indicate on video output"). Use for
						// content that does not contain copyright protection information.

	// Use the following three when the content contains copyright protection information
	VCPMD_COPYING_PERMITTED,		// indicate that copying is always allowed
	VCPMD_ONE_COPY_PERMITTED,		// one copy allowed
	VCPMD_NO_COPYING_PERMITTED,	// no copy allowed
	VCPMD_NUMBER
	};

enum VideoCopyProtection
	{
	VCP_OFF = 0,	// Macrovision is turned off
	VCP_STAGE1,		// Stage 1 is enabled only
	VCP_STAGE2,
	VCP_STAGE3
	};
	
enum VideoMode
	{
	VMOD_RESET,
	VMOD_PATTERN,
	VMOD_CAPTURE,
	VMOD_PLAYBACK,
	VMOD_PATTERNSYNC
	};


// for NTSC bit11 ASB
enum VideoAnalogueSourceMode
	{
	VASMD_DEFAULT,
	VASMD_ANALOGUE_PRE_RECORDED,
	VASMD_NOT_ANALOGUE_PRE_RECORDED
	};

// for PAL WSS (ETS300-294)
enum VideoCopyrightMode
	{
	VCRMD_DEFAULT,
	VCRMD_NO_COPYRIGHT_ASSERTED,
	VCRMD_COPYRIGHT_ASSERTED
	};

enum VideoFilmCameraMode
	{
	VFCMD_DEFAULT,
	VFCMD_CAMERA,
	VFCMD_FILM
	};

enum VideoSubTitlingMode
	{
	VSTM_DEFAULT,
	VSTM_NO_SUBTITLE,
	VSTM_INSIDE_ACTIVE_PICTURE,
	VSTM_OUTSIDE_ACTIVE_PICTURE
	};


// Video settings for bobbing and weaving for progressive scan 
enum VideoDeinterlaceMode
	{
	VDIM_INVALID,
	VDIM_UNKNOWN,
	VDIM_DEFAULT,
	VDIM_AUTO=VDIM_DEFAULT,		// content specific
	VDIM_FILM,						// weaving
	VDIM_VIDEO,						// bobbing
	VDIM_NUMBER
	};


enum MPEGVideoSourceDisplayFormat
	{
	MP2VSDF_INVALID,
	MP2VSDF_UNKNOWN,
	MP2VSDF_FULL,				// Source material is encoded full size (4:3 or 16:9)
	MP2VSDF_LETTERBOXED,		// Source material is encoded letterboxed (only possible for 4:3)
	MP2VSDF_NUMBER
	};


enum VideoTestPattern
	{
	VTESTPATT_INVALID,
	VTESTPATT_UNKNOWN,
	VTESTPATT_DISABLED,		//Video test pattern disabled
	VTESTPATT_1_ENABLED,		//Video test pattern 1 enabled
	VTESTPATT_2_ENABLED,		//Video test pattern 2 enabled
	VTESTPATT_3_ENABLED,		//Video test pattern 3 enabled
	VTESTPATT_NUMBER
	};


enum ComponentOutputMode
	{
	//VCOMPOM_INVALID,
	//VCOMPOM_UNKOWN,
	VCOMPOM_DEFAULT,	// let the Video Encoder implementation decide
	VCOMPOM_DISABLE,	// force to no output
	VCOMPOM_YUV,
	VCOMPOM_RGB,
	VCOMPOM_CVBS_COLORKILL
	//VCOMPOM_NUMBER
	};

enum DigitalOutputMode
	{
	VDIGOM_DEFAULT,	// let the Video Encoder implementation decide
	VDIGOM_DISABLE,	// force to no output
	VDIGOM_ENABLE	
	};


enum ScartOutputMode
	{
	//SCOM_INVALID,
	//SCOM_UNKNOWN,
	SCOM_DISABLE,						// SCART output disabled (if hardware supports it)
	SCOM_DEFAULT,
	SCOM_COMPOSITE=SCOM_DEFAULT,	// Composite output
	SCOM_SVIDEO,						// YC out
	SCOM_RGB								// RGB out
	//SCOM_NUMBER
	};


enum BlackLevelPedestal
	{
	//BLK_LEVEL_PEDESTAL_INVALID,
	//BLK_LEVEL_PEDESTAL_UNKNOWN,
	BLK_LEVEL_PEDESTAL_DEFAULT,
	BLK_LEVEL_PEDESTAL_ENABLE,
	BLK_LEVEL_PEDESTAL_DISABLE
	//BLK_LEVEL_PEDESTAL_NUMBER
	};


enum SVideoWidescreenSignalingMode
	{
	//SVIDEO_WIDESCREEN_SIGNAL_MODE_INVALID,
	//SVIDEO_WIDESCREEN_SIGNAL_MODE_UNKNOWN,
	SVIDEO_WIDESCREEN_SIGNAL_MODE_OFF = 0,  // widescreen and normal only
	SVIDEO_WIDESCREEN_SIGNAL_MODE_S1 = 1,  // widescreen and normal only
	SVIDEO_WIDESCREEN_SIGNAL_MODE_S2 = 2  // widescreen, letterboxed, and normal
	//SVIDEO_WIDESCREEN_SIGNAL_MODE_NUMBER
	};


////////////////////////////////////////////////////////////////////
// Tag type definition.
////////////////////////////////////////////////////////////////////

// TAG IDs created by the VDR ID value manager
static const VDRTID VDRTID_VIDEO_DISPLAY = 0x00028000;

//
/// Video Display TAGs
///{

/// @brief Size and frame rate of video standard on the output. This depends on what standard can
///		  TV set/monitor accept. Combined with 50HZ_VIDEOSTANDARD and 60HZ_VIDEOSTANDARD below,
///		  this determines exact video standard that will appear on the output connector (e.g. PAL 60).
MKTAG(VDISP_OUTPUT_VIDEOSTANDARD,		VDRTID_VIDEO_DISPLAY, 0x002, MovingImageStandard)

/// Video standard to be used when 50 Hz output standard is set by OUTPUT_VIDEOSTANDARD
MKTAG(VDISP_50HZ_VIDEOSTANDARD,			VDRTID_VIDEO_DISPLAY, 0x003, NormVideoStandard)

/// Video standard to be used when 60 Hz output standard is set by OUTPUT_VIDEOSTANDARD
MKTAG(VDISP_60HZ_VIDEOSTANDARD,			VDRTID_VIDEO_DISPLAY, 0x004, NormVideoStandard)

/// Video deinterlace mode to be used if progressive mode is on (set by PROSCAN_MODE)
MKTAG(VDISP_DEINTERLACE_MODE,				VDRTID_VIDEO_DISPLAY, 0x005, VideoDeinterlaceMode)

/// Turn progressive output mode on/off
MKTAG(VDISP_PROSCAN_MODE,					VDRTID_VIDEO_DISPLAY, 0x006, VideoProscanMode)

/// Display format as reported by DVD content
MKTAG(MPEG_SOURCE_DISPLAY_FORMAT,		VDRTID_VIDEO_DISPLAY, 0x007, MPEGVideoSourceDisplayFormat)

/// Mpeg presentation mode required by user
MKTAG(MPEG_PRESENTATION_MODE,				VDRTID_VIDEO_DISPLAY, 0x008, MPEGPresentationMode)

/// Enable/disable different test patterns
MKTAG(VDISP_TEST_PATTERN,					VDRTID_VIDEO_DISPLAY, 0x009, VideoTestPattern)

//
// Video Output - DENC/PDENC Encoder Tags
//

///
MKTAG (VDISP_CHROMA_FILTER,				VDRTID_VIDEO_DISPLAY, 0x00a, uint32)

///
MKTAG (VDISP_SVIDEO_ACTIVE,				VDRTID_VIDEO_DISPLAY, 0x00b, bool)

/// Selects which type of component output to use
MKTAG (VDISP_COMPONENT_OUTPUT,			VDRTID_VIDEO_DISPLAY, 0x00c, ComponentOutputMode)

///
MKTAG (VDISP_SCART_OUTPUT,					VDRTID_VIDEO_DISPLAY, 0x00d, ScartOutputMode)


/// Used for CGMS information
///{

///
MKTAG (VDISP_WSS_ASPECT,					VDRTID_VIDEO_DISPLAY, 0x00e, VideoPresentationMode)

///
MKTAG (VDISP_COPY_MODE,						VDRTID_VIDEO_DISPLAY, 0x00f, VideoCopyMode)

///
MKTAG (VDISP_COPY_PROTECTION,				VDRTID_VIDEO_DISPLAY, 0x010, VideoCopyProtection)

///
MKTAG (VDISP_COPYRIGHT,						VDRTID_VIDEO_DISPLAY, 0x011, VideoCopyrightMode)

///
MKTAG (VDISP_ANALOGUE_SOURCE_MODE,		VDRTID_VIDEO_DISPLAY, 0x012, VideoAnalogueSourceMode)

///
MKTAG (VDISP_FILM_CAMERA_MODE,			VDRTID_VIDEO_DISPLAY, 0x013, VideoFilmCameraMode)

///
MKTAG (VDISP_SUBTITLING_MODE,				VDRTID_VIDEO_DISPLAY, 0x014, VideoSubTitlingMode)

///
MKTAG (VDISP_CGMS_INSERTION,				VDRTID_VIDEO_DISPLAY, 0x015, bool)
///}


/// The range for TAGs below is from 0 to 10000. 5000 is neutral (default) setting
///{

///
MKTAG (VDISP_BRIGHTNESS,					VDRTID_VIDEO_DISPLAY, 0x016, uint16)

///
MKTAG (VDISP_CONTRAST,						VDRTID_VIDEO_DISPLAY, 0x017, uint16)

///
MKTAG (VDISP_SATURATION,					VDRTID_VIDEO_DISPLAY, 0x018, uint16)

///
MKTAG (VDISP_HUE,								VDRTID_VIDEO_DISPLAY, 0x019, uint16)
///}

///
MKTAG (VDISP_SVIDEO_WIDESCREEN_SIGNALING_MODE,   VDRTID_VIDEO_DISPLAY, 0x020, SVideoWidescreenSignalingMode)

///
MKTAG (VDISP_CLOSED_CAPTION_INSERTION,	VDRTID_VIDEO_DISPLAY, 0x021, bool)

/// Used for setting Black Level, VBI or 7.5 IRE
MKTAG (VDISP_BLACK_LEVEL,				VDRTID_VIDEO_DISPLAY, 0x022, BlackLevelPedestal)

/// Selects which type of component output to use
MKTAG (VDISP_DIGITAL_OUTPUT,			VDRTID_VIDEO_DISPLAY, 0x023, DigitalOutputMode)

MKTAG (VDISP_BLANK_SCREEN,				VDRTID_VIDEO_DISPLAY, 0x024, bool)
///}


#endif // VDRVIDEOTYPES_H
