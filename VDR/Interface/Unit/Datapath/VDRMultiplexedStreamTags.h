///
/// @brief    : 
///

#ifndef VDRMULTIPLEXEDSTREAMTAGS_H
#define VDRMULTIPLEXEDSTREAMTAGS_H

#include "VDR/Interface/Unit/IVDRTags.h"

/// ******************************
/// Tags that can be present in the _stream_ and are taken by the DVD Stream Demultiplexer
/// ******************************
static const VDRTID VDRTID_MULTIPLEXED_STREAM = 0x0004a000;

MKTAG(START_PTM, VDRTID_MULTIPLEXED_STREAM, 0x01, uint32)	// taken by the DVD PES Splitters, handled at group start
MKTAG(END_PTM,   VDRTID_MULTIPLEXED_STREAM, 0x02, uint32)	// taken by the DVD PES Splitters, handled at group start
MKTAG(DEMUX_DVDAUDIO_SCAN_MODE,   VDRTID_MULTIPLEXED_STREAM, 0x03, uint32)	// Set AudioScan mode in case of Audio with scanning in DVDAudio






/// ******************************
/// Tags that can be present in the _stream_ and are used by the ASF unpacker, splitter or WMA frame decoder
/// ******************************

static const VDRTID VDRTID_ASFHDR_ATTRIBUTE = 0x00048000;

// ASF Stream Tags from ASF file header parser
MKTAG(ASF_STREAM_FILEHDR_FORMATTAG,				VDRTID_ASFHDR_ATTRIBUTE,  0x01,  uint16)
MKTAG(ASF_STREAM_FILEHDR_BITSPERSEC,			VDRTID_ASFHDR_ATTRIBUTE,  0x02,  uint32)
MKTAG(ASF_STREAM_FILEHDR_AVGBYTESPERSEC,		VDRTID_ASFHDR_ATTRIBUTE,  0x03,  uint32)
MKTAG(ASF_STREAM_FILEHDR_BITSPERSAMPLE,		VDRTID_ASFHDR_ATTRIBUTE,  0x04,  uint32)
MKTAG(ASF_STREAM_FILEHDR_SAMPLESPERSEC,		VDRTID_ASFHDR_ATTRIBUTE,  0x05,  uint32)
MKTAG(ASF_STREAM_FILEHDR_NUMCHANNELS,			VDRTID_ASFHDR_ATTRIBUTE,  0x06,  uint16)
MKTAG(ASF_STREAM_FILEHDR_BLOCKALIGN,			VDRTID_ASFHDR_ATTRIBUTE,  0x07,  uint32)
MKTAG(ASF_STREAM_FILEHDR_VALIDBITSPERSAMPLE,	VDRTID_ASFHDR_ATTRIBUTE,  0x08,  uint16)
MKTAG(ASF_STREAM_FILEHDR_CHANNELMASK,			VDRTID_ASFHDR_ATTRIBUTE,  0x09,  uint32)
MKTAG(ASF_STREAM_FILEHDR_ENCODEOPT,				VDRTID_ASFHDR_ATTRIBUTE,  0x0a,  uint32)
MKTAG(ASF_STREAM_FILEHDR_SAMPLESPERBLOCK,		VDRTID_ASFHDR_ATTRIBUTE,  0x0b,  uint32)
MKTAG(ASF_STREAM_FILEHDR_PACKETSIZE,			VDRTID_ASFHDR_ATTRIBUTE,  0x0c,  uint32)
MKTAG(ASF_STREAM_FILEHDR_MAXBITRATE,			VDRTID_ASFHDR_ATTRIBUTE,  0x0d,  uint32)
MKTAG(ASF_STREAM_MAXPAGESIZE,						VDRTID_ASFHDR_ATTRIBUTE,  0x0e,  uint32)
MKTAG(ASF_STREAM_FINALBLOCK,						VDRTID_ASFHDR_ATTRIBUTE,  0x0f,  bool)
MKTAG(ASF_STREAM_FILEHDR_NOT_PARSED,			VDRTID_ASFHDR_ATTRIBUTE,  0x10,  bool)


static const VDRTID VDRTID_ASFPKT_ATTRIBUTE = 0x00047000;

// ASF Stream Tags from packet parser
MKTAG(ASF_STREAM_PARSEINFO_OFS_LEN_TYPE,		VDRTID_ASFPKT_ATTRIBUTE,  0x01,  uint8)
MKTAG(ASF_STREAM_PARSEINFO_PAY_LEN_TYPE,		VDRTID_ASFPKT_ATTRIBUTE,  0x02,  uint8)
MKTAG(ASF_STREAM_PARSEINFO_MULTI_PAYLOAD,		VDRTID_ASFPKT_ATTRIBUTE,  0x03,  bool)
MKTAG(ASF_STREAM_PARSEINFO_EXP_PACKET_LEN,	VDRTID_ASFPKT_ATTRIBUTE,  0x04,  uint32)
MKTAG(ASF_STREAM_PARSEINFO_PADDING,				VDRTID_ASFPKT_ATTRIBUTE,  0x05,  uint32)
MKTAG(ASF_STREAM_PARSEINFO_PAYBYTES,			VDRTID_ASFPKT_ATTRIBUTE,  0x06,  uint8)
MKTAG(ASF_STREAM_PARSEINFO_PARSEOFS,			VDRTID_ASFPKT_ATTRIBUTE,  0x07,  uint32)
MKTAG(ASF_STREAM_PARSEINFO_OFSBYTES,			VDRTID_ASFPKT_ATTRIBUTE,  0x08,  uint8)
MKTAG(ASF_STREAM_PARSEINFO_PAYLD_OFS,			VDRTID_ASFPKT_ATTRIBUTE,  0x09,  uint32)	// compacted tag of OFS_LEN_TYPE<<24, OFSBYTES<<16, PAY_LEN_TYPE<<8, PAYBYTES (all of these are single byte vals)


 
#endif // VDRMULTIPLEXEDSTREAMTAGS_H