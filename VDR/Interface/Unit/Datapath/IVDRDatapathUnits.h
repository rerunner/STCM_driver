///
///	@brief	Interface definition for attributes concerning the stream in general
///

/*!
	Note: every enum should have the first two entrys 'invalid' and 'unknown'.
	In case there is no need for special numbering of the enum,
	the last entry should be 'number'.
	Asking for 'number' will automatically give us the number of entries
	including 'invalid' and 'unknown'.
*/

#ifndef IVDRDATAPATHUNITS_H
#define IVDRDATAPATHUNITS_H

#include "VDR/Interface/Unit/IVDRGlobalUnitIDs.h"

/////////////////////////////////////////////////////////////////////////////
//	Public Unit IDs
/////////////////////////////////////////////////////////////////////////////

/// Program Stream Transducer Streaming Proxy Unit
/// DEPRECATED
static const VDRUID VDRUID_PROGRAM_STREAM_TRANSDUCER_PROXY       = 0x00000013;


/// DVD Program Stream Transducer Streaming Proxy Unit
static const VDRUID VDRUID_DVD_STREAM_TRANSDUCER_PROXY           = 0x0000001a;

/// MPEG 2 Program and MPEG 1 System Stream Transducer Streaming Proxy Unit
static const VDRUID VDRUID_MPEG_PROGRAM_STREAM_TRANSDUCER_PROXY  = 0x0000002f;

/// Demultiplexed Stream Transducer Streaming Proxy Unit (for DivX, or other types demuxed before VDR)
static const VDRUID VDRUID_DEMULTIPLEXED_STREAM_TRANSDUCER_PROXY = 0x00000037;

/// DVD-Audio Proxy for Audio Only (AOTT) Playback
static const VDRUID VDRUID_DVDAUDIO_STREAM_TRANSDUCER_PROXY      = 0x00000039;

/// DVD-Audio proxy for ASV playback 
static const VDRUID VDRUID_ASV_TRANSDUCER_PROXY                  = 0x0000003a;

/// DVD Program Stream Video Frame Extractor Proxy Unit
static const VDRUID VDRUID_DVD_VIDEO_FRAME_EXTRACTOR_PROXY       = 0x00000053;

// JPEG Frame Extractor Proxy Unit
static const VDRUID VDRUID_JPEG_FRAME_EXTRACTOR_PROXY            = 0x00000060;

#endif
