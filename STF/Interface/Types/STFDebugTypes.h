///
/// @brief OS-dependent debug-method types
///

#ifndef _STFDEBUGTYPES_H
#define _STFDEBUGTYPES_H

#include "STFBasicTypes.h"


typedef enum tagStreamIDs
{
   STRMID_GENERAL				= 0x00,
   STRMID_AUDIO_ENCODER		= 0x01,
   STRMID_AUTHORING			= 0x02,
   STRMID_BLOCK_MANAGER		= 0x03,
   STRMID_BUFFER_MANAGER	= 0x04,
   STRMID_DDP_PLAYER			= 0x05,
   STRMID_DDP_RECORDER		= 0x06,
   STRMID_DISC_LAYOUT		= 0x07,
   STRMID_DLTS_SERVER		= 0x08,
   STRMID_DVD_REC_INT		= 0x09,
   STRMID_DVP					= 0x0a,
   STRMID_FILE_SYSTEM		= 0x0b,
   STRMID_HVF					= 0x0c,
   STRMID_IFP					= 0x0d,
   STRMID_MULTIPLEXER		= 0x0e,
   STRMID_NAVIGATION			= 0x0f,
   STRMID_PICTURE_DATA		= 0x10,
   STRMID_RECORD				= 0x11,
   STRMID_STILL_ENCODER		= 0x12,
   STRMID_THUMB_MANAGER		= 0x13,
   STRMID_VFI					= 0x14,
   STRMID_VIDEO_ENCODER		= 0x15,
   __STRMID_COUNT // always leave as last one.
}
STRMID;

typedef enum tagLevelIDs
{
   STRLID_ALL,
   STRLID_METHOD_NAME,
   STRLID_METHOD_PARAM,
   STRLID_METHOD_IN_OUT,
   STRLID_ERROR,
   STRLID_CRITICAL,
   STRLID_POC,
   // Module Specific Levels
   STRLID_MODULE_01,
   STRLID_MODULE_02,
   STRLID_MODULE_03,
   STRLID_MODULE_04,
   STRLID_MODULE_05,
   STRLID_MODULE_06,
   STRLID_MODULE_07,
   STRLID_MODULE_08,
   STRLID_MODULE_09,
   STRLID_MODULE_10,
   STRLID_MODULE_11,
   STRLID_MODULE_12,
   STRLID_MODULE_13,
   STRLID_MODULE_14,
   STRLID_MODULE_15,
   STRLID_MODULE_16,
   STRLID_MODULE_17,
   STRLID_MODULE_18,
   STRLID_MODULE_19,
   STRLID_MODULE_20,
   STRLID_MODULE_21,
   STRLID_MODULE_22,
   STRLID_MODULE_23,
   STRLID_MODULE_24,
   STRLID_MODULE_25,
   __STRLID_COUNT // always leave as last one.
}
STRLID;

// Point Of Check Definition
typedef struct  
{
	uint16			pocId;				// Unique PoC identifier
	const char	*  longFormat;			// Parameter Format - long version
	const char	*  shortFormat;		// Parameter Format - short version
} pocDescriptor;

const uint16 NOPOC_ID = 0;

#endif

