#ifndef IVDRSTREAMTYPES_H
#define IVDRSTREAMTYPES_H

///
/// @brief VDR General Stream Tags and Types
///

#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Unit/IVDRTags.h"


/////////////////////////////////////////////////////////////////////////////
//	Tags and their Types for all kinds of streams
/////////////////////////////////////////////////////////////////////////////

static const VDRTID VDRTID_STREAM_PROPERTY = 0x00023000;


/////////////////////////////////////////////////////////////////////////////

// Endianess

enum VDRStreamEndianess
	{
	VDR_STREAM_ENDIANESS_LITTLE,
	VDR_STREAM_ENDIANESS_BIG
	};

MKTAG (STREAM_ENDIANESS, VDRTID_STREAM_PROPERTY, 0x001, VDRStreamEndianess)


/////////////////////////////////////////////////////////////////////////////

// Copyright Information

enum VDRStreamCopyrightInfo
	{
	VDR_STREAM_COPYRIGHT_FREE,
	VDR_STREAM_COPYRIGHT_ASSERTED
	};

MKTAG (STREAM_COPYRIGHT_INFO, VDRTID_STREAM_PROPERTY, 0x002, VDRStreamCopyrightInfo)


/////////////////////////////////////////////////////////////////////////////

// Original Information

enum VDRStreamOriginalInfo
	{
	VDR_STREAM_COPY,		// Stream is a copy
	VDR_STREAM_ORIGINAL	// Stream is original
	};

MKTAG (STREAM_ORIGINAL_INFO, VDRTID_STREAM_PROPERTY, 0x003, VDRStreamOriginalInfo)


/////////////////////////////////////////////////////////////////////////////

// Generation Information - number indicates nth generation

MKTAG (STREAM_COPY_GENERATION_INFO, VDRTID_STREAM_PROPERTY, 0x004, uint32)

// Bitrate information (unit is "bits per second")

MKTAG (STREAM_BITRATE, VDRTID_STREAM_PROPERTY, 0x005, uint32)
MKTAG (STREAM_BITRATE_GROUP2, VDRTID_STREAM_PROPERTY, 0x006, uint32)


/////////////////////////////////////////////////////////////////////////////

// Support for stream-synchronized immediate action in the output stage

MKTAG (DIRECT_OUTPUT_CONTROL, VDRTID_STREAM_PROPERTY, 0x007, uint32)	// flagfield & ID


/////////////////////////////////////////////////////////////////////////////

// Support for decompressed framebuffer retreat by application

MKTAG (DESTINATION_BUFFER_FOR_MAIN_VIDEO_DECODER_ONLY,	VDRTID_STREAM_PROPERTY, 0x008, uint32)
MKTAG (TARGET_FRAME_FOR_MAIN_VIDEO_DECODER_ONLY,			VDRTID_STREAM_PROPERTY, 0x009, uint32)


#endif
