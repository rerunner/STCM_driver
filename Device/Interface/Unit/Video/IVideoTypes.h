///
///	@brief	Video decode & display private data types, definitions and simple helper classes
///

/*!
	Note: every video type should have the first two entrys 'invalid' and 'unknown'.
	In case there is no need for special numbering of the enum,
	the last entry should be 'number'. This system can replace the 'min'
	and 'max' entrys with the need for renumbering.
	Asking for 'number' will automatically give us the number of entries
	including 'invalid' and 'unknown'.
*/

#ifndef VIDEOTYPES_H
#define VIDEOTYPES_H


#include "STF/Interface/Types/STFBasicTypes.h"
#include "VDR/Source/Construction/IUnitConstruction.h"
#include "VDR/Interface/Unit/Video/Display/IVDRVideoTypes.h"
#include "Device/Interface/Unit/Video/IVideoDisplayTypes.h"


/////////////////////////////////////////////////////////////////////////////
//	Video Types
/////////////////////////////////////////////////////////////////////////////

struct VideoStandardComponents
	{
	MovingImageStandard	movingImageStandard;
	ColorStandard			colorStandard;
	uint32					colorCarrierFrequency;
	uint32					colorFilterBandwidth;
	bool						colorCarrierTrap;		// filters the luma to avoid cross color
	bool						blackLevelPedestal;	// TRUE = 7.5IRE offset
	};

/*!
	Be sure to use the following set of Video Standard & SubStandard
	only for communication with the application component.
	This division of the complete video standard is not good
	for technical use and outdated.
	Instead use VideoMovingImageStandard & the additional
	encoding attributes or the complete NormVideoStandard.
*/

enum VideoFormat
	{
	VFMT_INVALID,
	VFMT_UNKNOWN,
	VFMT_YUV_422,
	VFMT_YUV_411,
	VFMT_YUV_420,
	VFMT_YUV_444,
	VFMT_RGB_444,
	VFMT_NUMBER
	};

enum VideoBusFormat
	{
	VBFM_INVALID,
	VBFM_UNKNOWN,
	VBFM_YC16 = 0,
	VBFM_YC8  = 1,
	VBFM_NUMBER
	};

enum VideoFieldTimingMode
	{
	VFTM_INVALID,
	VFTM_UNKNOWN,
	VFTM_PROGRESSIVE,
	VFTM_INTERLACED,
	VFTM_NUMBER
	};

enum VideoField
	{
	VFLD_INVALID,
	VFLD_UNKNOWN,
	VFLD_SINGLE ,
	VFLD_ODD,
	VFLD_EVEN,
	VFLD_FRAME,
	VFLD_NUMBER
	};
	
enum VideoMuxSource
	{
	VMUXSRC_INVALID,
	VMUXSRC_UNKNOWN,
	VMUXSRC_NONE,
	VMUXSRC_INTERNAL,
	VMUXSRC_EXTERNAL,
	VMUXSRC_NUMBER
	};
	
enum VideoPixClockMode
	{
	VPIXCLK_INVALID,
	VPIXCLK_UNKNOWN,
	VPIXCLK_INTERNAL,
	VPIXCLK_EXTERNAL,
	VPIXCLK_NUMBER
	};

// The values of the following two enums are hardware-independent.

enum GrabFormat
	{
	GRB_INVALID,
	GRB_UNKNOWN,
	GRB_MJPEG,
	GRB_RGB_888x,
	GRB_RGB_888,
	GRB_RGB_565,
	GRB_RGB_555,
	GRB_YUV_422,
	GRB_NUMBER
	};

enum PIPFormat
	{
	PIP_INVALID,
	PIP_UNKNOWN,
	PIP_RGB_888x,
	PIP_RGB_888,
	PIP_RGB_565,
	PIP_RGB_555,
	PIP_YUV_422, 
	PIP_UVY_422,
	PIP_PALETTE_8,
	PIP_NUMBER
	};

/*
	This is an enum with talking names.
	It specifies how video frame data is placed in memory.
	The format of the entry names is:
	Number, Datatype _ Relationship _ Number, Datatype ...

	Datatypes:
	Y  - Lumabyte
	CB - Cb value byte
	CR - Cr value byte

	Relationship:
	I - Interleaved
	S - Separated
*/

enum LumaChromaStorageFormat
	{
	LCSF_INVALID,
	LCSF_UNKNOWN,
	LCSF_1Y_I_1CB_I_1CR, 		// 4:4:4, Y/Cb/Cr interleaved
	LCSF_2Y_I_1CB_I_1CR,			// 4:2:2, Y/Cb/Cr interleaved
	LCSF_1Y_I_1CB_I_1Y_I_1CR,	// 4:2:2, Y/Cb/Cr interleaved
	LCSF_4Y_I_1CB_I_1CR,			// 4:2:0, Y/Cb/Cr interleaved
	LCSF_1Y_S_1CB_I_1CR,			// 4:4:4, Y separated, Cb/Cr interleaved
	LCSF_2Y_S_1CB_I_1CR,			// 4:2:2, Y separated, Cb/Cr interleaved
	LCSF_4Y_S_1CB_I_1CR,			// 4:2:0, Y separated, Cb/Cr interleaved
	LCSF_16Y_S_4CB_I_4CR,		// 4:2:0, Y separated, Cb/Cr interleaved
	LCSF_1Y_S_1CB_S_1CR,			// 4:4:4, all planes separated
	LCSF_2Y_S_1CB_S_1CR,			// 4:2:2, all planes separated
	LCSF_4Y_S_1CB_S_1CR,			// 4:2:0, all planes separated
	LCSF_NUMBER
	};


enum PresentationAspectRatio
	{
	PAR_INVALID,
	PAR_UNKNOWN,
	PAR_FULLFRAME,
	PAR_FORCED4BY3,
	PAR_FORCED16BY9,
	PAR_NUMBER
	};


/////////////////////////////////////////////////////////////////////////////
//	Video Frame Tags
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_VIDEO_FRAME = 0x00027000;

MKTAG(STORAGE_FORMAT,				VDRTID_VIDEO_FRAME,  0x01,  LumaChromaStorageFormat)
MKTAG(SCAN_LAYOUT,					VDRTID_VIDEO_FRAME,  0x08,  VideoField)
MKTAG(SCAN_MODE,						VDRTID_VIDEO_FRAME,  0x09,  VideoFieldTimingMode)
MKTAG(SRC_RECT16_LEFT,				VDRTID_VIDEO_FRAME,  0x0b,  uint32)
MKTAG(SRC_RECT16_RIGHT,				VDRTID_VIDEO_FRAME,  0x0c,  uint32)
MKTAG(SRC_RECT16_TOP,				VDRTID_VIDEO_FRAME,  0x0d,  uint32)
MKTAG(SRC_RECT16_BOTTOM,			VDRTID_VIDEO_FRAME,  0x0e,  uint32)
MKTAG(DEST_RECT16_LEFT,				VDRTID_VIDEO_FRAME,  0x0f,  uint32)
MKTAG(DEST_RECT16_RIGHT,			VDRTID_VIDEO_FRAME,  0x10,  uint32)
MKTAG(DEST_RECT16_TOP,				VDRTID_VIDEO_FRAME,  0x11,  uint32)
MKTAG(DEST_RECT16_BOTTOM,			VDRTID_VIDEO_FRAME,  0x12,  uint32)
MKTAG(COLOR_FORMAT,					VDRTID_VIDEO_FRAME,  0x13,  VDRGfxColorFormat)

#endif
