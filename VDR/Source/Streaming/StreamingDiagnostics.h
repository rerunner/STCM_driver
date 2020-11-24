#ifndef STREAMINGDIAGNOSTICS_H
#define STREAMINGDIAGNOSTICS_H

///
/// @file VDR/Source/Streaming/StreamingDiagnostics.h
///
/// @brief A set of controls for debugging streaming units within the units themselves
/// CAVEATS: This header is only intended to be included by streaming connectors, 
///          and streaming units, and objects related to the streaming units.
/// 
/// The class DiagnosticStreamMixerRuntimeDatabase is implemented as the global
/// symbol diagnosticStreamMixerDatabase in StreamMixer.cpp. The class is 
/// threadsafe.  It's purpose is to detect starvation conditions, and to allow
/// conditional logging within streaming units based upon the starvation status.
///
/// @author Sam Frantz
///
/// @date 2004-05-14
///
/// @par SCOPE:
/// INTERNAL Header File
///
/// Logging and other diagnostics
///
/// &copy: 2004 ST Microelectronics. All Rights Reserved.
///

#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/STFDebug.h"

#if _DEBUG
// by changing these settings, a large number of DPs can be switched to DPRs, vice versa
#if WIN32
#define STREAMING_DP		DP // use this setting for DP on the PC
#define STREAMING_DPR	DPR // use this setting for DPR on the PC
#else // ! WIN32
#define STREAMING_DP		DP // use this setting for DP on the STi hardware
#define STREAMING_DPR	DPR // use this setting for DPR on the STi hardware
#endif // WIN32
#else // ! _DEBUG
// do not change these - nondebug compatibility definitions
#define STREAMING_DP		DP_EMPTY
#define STREAMING_DPR	DP_EMPTY
#endif // _DEBUG

// This allows global activation and deactivation of hardcoded streaming breakpoints
#define STREAMING_BREAKPOINT	BREAKPOINT  // DP_EMPTY("")

#if _DEBUG
// this is useful all over, see VDR/Source/Construction/UnitConstruction.cpp
extern char * DebugUnitNameFromID(uint32 unitID);

// These switches control various general-purpose diagnostic facilities
// In some cases, they also control other macros below...
#define DIAGNOSTIC_STREAM_MIXER_STATISTICS 0 // see StreamMixer.cpp
#define DEBUG_ENABLE_UPSTREAM_NOTIFICATION_LOGGING 0 // see StreamingConnectors.h
#define DEBUG_OCTOPUS_UPSTREAM_NOTIFICATION_LOGGING 0 // see OctopusAudioDecoder.cpp
#define DEBUG_OCTOPUS_SEND_PACKETS 0 // see OctopusAudioDecoder.cpp
#define DEBUG_MIXER_STARVATION_TRANSITIONS 0 // see StreamMixer.cpp
#define DEBUG_MMEFRAMEDECODER_CMD_COMPLETED 0 // see MMEFrameDecoder.cpp
#define DEBUG_REQUEST_PACKETS	0 // see StreamingConnectors.cpp
#define DEBUG_AUDIO_OBJECT_FULL_ERROR	0
#define DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE	0
#endif // _DEBUG

// no need to change any of these, except to switch DP to DPR
#if DEBUG_MMEAUDIOMIXER_TRANSFORM_CALLBACK && _DEBUG
#define MMEAUDMIX_DP STREAMING_DP
#define MMEAUDMIX_DPR STREAMING_DPR
#else // DEBUG_MMEAUDIOMIXER_TRANSFORM_CALLBACK
#define MMEAUDMIX_DP DP_EMPTY
#define MMEAUDMIX_DPR DP_EMPTY
#endif // DEBUG_MMEAUDIOMIXER_TRANSFORM_CALLBACK

#if DIAGNOSTIC_STREAM_MIXER_STATISTICS && _DEBUG
// use these defines for specific behavior from the individual StreamMixers
#define DEBUG_AUDIO_MIXER_TRACE_STARVATIONS	0
#define DEBUG_VIDEO_MIXER_TRACE_STARVATIONS	0
#define DEBUG_SPDIF_MIXER_TRACE_STARVATIONS	0
#define DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD	0 // 20
#define DEBUG_VIDEO_MIXER_STARVATION_BREAK_THRESHOLD	0 // 20
#define DEBUG_SPDIF_MIXER_STARVATION_BREAK_THRESHOLD	0 // 20
#define AUDIO_MIXER_IS_STARVING	diagnosticStreamMixerDatabase.AudioPCMMixerIsStarving()
#define AUDIO_MIXER_IS_VERY_STARVED	diagnosticStreamMixerDatabase.AudioPCMMixerConsecutiveStarvationsExceedsThreshold(DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD)

#if DEBUG_REQUEST_PACKETS
#define LOG_REQUEST_PACKETS_CONDITION	AUDIO_MIXER_IS_STARVING
#define CONDITIONAL_REQPKT_DP		if (LOG_REQUEST_PACKETS_CONDITION) STREAMING_DP
#else // ! DEBUG_REQUEST_PACKETS
#define LOG_REQUEST_PACKETS_CONDITION	false
#define CONDITIONAL_REQPKT_DP		DP_EMPTY
#endif // DEBUG_REQUEST_PACKETS

#else // ! DIAGNOSTIC_STREAM_MIXER_STATISTICS && _DEBUG
#define AUDIO_MIXER_IS_STARVING	false
#define AUDIO_MIXER_IS_VERY_STARVED	false
#define LOG_REQUEST_PACKETS_CONDITION false
#define CONDITIONAL_REQPKT_DP		DP_EMPTY
#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS && _DEBUG


#if DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE && _DEBUG
// this list is quite large
#define DEBUG_STF_RESULT_STRING(_result) \
  ((_result == STFRES_CONNECTION_LOST) ? "STFRES_CONNECTION_LOST" : \
	(_result == STFRES_INSUFFICIENT_RIGHTS) ? "STFRES_INSUFFICIENT_RIGHTS" : \
	(_result == STFRES_INVALID_PARAMETERS) ? "STFRES_INVALID_PARAMETERS" : \
	(_result == STFRES_OBJECT_ALREADY_JOINED) ? "STFRES_OBJECT_ALREADY_JOINED" : \
	(_result == STFRES_OBJECT_EMPTY) ? "STFRES_OBJECT_EMPTY" : \
	(_result == STFRES_OBJECT_EXISTS) ? "STFRES_OBJECT_EXISTS" : \
	(_result == STFRES_OBJECT_FOUND) ? "STFRES_OBJECT_FOUND" : \
	(_result == STFRES_OBJECT_FULL) ? "STFRES_OBJECT_FULL" : \
	(_result == STFRES_OBJECT_IN_USE) ? "STFRES_OBJECT_IN_USE" : \
	(_result == STFRES_OBJECT_INVALID) ? "STFRES_OBJECT_INVALID" : \
	(_result == STFRES_OBJECT_NOT_ALLOCATED) ? "STFRES_OBJECT_NOT_ALLOCATED" : \
	(_result == STFRES_OBJECT_NOT_CURRENT) ? "STFRES_OBJECT_NOT_CURRENT" : \
	(_result == STFRES_OBJECT_NOT_FOUND) ? "STFRES_OBJECT_NOT_FOUND" : \
	(_result == STFRES_OBJECT_READ_ONLY) ? "STFRES_OBJECT_READ_ONLY" : \
	(_result == STFRES_OBJECT_WRITE_ONLY) ? "STFRES_OBJECT_WRITE_ONLY" : \
	(_result == STFRES_OPERATION_ABORTED) ? "STFRES_OPERATION_ABORTED" : \
	(_result == STFRES_OPERATION_FAILED) ? "STFRES_OPERATION_FAILED" : \
	(_result == STFRES_OPERATION_PENDING) ? "STFRES_OPERATION_PENDING" : \
	(_result == STFRES_OPERATION_PROHIBITED) ? "STFRES_OPERATION_PROHIBITED" : \
	(_result == STFRES_OPERATION_NOT_SUPPORTED) ? "STFRES_OPERATION_NOT_SUPPORTED" : \
	(_result == STFRES_RANGE_VIOLATION) ? "STFRES_RANGE_VIOLATION" : \
	(_result == STFRES_TIMEOUT) ? "STFRES_TIMEOUT" : \
	(_result == STFRES_UNIMPLEMENTED) ? "STFRES_UNIMPLEMENTED" : \
	(_result == STFRES_NOT_ENOUGH_MEMORY) ? "STFRES_NOT_ENOUGH_MEMORY" : \
	(_result == STFRES_END_OF_FILE) ? "STFRES_END_OF_FILE" : \
	(_result == STFRES_FILE_IN_USE) ? "STFRES_FILE_IN_USE" : \
	(_result == STFRES_FILE_NOT_FOUND) ? "STFRES_FILE_NOT_FOUND" : \
	(_result == STFRES_FILE_READ_ERROR) ? "STFRES_FILE_READ_ERROR" : \
	(_result == STFRES_FILE_WRITE_ERROR) ? "STFRES_FILE_WRITE_ERROR" : \
	(_result == STFRES_FILE_WRONG_FORMAT) ? "STFRES_FILE_WRONG_FORMAT" : \
	(_result == STFRES_CAN_NOT_PASSIVATE_IDLE_UNIT) ? "STFRES_CAN_NOT_PASSIVATE_IDLE_UNIT" : \
	(_result == STFRES_INVALID_CONFIGURE_STATE) ? "STFRES_INVALID_CONFIGURE_STATE" : \
	(_result == STFRES_ERROR_RESPONSE_RECEIVED) ? "STFRES_ERROR_RESPONSE_RECEIVED" : \
	(_result == STFRES_ERROR_RESPONSE_RETURNED) ? "STFRES_ERROR_RESPONSE_RETURNED" : \
	(_result == STFRES_UNDEFINED_LOCATION_ACCESS) ? "STFRES_UNDEFINED_LOCATION_ACCESS" : \
	(_result == STFRES_UNSOLICITED_RESPONSE_RECEIVED) ? "STFRES_UNSOLICITED_RESPONSE_RECEIVED" : \
	(_result == STFRES_UNDEFINED_MEMORY_ACCESS) ? "STFRES_UNDEFINED_MEMORY_ACCESS" : \
	(_result == STFRES_UNSUPPORTED_OPERATION) ? "STFRES_UNSUPPORTED_OPERATION" : \
	(_result == STFRES_ALIGNMENT_ERROR) ? "STFRES_ALIGNMENT_ERROR" : \
	(_result == STFRES_TRANSFER_IN_PROGRESS) ? "STFRES_TRANSFER_IN_PROGRESS" :	 \
	"UNKNOWN_ERROR")
#else // ! DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE && _DEBUG
#define DEBUG_STF_RESULT_STRING(_result) ""
#endif // DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE && _DEBUG

#if DIAGNOSTIC_STREAM_MIXER_STATISTICS && _DEBUG
#include "VDR/Interface/Unit/Audio/IVDRAudioUnits.h" // to get specific public unitIDs
#include "VDR/Interface/Unit/Video/IVDRVideoUnits.h" // to get specific public unitIDs
#include "VDR/Interface/Unit/Video/Display/IVDRVideoDisplay.h" // to get specific public unitIDs

class DiagnosticStreamMixerStats
	{
	public:
		VDRUID		unitID;
		bool			isStarving;
		STFMutex		mutex;
		bool			lastStarvationValue;
		int			consecutiveStarvations;
		int			consecutiveNonStarvations;
		int			totalStarvations;
		int			totalNonStarvations;
		int			autoBreakThreshold;
		bool			traceTransitions;
		DiagnosticStreamMixerStats		**externalPtr; // keeps external pointers in sync

	public:
		DiagnosticStreamMixerStats(void)
			{
			unitID = 0;
			isStarving = false;
			lastStarvationValue = false;
			consecutiveStarvations = 0;
			consecutiveNonStarvations = 0;
			totalStarvations = 0;
			totalNonStarvations = 0;
			autoBreakThreshold = 0;
			traceTransitions = false;
			externalPtr = NULL;
			}

		void SetUnitId(VDRUID unitID)
			{ this->unitID = unitID; }

		void SetStarving(bool b)
			{
			mutex.Enter();
			isStarving = b;
			if (lastStarvationValue != b)
				{
				if (traceTransitions)
					{
					if (b)
						STREAMING_DP("%s starving after %d OK iter\n",
							DebugUnitNameFromID(unitID), consecutiveNonStarvations);
					else
						STREAMING_DP("%s OK after %d starving iter\n",
							DebugUnitNameFromID(unitID), consecutiveStarvations);
					}
				consecutiveStarvations = 0;
				consecutiveNonStarvations = 0;
				}
			if (b)
				{
				consecutiveStarvations++;
				totalStarvations++;
				if (autoBreakThreshold && autoBreakThreshold == consecutiveStarvations)
					{
					STREAMING_DP("BREAK: %s has been in starvation for %d iterations\n",
						DebugUnitNameFromID(unitID), consecutiveStarvations);
					STREAMING_BREAKPOINT;
					}
				}
			else
				{
				consecutiveNonStarvations++;
				totalNonStarvations++;
				}
			lastStarvationValue = b;
			mutex.Leave();
			}

		void SetTracetransitions(bool b)
			{ this->traceTransitions = b; }

		void SetBreakOnConsecutiveStarvationThreshold(int threshold)
			{
			autoBreakThreshold = threshold;
			}

		bool IsStarving(void)
			{
			bool b;
			
			mutex.Enter();
			b = isStarving;
			mutex.Leave();

			return b;
			}

		bool ConsecutiveStarvationsExceedsThreshold(int threshold)
			{
			bool b;
			
			mutex.Enter();
			b = (consecutiveStarvations > threshold) ? true : false;
			mutex.Leave();

			return b;
			}

		void SetExternalPtr(DiagnosticStreamMixerStats **pptr)
			{
			externalPtr = pptr;
			}

		void UpdateExternalPtr(void)
			{
			if (externalPtr)
				*externalPtr = this;
			}

	};

//
// DiagnosticStreamMixerRuntimeDatabase is instantiated in StreamMixer.cpp
// It is implemented as a global variable, externed here for use in all
// streaming-related modules.  Some of the things you can do with this 
// from ANYWHERE in the code (on host processor only!)
//
// if (diagnosticStreamMixerDatabase.AudioPCMMixerIsStarving())
//		...
// if (diagnosticStreamMixerDatabase.AudioPCMMixerConsecutiveStarvationsExceedsThreshold(10))
//		...
//

class DiagnosticStreamMixerRuntimeDatabase
	{
	protected:
		DiagnosticStreamMixerStats		*streamMixerArray;
		DiagnosticStreamMixerStats		*audioPCMMixerStats;
		DiagnosticStreamMixerStats		*spdifMixerStats;
		DiagnosticStreamMixerStats		*videoMixerStats;
		int		arraySize;
		int		actualCount;
	public:
		DiagnosticStreamMixerRuntimeDatabase(void)
			{
			streamMixerArray = NULL;
			audioPCMMixerStats = NULL;
			spdifMixerStats = NULL;
			videoMixerStats = NULL;
			arraySize = 0;
			actualCount = 0;
			}

		~DiagnosticStreamMixerRuntimeDatabase(void)
			{
			delete [] streamMixerArray;
			}

		STFResult AddStreamMixer(VDRUID unitID, DiagnosticStreamMixerStats ** pptr = NULL)
			{
			DiagnosticStreamMixerStats *	newStats;
			DiagnosticStreamMixerStats *	thisStat;
			int newSize;
			int i;

			STREAMING_DP("DiagnosticStreamMixerRuntimeDatabase::AddStreamMixer(%s)\n",DebugUnitNameFromID(unitID));
			if (actualCount == arraySize)
				{
				// The array is too small. Allocate a new one with doubled size.
				newSize = (arraySize) ? arraySize * 2 : 4;
				newStats = new DiagnosticStreamMixerStats[newSize];
				if (newStats == NULL)
					{
					if (pptr != NULL)
						*pptr = NULL;
					STFRES_RAISE(STFRES_NOT_ENOUGH_MEMORY);
					}
			
				// initialize array	
				for (i=0; i<newSize; i++)
					{
					if (i<actualCount)
						{
						newStats[i] = streamMixerArray[i];	// Copy the array content.
						thisStat = &newStats[i];
						thisStat->UpdateExternalPtr();
						switch (unitID)
							{
							case VDRUID_AUDIO_PCM_STREAM_MIXER:
								audioPCMMixerStats = thisStat;
#if DEBUG_AUDIO_MIXER_TRACE_STARVATIONS
								thisStat->SetTracetransitions(true);
#endif
#if DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD
								thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
								break;
							case VDRUID_SPDIF_STREAM_MIXER:
#if DEBUG_SPDIF_MIXER_TRACE_STARVATIONS
								thisStat->SetTracetransitions(true);
#endif
#if DEBUG_SPDIF_MIXER_STARVATION_BREAK_THRESHOLD
								thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_SPDIF_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
								spdifMixerStats = thisStat;
								break;
#if 0 // compile error in VideoFrameMixerInput.cpp - VDRUID_VIDEO_MIXER undefined?
							case VDRUID_VIDEO_MIXER:
#if DEBUG_VIDEO_MIXER_TRACE_STARVATIONS
								thisStat->SetTracetransitions(true);
#endif
#if DEBUG_VIDEO_MIXER_STARVATION_BREAK_THRESHOLD
								thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_VIDEO_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
								videoMixerStats = thisStat;
								break;
#endif // 0
							default:
								break;
							}
						}
					// else
					// 	newStats[i] = NULL;
					}

				// Replace the old array with the new one.
				delete[] streamMixerArray;
				streamMixerArray = newStats;
				arraySize = newSize;
				}

			// Add unit at the end.
			thisStat = &streamMixerArray[actualCount];
			actualCount++;
			thisStat->SetUnitId(unitID);
			switch (unitID)
				{
				case VDRUID_AUDIO_PCM_STREAM_MIXER:
					audioPCMMixerStats = thisStat;
#if DEBUG_AUDIO_MIXER_TRACE_STARVATIONS
					thisStat->SetTracetransitions(true);
#endif
#if DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD
					thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_AUDIO_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
					break;
				case VDRUID_SPDIF_STREAM_MIXER:
#if DEBUG_SPDIF_MIXER_TRACE_STARVATIONS
					thisStat->SetTracetransitions(true);
#endif
#if DEBUG_SPDIF_MIXER_STARVATION_BREAK_THRESHOLD
					thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_SPDIF_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
					spdifMixerStats = thisStat;
					break;
#if 0 // compile error in VideoFrameMixerInput.cpp - VDRUID_VIDEO_MIXER undefined?
				case VDRUID_VIDEO_MIXER:
#if DEBUG_VIDEO_MIXER_TRACE_STARVATIONS
					thisStat->SetTracetransitions(true);
#endif
#if DEBUG_VIDEO_MIXER_STARVATION_BREAK_THRESHOLD
					thisStat->SetBreakOnConsecutiveStarvationThreshold(DEBUG_VIDEO_MIXER_STARVATION_BREAK_THRESHOLD);
#endif
					videoMixerStats = thisStat;
					break;
#endif
				default:
					break;
				}

			if (pptr)
				{
				thisStat->SetExternalPtr(pptr);
				thisStat->UpdateExternalPtr();
				}

			STFRES_RAISE_OK;
			}

		DiagnosticStreamMixerStats * GetStreamMixerDiagStats(VDRUID unitID)
			{
			int i;

			for (i=0; i<actualCount; i++)
				if (streamMixerArray[i].unitID == unitID)
					return &streamMixerArray[i];

			return NULL;
			}

		bool AllStreamMixersAreStarving(void)
			{
			for (int i=0; i<actualCount; i++)
				if (!streamMixerArray[i].IsStarving())
					return false;

			return true;
			}

		bool AnyStreamMixersAreStarving(void)
			{
			for (int i=0; i<actualCount; i++)
				if (streamMixerArray[i].IsStarving())
					return true;

			return false;
			}

		bool AudioPCMMixerIsStarving(void)
			{
			if (audioPCMMixerStats)
				return audioPCMMixerStats->IsStarving();
			else
				return false;
			}

		bool SPDIFMixerIsStarving(void)
			{
			if (spdifMixerStats)
				return spdifMixerStats->IsStarving();
			else
				return false;
			}

		bool VideoMixerIsStarving(void)
			{
			if (videoMixerStats)
				return videoMixerStats->IsStarving();
			else
				return false;
			}

		bool AudioPCMMixerConsecutiveStarvationsExceedsThreshold(int n)
			{
			if (audioPCMMixerStats)
				return audioPCMMixerStats->ConsecutiveStarvationsExceedsThreshold(n);
			else
				return false;
			}

		bool SPDIFMixerConsecutiveStarvationsExceedsThreshold(int n)
			{
			if (spdifMixerStats)
				return spdifMixerStats->ConsecutiveStarvationsExceedsThreshold(n);
			else
				return false;
			}

		bool VideoMixerConsecutiveStarvationsExceedsThreshold(int n)
			{
			if (videoMixerStats)
				return videoMixerStats->ConsecutiveStarvationsExceedsThreshold(n);
			else
				return false;
			}
		};

extern DiagnosticStreamMixerRuntimeDatabase diagnosticStreamMixerDatabase; // see StreamMixer.cpp

#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS && _DEBUG

#if DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE
#undef STFRES_RAISE
#if DIAGNOSTIC_STREAM_MIXER_STATISTICS
#define STFRES_RAISE(_result) \
	do \
	{ \
	STFResult	_tmpResult = _result; \
	if (STFRES_FAILED(_tmpResult) && (_tmpResult != STFRES_OBJECT_FULL) && (_tmpResult != STFRES_OBJECT_EMPTY)) \
		{ \
		STREAMING_DP("STFRES_RAISE(%s=0x%x) %s:%d\n", \
			DEBUG_STF_RESULT_STRING(_tmpResult), _tmpResult, __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else if (_tmpResult == STFRES_OBJECT_FULL && AUDIO_MIXER_IS_STARVING) \
		{ \
		STREAMING_DP("STFRES_RAISE(STFRES_OBJECT_FULL) %s:%d\n", __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else if (_tmpResult == STFRES_OBJECT_EMPTY && AUDIO_MIXER_IS_STARVING) \
		{ \
		STREAMING_DP("STFRES_RAISE(STFRES_OBJECT_EMPTY) %s:%d\n", __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else \
		return (_tmpResult); \
	} while (0)

#define STFRES_RAISE_STREAMING(_result) \
	do \
	{ \
	STFResult	_tmpResult = _result; \
	if (STFRES_FAILED(_tmpResult) && (_tmpResult != STFRES_OBJECT_FULL) && (_tmpResult != STFRES_OBJECT_EMPTY)) \
		{ \
		STREAMING_DP("%s: STFRES_RAISE(%s=0x%x) %s:%d\n", \
			(char *) this->GetInformation(), DEBUG_STF_RESULT_STRING(_tmpResult), _tmpResult, __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else if (_tmpResult == STFRES_OBJECT_FULL && AUDIO_MIXER_IS_STARVING) \
		{ \
		STREAMING_DP("%s: STFRES_RAISE(STFRES_OBJECT_FULL) %s:%d\n", (char *) this->GetInformation(), __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else if (_tmpResult == STFRES_OBJECT_EMPTY && AUDIO_MIXER_IS_STARVING) \
		{ \
		STREAMING_DP("%s: STFRES_RAISE(STFRES_OBJECT_EMPTY) %s:%d\n", (char *) this->GetInformation(), __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else \
		return (_tmpResult); \
	} while (0)

#else // ! DIAGNOSTIC_STREAM_MIXER_STATISTICS
#define STFRES_RAISE(_result) \
	do \
	{ \
	STFResult	_tmpResult = _result; \
	if (STFRES_FAILED(_tmpResult) && (_tmpResult != STFRES_OBJECT_FULL)) \
		{ \
		STREAMING_DP("STFRES_RAISE(%s=0x%x) %s:%d\n", \
			DEBUG_STF_RESULT_STRING(_tmpResult), _tmpResult, __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else \
		return (_tmpResult); \
	} while (0)

#define STFRES_RAISE_STREAMING(_result) \
	do \
	{ \
	STFResult	_tmpResult = _result; \
	if (STFRES_FAILED(_tmpResult) && (_tmpResult != STFRES_OBJECT_FULL)) \
		{ \
		STREAMING_DP("$s: STFRES_RAISE(%s=0x%x) %s:%d\n", \
			(char *) this->GetInformation(), DEBUG_STF_RESULT_STRING(_tmpResult), _tmpResult, __FUNCTION__, __LINE__); \
		return (_tmpResult); \
		} \
	else \
		return (_tmpResult); \
	} while (0)

#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS
#undef STFRES_REASSERT
#define STFRES_REASSERT(_result) \
	do \
	{ \
	STFResult	_tmpResult2; \
	_tmpResult2 = _result; \
	if (STFRES_IS_ERROR(_tmpResult2)) \
		STFRES_RAISE(_tmpResult2); \
	} while (0)
#endif // DEBUG_STFRES_RAISE_PRINT_FILE_AND_LINE

#endif // STREAMINGDIAGNOSTICS_H

