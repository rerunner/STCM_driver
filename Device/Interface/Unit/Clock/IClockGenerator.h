//
// PURPOSE:		Generic Clock Generator Unit Interface
//

#ifndef ICLOCKGENERATOR_H
#define ICLOCKGENERATOR_H

#include "VDR/Interface/Base/IVDRBase.h"


static const VDRIID VDRIID_CLOCK_GENERATOR = 0x8000001f;

//! Elementary clock generator
class IClockGenerator : public virtual IVDRBase
	{
	public:
		//! Get current frequency of clock generator
		virtual STFResult GetFrequency(uint32 & freq) = 0;				// freq unit is Hz
	};
	


static const VDRIID VDRIID_PROGRAMMABLE_CLOCK_GENERATOR = 0x80000020;

// Programmable clock generator (output frequency programmable)
class IProgrammableClockGenerator : public virtual IClockGenerator 
	{
	public:
		//! Set clock generator to a new frequency
		virtual STFResult SetFrequency(uint32 freq) = 0;

		//! Get error of frequency (i.e. the deviation from the ideal frequency (in Hz)).
		virtual STFResult GetError(uint32 & error) = 0;
	};



static const VDRIID VDRIID_PLL_CLOCK_GENERATOR = 0x80000021;

//! Programmable clock generator with variable base frequency
class IPLLClockGenerator : public virtual IProgrammableClockGenerator
	{
	public:
		//! Set base frequency of PLL from which the output frequency is created
		virtual STFResult SetBaseFrequency(uint32 freq) = 0;
	};


#endif	// #ifndef ICLOCKGENERATOR_H

