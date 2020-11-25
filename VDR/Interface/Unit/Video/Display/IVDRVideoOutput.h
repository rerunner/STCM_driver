///
/// @brief      Video Output (Encoder) interface description
///

#ifndef IVDRVIDEOOUTPUT_H
#define IVDRVIDEOOUTPUT_H

#include "VDR/Source/Construction/IUnitConstruction.h"
#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Unit/Video/Display/IVDRVideoTypes.h"



/*
////////////////////////////////////////////////////////////////////
// Tag type definition.
////////////////////////////////////////////////////////////////////

// TAG IDs created by the VDR ID value manager
static const VDRTID VDRTID_ENC = 0x0008d000;  // Check if ID is OK !!!


// Video Output Encoder Tags
//

MKTAG (ENC_CHROMA_FILTER,					VDRTID_ENC, 0x008, uint32)
MKTAG (ENC_SVIDEO_ACTIVE,					VDRTID_ENC, 0x009, bool)

MKTAG (ENC_SET_PRESENTATION_MODE,		VDRTID_ENC, 0x00b, VideoPresentationMode)
MKTAG (ENC_VIDEOSTANDARD,					VDRTID_ENC, 0x00c, MovingImageStandard)
MKTAG (ENC_OUTPUT_VIDEOSTANDARD,			VDRTID_ENC, 0x00d, MovingImageStandard)
MKTAG (ENC_PAL_VIDEOSUBSTANDARD,       VDRTID_ENC, 0x00e, PALVideoSubStandard)
MKTAG (ENC_NTSC_VIDEOSUBSTANDARD,      VDRTID_ENC, 0x00f, NTSCVideoSubStandard)

// Selects which type of component output to use
MKTAG (ENC_COMPONENT_OUTPUT,				VDRTID_ENC, 0x020, ComponentOutputMode)


MKTAG (ENC_SCART_OUTPUT,					VDRTID_ENC, 0x021, ScartOutputMode)

// Used for CGMS information
MKTAG (ENC_WSS_ASPECT,						VDRTID_ENC, 0x024, VideoPresentationMode)
MKTAG (ENC_COPY_MODE,						VDRTID_ENC, 0x025, VideoCopyMode)
MKTAG (ENC_COPY_PROTECTION,				VDRTID_ENC, 0x026, int)
MKTAG (ENC_COPYRIGHT,						VDRTID_ENC, 0x027, VideoCopyrightMode)
MKTAG (ENC_ANALOGUE_SOURCE_MODE,			VDRTID_ENC, 0x028, VideoAnalogueSourceMode)
MKTAG (ENC_FILM_CAMERA_MODE,				VDRTID_ENC, 0x029, VideoFilmCameraMode)
MKTAG (ENC_SUBTITLING_MODE,				VDRTID_ENC, 0x02A, VideoSubTitlingMode)
MKTAG (ENC_CGMS_INSERTION,				VDRTID_ENC, 0x02B, bool)

// Set the following from 0 to 10000. 5000 is neutral (default) setting
MKTAG (ENC_BRIGHTNESS,						VDRTID_ENC, 0x030, uint16)
MKTAG (ENC_CONTRAST,							VDRTID_ENC, 0x031, uint16)
MKTAG (ENC_SATURATION,						VDRTID_ENC, 0x032, uint16)
MKTAG (ENC_HUE,								VDRTID_ENC, 0x033, uint16)

// MKTAG (ENC_WIDTH,								VDRTID_ENC, 0x034, uint32)
// MKTAG (ENC_HEIGHT,							VDRTID_ENC, 0x035, uint32)
// MKTAG (ENC_FILTER,							VDRTID_ENC, 0x036, uint16)


MKTAG (ENC_SVIDEO_WIDESCREEN_SIGNALING_MODE,   VDRTID_ENC, 0x037, SVideoWidescreenSignalingMode)
MKTAG (ENC_CLOSED_CAPTION_INSERTION,           VDRTID_ENC, 0x038, bool)


// Used for setting Proscan Mode
MKTAG (ENC_PROSCAN_MODE,				VDRTID_ENC, 0x039, VideoProscanMode)
//

// Used for setting Black Level, VBI or 7.5 IRE
MKTAG (ENC_BLACK_LEVEL,				VDRTID_ENC, 0x03A, BlackLevelPedestal)
//
*/
//! External "Video Output - Encoder" Interface ID
static const VDRIID VDRIID_ENC = 0x00009999;   // Check if ID is OK

/*!
	The interface provides functions for a Video Output. 
	
*/
static const VDRIID VDRIID_VIDEO_OUTPUT = 0x00000090;


class IVDRVideoOutput : public virtual IVDRBase
	{
	// Only tags are supported
	public:
	};


// Global Video Output Creation function definition
DECLARE_UNIT_CREATION_FUNCTION(CreateVideoOutput)

#endif // IVIDEOOUTPUT_H
