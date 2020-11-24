///
/// @brief      Types and tags used by Video Decoder Units
///

#ifndef IVDRVIDEODECODERTYPES_H
#define IVDRVIDEODECODERTYPES_H

#include "VDR/Interface/Unit/IVDRTags.h"
#include "VDR/Interface/Base/IVDRMessage.h"


/////////////////////////////////////////////////////////////////////////////
// General Video Decoder Unit Tags and their types
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
//
// MPEG Video Decoder Properties
//

static const VDRTID VDRTID_MPEG_VIDEO_DECODER_PROPERTIES = 0x00029000;


enum SourceAspectRatio
	{
	SAR_INVALID,
	SAR_UNKNOWN,
	SAR_DEFAULT,
	SAR_FORCED4BY3,
	SAR_FORCED16BY9,
	SAR_NUMBER
	};


enum MPEGCodingStandard
	{ 
	MPCS_INVALID,
	MPCS_UNKNOWN,
	MPCS_MPEG1,
	MPCS_MPEG2,
	MPCS_MPEG4,
	MPCS_NUMBER
	};
	

// use this struct if the Sequence header
// and the Sequence Extension data is combined
struct SequenceHeaderExtension	// MPEG 1/2
	{
	uint16 horizontalSize;
	uint16 verticalSize;
	uint8  aspectRatioInformation;
	uint8  frameRateCode;
	uint32 bitRate;
	uint32 vbvBufferSize;
	bool   constrainedParametersFlag;	// MPEG 1 flag
	uint8  profileAndLevelIndication;
	bool   progressiveSequence;
	uint8  chromaFormat;
	bool   lowDelay;
	uint8  frameRateExtensionN;
	uint8  frameRateExtensionD;
	};

struct Mpeg4FrameInfo	
	{
	uint16 frameWidth;
	uint16 frameHeight;
	uint16  frameRate;
	uint16 codecID;
	};

struct ColorRGB
	{
	uint8 red;
	uint8 green;
	uint8 blue;
	};

struct ColorSubPxl
	{
	ColorRGB background;
	ColorRGB pattern;
	ColorRGB emphasis1;
	ColorRGB emphasis2;
	};

//! Active MPEG video channel number for use in Stream Demultiplexers
MKTAG (MPEG_VIDEO_STREAM_ID,					VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x001, uint32)

//! Force frame aspect ratio to overwrite wrong info in stream
MKTAG (MPEG_FORCED_SOURCE_ASPECT_RATIO,	VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x002, SourceAspectRatio)

//! 7 TAGs to support resume after stop (GET and SET):

//! Stream follows MPEG 1 or 2 syntax, needed for combined MPEG1/2 Stream Demultiplexer
MKTAG (PACK_HEADER_MPEG_STANDARD,			VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x003, MPEGCodingStandard)

//! Group number with which we have to resume, for the sequence parameters to be valid
MKTAG (RESUME_GROUP_NUMBER,					VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x004, uint16)

//! Parameters of currently delivered MPEG Video sequence
MKTAG (MPEG_VIDEO_SEQUENCE_PARAMETERS,		VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x005, SequenceHeaderExtension*)

//! Content of Quantizer tables
MKTAG (INTRA_QTABLE,								VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x006, uint8*)
MKTAG (NON_INTRA_QTABLE,						VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x007, uint8*)
MKTAG (CHROMA_INTRA_QTABLE,					VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x008, uint8*)
MKTAG (CHROMA_NON_INTRA_QTABLE,				VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x009, uint8*)

//! Thresholds set by application for error concealment control (decision to show or hide frame)
MKTAG (VIDEO_B_FRAME_ERROR_TRESHOLD,		VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x00a, uint32)
MKTAG (VIDEO_IP_FRAME_ERROR_TRESHOLD,		VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x00b, uint32)

//! DIVX/MPEG4 Video Parameters
MKRTG (MPEG4_VIDEO_PARAMETERS,				VDRTID_MPEG_VIDEO_DECODER_PROPERTIES ,	0x020, Mpeg4FrameInfo)


/////////////////////////////////////////////////////////////////////////////
//
// Subpicture Decoder Properties
//

static const VDRTID VDRTID_SUBPICTURE_DECODER_PROPERTIES = 0x0002e000;

//! Active subpicture channel number for use in Stream Demultiplexers
MKTAG (SUBPICTURE_STREAM_ID,					VDRTID_SUBPICTURE_DECODER_PROPERTIES ,	0x001, uint32)
MKTAG (MPEG_SPU_PALETTE_ENTRY,				VDRTID_SUBPICTURE_DECODER_PROPERTIES , 0x002, uint32)
MKRTG (MPEG4_SUBTITLE_COLOR,					VDRTID_SUBPICTURE_DECODER_PROPERTIES ,	0x003, ColorSubPxl)


/////////////////////////////////////////////////////////////////////////////
//
//	All Video Decoder Notification Messages
//

static const VDRMID VDRMID_VIDEO_DECODE_ERRORS_OCCURRED = 0x00077000;	// param1 may contain the error quantity


#endif // #ifndef IVDRVIDEODECODERTYPES_H
