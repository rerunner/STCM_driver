///
/// @brief Specific MPEG coding types
///

/*!
	Note: every enum - except the MPEG stream types - should have
	the first two entrys 'invalid' and 'unknown'.
	In case there is no need for special numbering of the enum,
	the last entry should be 'number'.
	Asking for 'number' will automatically give us the number of entries
	including 'invalid' and 'unknown'.
*/

#ifndef MPEGTYPES_H
#define MPEGTYPES_H

#include "VDR/Source/Construction/IUnitConstruction.h"
#include "VDR/Interface/Unit/Video/Decoder/IVDRVideoDecoderTypes.h"


/////////////////////////////////////////////////////////////////////////////
//	MPEG Video Stream Tags
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_MPEG_VIDEO = 0x00026000;

//MKTAG(DECODING_TIME_STAMP,  VDRTID_MPEG_VIDEO,  0x01,  STFInt64)


/////////////////////////////////////////////////////////////////////////////
//	MPEG Video Types
/////////////////////////////////////////////////////////////////////////////

//
// potential MPEG video stream header
//
enum MPEGVideoHeader
	{
	MPVH_INVALID,
	MPVH_UNKNOWN,
	MPVH_SYSTEM_START_CODE, // 2
	MPVH_SEQUENCE_HEADER,   // 3
	MPVH_GROUP_START,       // 4
	MPVH_PICTURE_START,     // 5
	MPVH_SLICE_HEADER,      // 6
	MPVH_EXTENSION_START,   // 7
	MPVH_USER_DATA,         // 8
	MPVH_STUFFING,          // 9
	MPVH_SEQUENCE_ERROR,    // 10
	MPVH_SEQUENCE_END,      // 11
	MPVH_NUMBER
	};

enum DIVXVideoHeader
	{
	DIVH_INVALID,
	DIVH_UNKNOWN,
	DIVH_VIDEO_OBJECT_START_CODE, // 2
	DIVH_VIDEO_OBJECT_LAYER,   // 3
	DIVH_USER_DATA,         // 4
	DIVH_VOP,          // 5
	DIVH_NUMBER
	};

//
// potential MPEG coding frame types
//

enum DIVXFrameType
	{
	DIFT_INVALID,
	DIFT_UNKNOWN, // picture_coding_type bits:
	DIFT_IFRAME,  // 00
	DIFT_PFRAME,  // 01
	DIFT_BFRAME,  // 10
	DIFT_SFRAME,  // 11
	DIFT_NUMBER
	};         

enum DIVXFrameStructure
	{
	DIFS_INVALID,
	DIFS_UNKNOWN,     // picture_structure bits:
	DIFS_TOPFIELD,    // 01
	DIFS_BOTTOMFIELD, // 10
	DIFS_FRAME,       // 11
	DIFS_NUMBER
	};

enum MPEGFrameType
	{
	MPFT_INVALID,
	MPFT_UNKNOWN,
	MPFT_IFRAME,
	MPFT_PFRAME,
	MPFT_BFRAME,
	MPFT_DFRAME,
	MPFT_NUMBER
	};

enum MPEGPictureCodingType // field picture_coding_type
	{
	MPCT_FORBIDDEN      = 0,
	MPCT_INTRA_CODED    = 1,  // 001
	MPCT_PREDICTIVE     = 2,  // 010
	MPCT_BIDIREC_PRED   = 3,  // 011
	MPCT_DC_INTRA_CODED = 4,  // 100 - MPEG 1 only type
	MPCT_RESERVED1      = 5,
	MPCT_RESERVED2      = 6,
	MPCT_RESERVED3      = 7
	};

enum MPEGFrameStructure
	{
	MPFS_INVALID,
	MPFS_UNKNOWN,
	MPFS_TOPFIELD,
	MPFS_BOTTOMFIELD,
	MPFS_FRAME,
	MPFS_NUMBER
	};

enum MPEGPictureStructure
	{
	MPS_RESERVED    = 0, // 00
	MPS_TOPFIELD    = 1, // 01
	MPS_BOTTOMFIELD = 2, // 10
	MPS_FRAME       = 3  // 11
	};

enum MPEGTemporalPosition
	{
	MTP_INVALID,
	MTP_UNKNOWN,
	MTP_INITIALFIELD,
	MTP_SECONDARYFIELD,
	MTP_FRAME,
	MTP_NUMBER
	};

//
// potential MPEG chroma formats, typically only 420 used
//
enum MPEGChromaType
	{ 
	MPCT_INVALID,
	MPCT_UNKNOWN, 
	MPCT_RESERVED, 
	MPCT_420, 
	MPCT_422, 
	MPCT_444, 
	MPCT_NUMBER
	};

enum MPEGChromaFormat // field chroma_format
	{ 
	MCF_RESERVED = 0, // 00
	MCF_420      = 1, // 01
	MCF_422      = 2, // 10
	MCF_444      = 3  // 11
	};

//
// video output formats, intended by stream
//
enum MPEGVideoFormat	// field video_format
	{ 
	MVF_COMPONENT   = 0, // 000
	MVF_PAL         = 1, // 001
	MVF_NTSC        = 2, // 010
	MVF_SECAM       = 3, // 011
	MVF_MAC         = 4, // 100
	MVF_UNSPECIFIED = 5, // 101
	MVF_RESERVED1   = 6, // 110
	MVF_RESERVED2   = 7  // 111
	};                


///////////////////////////////
// MPEG Video header structs
///////////////////////////////


struct SequenceHeader				// MPEG 1, 2
	{
	uint16 horizontalSizeValue;
	uint16 verticalSizeValue;
	uint8  aspectRatioInformation;
	uint8  frameRateCode;
	uint32 bitRateValue;
	uint16 vbvBufferSizeValue;
	bool   constrainedParametersFlag;	// MPEG 1 flag
	};

// Note: the 'load' flags contained in the 'Sequence header'
// and the 'Quant matrix extension' are reported seperately
// so that the changed Quantiser Matrix can be accessed seperately.


struct SequenceExtension			// MPEG 2
	{
	uint8            profileAndLevelIndication;
	bool             progressiveSequence;
	MPEGChromaFormat chromaFormat;
	uint8            horizontalSizeExtension;
	uint8            verticalSizeExtension;
	uint16           bitRateExtension;
	uint8            vbvBufferSizeExtension;
	bool             lowDelay;
	uint8            frameRateExtensionN;
	uint8            frameRateExtensionD;
	};

// Note: SequenceHeaderExtension is defined in IVDRVideoDecoderTypes.h
//       because we need to exchange this info with the upper layer for 'power off resume'


struct SequenceDisplayExtension	// MPEG 2
	{
	MPEGVideoFormat videoFormat;
	bool            colourDescription;
	uint8           colourPrimaries;
	uint8           transferCharacteristics;
	uint8           matrixCoefficients;
	uint16          displayHorizontalSize;
	uint16          displayVerticalSize;
	};


struct SequenceScalableExtension	// MPEG 2
	{
	// not supported
	};


struct GroupOfPicturesHeader		// MPEG 1, 2
	{
	uint32 timeCode;
	bool   closedGop;
	bool   brokenLink;
	};


struct PictureHeader					// MPEG 1, 2
	{
	uint16                temporalReference;
	MPEGPictureCodingType pictureCodingType;
	uint16                vbvDelay;
	bool                  fullPelForwardVector;
	uint8                 forwardFCode;
	bool                  fullPelBackwardVector;
	uint8                 backwardFCode;
	};


struct PictureCodingExtension		// MPEG 2
	{
	uint8                 fCodeForwardHorizontal;
	uint8                 fCodeForwardVertical;
	uint8                 fCodeBackwardHorizontal;
	uint8                 fCodeBackwardVertical;
	uint8                 intraDcPrecision;
	MPEGPictureStructure  pictureStructure;
	MPEGTemporalPosition  temporalPosition;	// this value is calculated by the parser
	bool                  topFieldFirst;
	bool                  framePredFrameDct;
	bool                  concealmentMotionVectors;
	bool                  qScaleType;
	bool                  intraVlcFormat;
	bool                  alternateScan;
	bool                  repeatFirstField;
	bool                  chroma420Type;
	bool                  progressiveFrame;
	bool                  compositeDisplayFlag;
	bool                  vAxis;
	uint8                 fieldSequence;
	bool                  subCarrier;
	uint8                 burstAmplitude;
	uint8                 subCarrierPhase;
	};


struct PictureDisplayExtension	// MPEG 2
	{
	uint8  validNumber;	// 1, 2 or 3 of the following pairs are valid
	uint16 frameCentreHorizontalOffset[3];		// Note: centre is british english and has the same meaning like center in the US
	uint16 frameCentreVerticalOffset[3];
	};


struct PictureTemporalScalableExtension	// MPEG 2
	{
	// not supported
	};


struct PictureSpatialScalableExtension		// MPEG 2
	{
	// not supported
	};


struct CopyrightExtension			// MPEG 2
	{
	// not supported
	};


struct ClosedCaptioning
	{
	bool		topFieldFlag;
	uint8		validBytes;
	uint8		buffer[6];	// 2 bytes per field
	};


struct UserData			// MPEG 2
	{
	ClosedCaptioning		closedCaptioning;
	};


/////////////////////////////
// MPEG parser defines
/////////////////////////////
//
// Start code definitions, all MPEG start codes are of the
// form 00 00 01 xx where xx is one of the following
// constants.
// The names follow the naming convention of
// ISO/IEC 11172 (MPEG 1) and ISO/IEC 13818 (MPEG 2).
//
//   *** MPEG 1/2 Video start code IDs ***
#define PICTURE_START_CODE          0x00
//                                  0x01 - 0xAF: slice start codes

//                                  0xB0: reserved
//                                  0xB1: reserved
#define USER_DATA_START_CODE        0xB2
#define SEQUENCE_HEADER_CODE        0xB3 // MPEG 1,2 identical header syntax
#define SEQUENCE_ERROR_CODE         0xB4
#define EXTENSION_START_CODE        0xB5
//                                  0xB6: reserved
#define SEQUENCE_END_CODE           0xB7
#define GROUP_START_CODE            0xB8

//   *** MPEG 4 Video start code IDs ***
#define DX_VO_START_CODE            0x00 // 0x00 - 0x1F
#define DX_VOL_START_CODE           0x20 // 0x20 - 0x2F
#define DX_USER_DATA_START_CODE     0xB2
#define DX_VOP_START_CODE           0xB6

//   *** MPEG System start code IDs ***
#define ISO_11172_END_CODE          0xB9
#define PACK_START_CODE             0xBA // MPEG 1 & 2 headers have no 'length' field
#define SYSTEM_HEADER_START_CODE    0xBB // MPEG 1,2 identical header syntax
#define MAP_STREAM_ID               0xBC // program_stream_map code
#define PRIVATE_STREAM_1_ID         0xBD // header follows same PES packet syntax like ISO/IEC 13818-2 video and -3 audio
#define PADDING_STREAM_ID           0xBE
#define PRIVATE_STREAM_2_ID         0xBF

//                                  0xC0 - 0xDF: MPEG 1,2 Audio stream

//                                  0xE0 - 0xEF: MPEG 1,2 Video stream

#define ECM_STREAM_ID               0xF0
#define EMM_STREAM_ID               0xF1
#define DSMCC_STREAM_ID             0xF2
#define ISO_13522_STREAM_ID         0xF3 // header follows same PES packet syntax like ISO/IEC 13818-2 video and -3 audio
#define H_222_1_TYPE_A_STREAM_ID    0xF4
#define H_222_1_TYPE_B_STREAM_ID    0xF5
#define H_222_1_TYPE_C_STREAM_ID    0xF6
#define H_222_1_TYPE_D_STREAM_ID    0xF7
#define H_222_1_TYPE_E_STREAM_ID    0xF8
#define ANCILLARY_STREAM_ID         0xF9 // only used in transport stream
//                                  0xFA: reserved
//                                  0xFB: reserved
//                                  0xFC: reserved
//                                  0xFD: reserved
//                                  0xFE: reserved
#define DIRECTORY_STREAM_ID         0xFF // program_stream_directory code

//
// MPEG 2 extension start code identifier
//
#define SEQUENCE_EXTENSION_ID                      0x1
#define SEQUENCE_DISPLAY_EXTENSION_ID              0x2
#define QUANT_MATRIX_EXTENSION_ID                  0x3
#define COPYRIGHT_EXTENSION_ID                     0x4
#define SEQUENCE_SCALABLE_EXTENSION_ID             0x5

#define PICTURE_DISPLAY_EXTENSION_ID               0x7
#define PICTURE_CODING_EXTENSION_ID                0x8
#define PICTURE_SPATIAL_SCALABLE_EXTENSION_ID      0x9
#define PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID     0xA


//
// MPVX flags determine which header types to parse / were parsed
//
#define MPVX_SEQUENCE_HEADER                       MKFLAG(0)
#define MPVX_GROUP_HEADER                          MKFLAG(1)
#define MPVX_PICTURE_HEADER                        MKFLAG(2)
#define MPVX_SLICE_HEADER                          MKFLAG(3)
#define MPVX_USER_DATA                             MKFLAG(4)
#define MPVX_SEQUENCE_ERROR                        MKFLAG(5)
#define MPVX_SEQUENCE_END                          MKFLAG(6)
#define MPVX_EXTENSION_HEADER                      MKFLAG(7)

// Extension headers
#define MPVX_SEQUENCE_EXTENSION                    MKFLAG(8)
#define MPVX_SEQUENCE_DISPLAY_EXTENSION            MKFLAG(9)	
#define MPVX_SEQUENCE_SCALABLE_EXTENSION           MKFLAG(10)
#define MPVX_PICTURE_CODING_EXTENSION              MKFLAG(11)
#define MPVX_QUANT_MATRIX_EXTENSION                MKFLAG(12)
#define MPVX_PICTURE_DISPLAY_EXTENSION             MKFLAG(13)
#define MPVX_PICTURE_TEMPORAL_SCALABLE_EXTENSION   MKFLAG(14)
#define MPVX_PICTURE_SPATIAL_SCALABLE_EXTENSION    MKFLAG(15)
#define MPVX_COPYRIGHT_EXTENSION                   MKFLAG(16)

//
// DIVX flags determine which header types to parse / were parsed
//
#define DIVX_VIDEO_OBJECT_START_CODE               MKFLAG(0)
#define DIVX_VIDEO_OBJECT_LAYER							MKFLAG(1)
#define DIVX_USER_DATA										MKFLAG(2)
#define DIVX_VOP												MKFLAG(3)

//
// MPVP flags determine which values really changed
//

// Sequence header, sequence extension
#define MPVPCHANGED_STANDARD                       MKFLAG(0)
#define MPVPCHANGED_FRAME_SIZE                     MKFLAG(1)
#define MPVPCHANGED_ASPECT_RATIO                   MKFLAG(2)
#define MPVPCHANGED_FRAME_RATE                     MKFLAG(3)
#define MPVPCHANGED_BIT_RATE                       MKFLAG(4)
#define MPVPCHANGED_VBV_BUFFER_SIZE                MKFLAG(5)
#define MPVPCHANGED_INTRA_QUANT                    MKFLAG(6)
#define MPVPCHANGED_NON_INTRA_QUANT                MKFLAG(7)
#define MPVPCHANGED_CHROMA_INTRA_QUANT             MKFLAG(8)
#define MPVPCHANGED_CHROMA_NON_INTRA_QUANT         MKFLAG(9)
#define MPVPCHANGED_PROFILE_AND_LEVEL              MKFLAG(10)
#define MPVPCHANGED_CHROMA_FORMAT                  MKFLAG(11)

// Sequence display extension
#define MPVPCHANGED_DISPLAY_SIZE                   MKFLAG(12)

// Picture header
#define MPVPCHANGED_ MKFLAG()
#define MPVPCHANGED_ MKFLAG()

// Picture coding extension
#define MPVPCHANGED_FIELD_POLARITY                 MKFLAG(16)
#define MPVPCHANGED_ MKFLAG()
#define MPVPCHANGED_ MKFLAG()
#define MPVPCHANGED_ MKFLAG()

// Picture display extension
#define MPVPCHANGED_ MKFLAG()
#define MPVPCHANGED_ MKFLAG()


#endif // MPEGTYPES_H
