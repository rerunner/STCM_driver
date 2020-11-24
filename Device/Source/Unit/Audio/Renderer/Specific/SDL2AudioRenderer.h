#ifndef SDL2AudioRENDERER_H
#define SDL2AudioRENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
extern "C" 
{
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <linux/soundcard.h>
/* a52dec AC3 library includes */
#include "GPL/a52dec-0.7.4/include/a52.h"
#include "GPL/a52dec-0.7.4/include/mm_accel.h"
#include "SDL2/SDL.h"
}

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"
#include "VDR/Interface/Unit/Audio/IVDRAudioStreamTypes.h"

///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

class SDL2AudioRendererUnit : public SharedPhysicalUnit
{
	friend class VirtualSDL2AudioRendererUnit;

public:
	SDL2AudioRendererUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

	//
	// IPhysicalUnit interface implementation
	//
	virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

	virtual STFResult Create(uint64 * createParams) {STFRES_RAISE_OK;}
	virtual STFResult Connect(uint64 localID, IPhysicalUnit * source) {STFRES_RAISE_OK;}
	virtual STFResult Initialize(uint64 * depUnitsParams) {STFRES_RAISE_OK;}
};


/// Unit to terminate a Streaming Chain (usually for test purposes or because of incomplete chain implementation)
class VirtualSDL2AudioRendererUnit : public VirtualNonthreadedStandardStreamingUnit
{
private:
	inline int16_t convert (int32_t i)
	{	if (i > 0x43c07fff)
			return 32767;
		else if (i < 0x43bf8000)
			return -32768;
		else
			return i - 0x43c00000;
	}

	void float2s16_2 (float * _f, int16_t * s16)
	{	int32_t * f = (int32_t *) _f;
		for (int i = 0; i < 256; i++) 
		{
			s16[2*i] = convert (f[i]);
			s16[2*i+1] = convert (f[i+256]);
		}
	}

	void float2s16_4 (float * _f, int16_t * s16)
	{	int32_t * f = (int32_t *) _f;
		for (int i = 0; i < 256; i++) 
		{
			s16[4*i] = convert (f[i]);
			s16[4*i+1] = convert (f[i+256]);
			s16[4*i+2] = convert (f[i+512]);
			s16[4*i+3] = convert (f[i+768]);
		}
	}

	void float2s16_5 (float * _f, int16_t * s16)
	{	int32_t * f = (int32_t *) _f;
		for (int i = 0; i < 256; i++) 
		{
			s16[5*i] = convert (f[i]);
			s16[5*i+1] = convert (f[i+256]);
			s16[5*i+2] = convert (f[i+512]);
			s16[5*i+3] = convert (f[i+768]);
			s16[5*i+4] = convert (f[i+1024]);
		}
	}

	void float2s16_multi (float * _f, int16_t * s16, int _flags)
	{
		int i;
		int32_t * f = (int32_t *) _f;
		switch (_flags) 
		{
			case A52_MONO:
				for (i = 0; i < 256; i++) 
				{
					s16[5*i] = s16[5*i+1] = s16[5*i+2] = s16[5*i+3] = 0;
					s16[5*i+4] = convert (f[i]);
				}
				break;
			case A52_CHANNEL:
			case A52_STEREO:
			case A52_DOLBY:
				float2s16_2 (_f, s16);
				break;
			case A52_3F:
				for (i = 0; i < 256; i++) 
				{
					s16[5*i] = convert (f[i]);
					s16[5*i+1] = convert (f[i+512]);
					s16[5*i+2] = s16[5*i+3] = 0;
					s16[5*i+4] = convert (f[i+256]);
				}
				break;
			case A52_2F2R:
				float2s16_4 (_f, s16);
				break;
			case A52_3F2R:
			float2s16_5 (_f, s16);
				break;
			case A52_MONO | A52_LFE:
				for (i = 0; i < 256; i++) 
				{
					s16[6*i] = s16[6*i+1] = s16[6*i+2] = s16[6*i+3] = 0;
					s16[6*i+4] = convert (f[i+256]);
					s16[6*i+5] = convert (f[i]);
				}
				break;
			case A52_CHANNEL | A52_LFE:
			case A52_STEREO | A52_LFE:
			case A52_DOLBY | A52_LFE:
				for (i = 0; i < 256; i++) 
				{
					s16[6*i] = convert (f[i+256]);
					s16[6*i+1] = convert (f[i+512]);
					s16[6*i+2] = s16[6*i+3] = s16[6*i+4] = 0;
					s16[6*i+5] = convert (f[i]);
				}
				break;
			case A52_3F | A52_LFE:
				for (i = 0; i < 256; i++) 
				{
					s16[6*i] = convert (f[i+256]);
					s16[6*i+1] = convert (f[i+768]);
					s16[6*i+2] = s16[6*i+3] = 0;
					s16[6*i+4] = convert (f[i+512]);
					s16[6*i+5] = convert (f[i]);
				}
				break;
			case A52_2F2R | A52_LFE:
				for (i = 0; i < 256; i++) 
				{
					s16[6*i] = convert (f[i+256]);
					s16[6*i+1] = convert (f[i+512]);
					s16[6*i+2] = convert (f[i+768]);
					s16[6*i+3] = convert (f[i+1024]);
					s16[6*i+4] = 0;
					s16[6*i+5] = convert (f[i]);
				}
				break;
			case A52_3F2R | A52_LFE:
				for (i = 0; i < 256; i++) 
				{
					s16[6*i] = convert (f[i+256]);
					s16[6*i+1] = convert (f[i+768]);
					s16[6*i+2] = convert (f[i+1024]);
					s16[6*i+3] = convert (f[i+1280]);
					s16[6*i+4] = convert (f[i+512]);
					s16[6*i+5] = convert (f[i]);
				}
				break;
		}
	}

	void s16_swap (int16_t * s16, int channels)
	{	uint16_t * u16 = (uint16_t *) s16;
		for (int i = 0; i < 256 * channels; i++)
			u16[i] = (u16[i] >> 8) | (u16[i] << 8);
	}

	STFResult channels_multi ();
protected:
	int fd, format;
	int chans; ///< Number of channels
	uint32 sampleRate;

	//pa_simple *s; // pulse audio handle
	//SDL Start
	SDL_AudioSpec want, have;
	SDL_AudioDeviceID dev;
	//SDL End
	VDRAudioCodingMode codingMode;
	bool Preparing;

	virtual STFResult Render(const VDRDataRange & range, uint32 & offset);
	virtual STFResult ConfigureRenderer();

	//
	// Data range parsing
	//
	//virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
	virtual STFResult ParseRange(const VDRDataRange & range, uint32 & offset);

	//
	// Range information parsing
	//
	virtual STFResult ParseConfigure(TAG *& tags);
	virtual STFResult ParseDataDiscontinuity(void) {STFRES_RAISE_OK;}
	virtual STFResult ParseTimeDiscontinuity(void) {STFRES_RAISE_OK;}
	virtual STFResult ParseBeginGroup(uint16 groupNumber, bool requestNotification, bool singleUnitGroup)
	{	if (requestNotification)
			inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_START, groupNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseEndGroup(uint16 groupNumber, bool requestNotification)
	{	if (requestNotification)
			inputConnector->SendUpstreamNotification(VDRMID_STRM_GROUP_END, groupNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseBeginSegment(uint16 segmentNumber, bool requestNotification)
	{	if (requestNotification)
		inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_START, segmentNumber, 0);
		STFRES_RAISE_OK;
	}
	virtual STFResult ParseEndSegment(uint16 segmentNumber, bool requestNotification)
	{	if (requestNotification)
		{	DP("Renderer  has parsed end segment!!!\n");
			inputConnector->SendUpstreamNotification(VDRMID_STRM_SEGMENT_END, segmentNumber, 0);
		}
		STFRES_RAISE_OK;
	}

	//
	// Time information
	//
	virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time) {STFRES_RAISE_OK;}
	virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time) {STFRES_RAISE_OK;}
	virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}
	virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}

	/// Returns if input data is currently being used for processing.
	virtual bool InputPending(void) {return false;}

public:
	/// Specific constructor.
	/// @param physical: Pointer to interface of corresponding physical unit
	VirtualSDL2AudioRendererUnit (IPhysicalUnit * physical) : VirtualNonthreadedStandardStreamingUnit(physical) {}

	//
	// IStreamingUnit interface implementation
	//
	virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
#if _DEBUG
	//
	// IStreamingUnitDebugging functions
	//
	virtual STFString GetInformation(void)
	{
		return STFString("VirtualSDL2AudioRendererUnit ") + STFString(physical->GetUnitID());
	}
#endif
};


#endif // #ifndef SDL2AudioRENDERER_H
