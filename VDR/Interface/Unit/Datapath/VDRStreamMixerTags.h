///
/// @brief    : 
///

#ifndef VDRSTREAMMIXERTAGS_H
#define VDRSTREAMMIXERTAGS_H
/// ******************************
/// Tags that can be present in the _stream_ and are handle by the mixer
/// ******************************
static const VDRTID VDRTID_MIXER_STREAM = 0x00045000;

/// @brief: A stream marked with this tag set to true must be rendererd even if the normal setting for this stream is silent
/// The setting will be in effect until explicitly changed.
MKTAG(STREAM_FORCED_RENDERING, VDRTID_MIXER_STREAM, 0x01, bool)

/// ******************************
/// The following tags are sent _directly_ to the input controls of the mixer
/// ******************************
// Stream mixer tags
static const VDRTID VDRTID_MIXER_INPUT_CONTROL = 0x0003d000;

/// State of mixer inputs
enum MixerInputState
	{
	MIS_OFF,				/// No data from this input will be mixed in
	MIS_FORCEDONLY,	/// Only forced data from this input will be mixed in
	MIS_ON				/// All data from this input will be mixed in
	};

MKTAG(MIXER_INPUT_STATE, VDRTID_MIXER_INPUT_CONTROL, 0x01, MixerInputState)

 
#endif // VDRSTREAMMIXERTAGSH
