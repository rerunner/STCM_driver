
//
// PURPOSE:		Internal Streaming Chain Clocks Interfaces
//

/*! \file
	\brief Internal Streaming Chain Clocks Interfaces
*/

#ifndef ISTREAMINGCLOCKS_H
#define ISTREAMINGCLOCKS_H

#include "STF/Interface/Types/STFMessage.h"
#include "STF/Interface/Types/STFTime.h"
#include "VDR/Interface/Base/IVDRBase.h"


//! Streaming clock client interface ID
static const VDRIID VDRIID_STREAMING_CLOCK_CLIENT = 0x8000002f;

//! Information provided by a streaming clock client for startup synchronization
struct StreamingClockClientStartupInfo
	{
	//! The stream time of the first available stream sample in the client's buffer
	STFHiPrec64BitTime		streamStartTime;
	//! Flag indicating that the stream time is valid
	bool                    streamStartTimeValid;
	//! The duration of a single atomic frame in the renderer/mixer
	STFHiPrec64BitDuration	renderFrameDuration;
	//! The system time for the next available frame start in the renderer
	STFHiPrec64BitTime		nextRenderFrameTime;
	//! The internal frame number in the renderer for the first available sample
	uint32						nextRenderFrameNumber;
	};


//! Internal streaming clock client interface
class IStreamingClockClient : public virtual IVDRBase
	{
	public:
		//! Notify client of startup information
		/*!
			The streaming clock sends the startup frame number. The client will 
			then start to output its data at the given frame number.
		*/
		virtual STFResult SetStartupFrame(uint32 frameNumber, const STFHiPrec64BitTime & startTime) = 0;

		//! Query the client for the current stream time offset
		/*!
			The streaming clock combines the timing informations from all clients
			to generate a common stream time for the application.
		*/
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset) = 0;

	};



//! Streaming clock interface ID
static const VDRIID VDRIID_STREAMING_CLOCK = 0x80000013;

//! Internal Streaming Clock interface
class IStreamingClock : public virtual IVDRBase
	{
	public:
		//! Register a client at the streaming clock
		/*!
			Each client of a streaming clock has to be registered at the clock,
			in order to be taken in account on startup and synchronization. The 
			client receives and clock relative ID, that has to be used on 
			further calls from the client to the clock.
		*/
		virtual STFResult RegisterClient(IStreamingClockClient * client, uint32 & id) = 0;

		//! Begin a synchronized startup sequence
		/*!
			Called by the owner of the clock (usually the Streaming Proxy Unit),
			before a startup sequence happens.
		*/
		virtual STFResult BeginStartupSequence(int32 speed) = 0;

		//! Register startup delay of one client
		/*!
			Each unit that provides its stream to a continuously running renderer
			has to provide its startup information to the clock, prior to starting
			playback. When all startup informations are available in the streaming
			clock, it will calculate the first valid start time, and provide this
			information to all clients via the SetStartupFrame() method. This will
			then trigger the startup of all clients.
		*/
		virtual STFResult SetStartupDelay(uint32 id, const StreamingClockClientStartupInfo & info) = 0;

		//! Provide and receive synchronization information for one client
		/*!
			During playback, one of the clients will have the mastership of the
			stream clock. All other clients will synchronize to this clock.
			The streaming clock decides mastership based on a priority level
			given by each client.
			Each client uses this method to provide relative timing information
			to the streaming clock, and in return gets its offset to the main
			streaming clock.
			During normal speed playback, the timing information is given as
			the delta of the "current private stream time" minus the "current 
			system time".
			The result will be the "current stream time" minus the "current
			private stream time". This delta will be used by the client for
			dropping, repeating or other measures of time warping.

			If the playback speed is not normal, some clock scaling has to be
			performed by the client. The time given will be the "current
			private stream time" minus the "current system time" multiplied
			by the "playback speed". The result will be in stream time
			regardless of the speed.
			In the extreme case of pause (playback speed of zero), this will
			yield a non ticking system clock and a non ticking stream clock
			resulting is a static offset.
		*/
		virtual STFResult SynchronizeClient(uint32 id, uint32 priority, const STFHiPrec64BitDuration & systemOffset, STFHiPrec64BitDuration & offset) = 0;

		//! Retrieve the offset of the current stream time relative to the system time
		/*!
			The streaming clock queries all clients for the current system time offset,
			and combines the result into a single offset. The application can calculate
			the current streamTime as

			STFHiPrec64BitTime			systemTime, streamTime;
			STFHiPrec64BitDuration			streamOffset;

			proxy->GetCurrentStreamTimeOffset(streamOffset);
			SystemTimer->GetTime(systemTime);
			streamTime = streamOffset + systemTime * speed / 0x10000;
		*/
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset) = 0;
	};


#endif	// #endif ISTREAMINGCLOCKS_H

