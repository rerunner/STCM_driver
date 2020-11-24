///
/// @brief OS-dependent debug-methods 
///

#ifndef _STFDEBUG_
#define _STFDEBUG_

#include "STF/Interface/Types/STFList.h"
#include "STF/Interface/Types/STFResult.h"
#include "OSSTFDebug.h"
#include "string.h"

//lint -esym(960, DP)
//lint -esym(960, DPR)
//lint -esym(960, DPS)
//lint -esym(960, DPF)
//lint -esym(960, DPDUMP)
//lint -esym(960, DPTN)



void MDebugPrint(const char * szFormat, ...);
#define RDP MDebugPrint

//! Function pointer to debug handler function
typedef void (*pDebugPrintHandler)(const char * dbgLine);

/*! Empty debug function,
    in non-debug prints DP is mapped to this, in debug prints you can manually
	 map DP to this if you want to exclude a specific file from debug prints.
*/
inline void DebugPrintEmpty(const char * szFormat, ...) {}		// empty function (optimized to nothing)
//lint -emacro( (681,960), DP_EMPTY )
#define DP_EMPTY while(0) DebugPrintEmpty


#if _DEBUG

	void DebugPrint (const char  * szFormat, ...);	// standard prototype for debug output
	#define DP DebugPrintWithThreadName // was: DebugPrint
	void DebugPrintRecord (const char  * szFormat, ...);	// standard prototype for debug output
	#define DPR DebugPrintRecord
	#define DPS	DPR
	void InitializeDebugRecording (void);
	void WriteDebugRecording (char *filename);

	#define DPDUMP DebugPrintDump // see STFGenericDebug.cpp
	void DebugPrintDump(unsigned char *bp, int len, int maxlen = 0, bool dumpASCII = false, uint32 startAddr = 0);

	/// Debug print with thread name in front (only call from an STFThread)
	#define DPTN DebugPrintWithThreadName
	void DebugPrintWithThreadName (const char  * szFormat, ...);	// standard prototype for debug output

//! Only DPs are routed to the Serial port for performance reasons.
#elif SERIAL_DEBUG
	void SetDebugHandler(pDebugPrintHandler handler); // standard prototype for debug handler setting
	void SerialPrint(const char * szFormat, ...);
	#define DP SerialPrint
	#define DPS SerialPrint
	//lint -emacro( (681,960), DPF)
	#define DPF while(0) DebugPrintEmpty
	//lint -emacro( (681,960), DPR)
	#define DPR while(0) DebugPrintEmpty
	//lint -emacro( (681,960), DPDUMP )
	#define DPDUMP while(0) DebugPrintEmpty // DebugPrintEmpty compiles because param1 is a char *
	//lint -emacro( (681,960), DPTN)
	#define DPTN while(0) DebugPrintEmpty
		
	inline void InitializeDebugRecording (void) {}
	inline void WriteDebugRecording (char *filename) {}
#else
		//lint -emacro( (681,960), DP )
		#define DP while(0) DebugPrintEmpty
		//lint -emacro( (681,960), DPF )
		#define DPF while(0) DebugPrintEmpty
		//lint -emacro( (681,960), DPR )
		#define DPR while(0) DebugPrintEmpty
		//lint -emacro( (681,960), DPDUMP )
		#define DPS	DPR
		#define DPDUMP while(0) DebugPrintEmpty // DebugPrintEmpty compiles because param1 is a char *
		//lint -emacro( (681,960), DPTN )
		#define DPTN while(0) DebugPrintEmpty
		inline void InitializeDebugRecording (void) {}
		inline void WriteDebugRecording (char *filename) {}

		//
#endif



// DebugLog is a PERMANENT facility. Logging allows to generate output according to levels or channels.
void DebugLog (uint32 id, const char  * szFormat, ...);
#define DEBUGLOG DebugLog

/// @name Debug log channel ID definition
//@{
/// @brief Debug log channel ID bitfield
///
/// Bit31                             Bit 0
/// |                                     |
/// C--- ---- ---- ---- UUUU UUUU UUUU UUUU
///
/// U: Unique value assigned by STIVAMA registration
/// C: Customer bit
/// -: Reserved for future use

// All log IDs are created by the ID value manager STIVAMA. Do not edit or invent values.
static const uint32 LOGID_ERROR_LOGGING	= 0;		// general channel, this is never filtered!
static const uint32 LOGID_KERNEL				= 1;
static const uint32 LOGID_APPLICATION		= 2;
static const uint32 LOGID_OPTICAL_DISC		= 3;
static const uint32 LOGID_HARD_DISC			= 4;
static const uint32 LOGID_USB					= 5;
static const uint32 LOGID_1394				= 6;
static const uint32 LOGID_NETWORK			= 7;
static const uint32 LOGID_FILE_SYSTEM		= 8;
static const uint32 LOGID_DRIVERS			= 9;
static const uint32 LOGID_NAV_PLAYBACK		= 10;
static const uint32 LOGID_NAV_RECORD		= 11;
static const uint32 LOGID_NAV_TIMESHIFT	= 12;
static const uint32 LOGID_NAV_AUTHORING	= 13;
static const uint32 LOGID_TUNER				= 14;
static const uint32 LOGID_PERIPHERAL		= 15;
static const uint32 LOGID_V_CAPTURE			= 16;
static const uint32 LOGID_V_ENCODER			= 17;
static const uint32 LOGID_V_DECODER			= 18;
static const uint32 LOGID_V_RENDERER		= 19;
static const uint32 LOGID_A_READER			= 20;
static const uint32 LOGID_A_RENDERER		= 21;
static const uint32 LOGID_EPG					= 22;
static const uint32 LOGID_BLOCK_LOGGING	= 23;		// may get changed

class STFDebugLogChannelInfo : public STFNode
	{
	public:
		virtual STFResult GetInfo (uint32 & id, bool & enabled, char *& description) = 0;
	};

#define STFDEBUGLOG_DEFAULT_BUFFERSIZE		(8*1024)

class STFDebugLogManager
	{
	public:
		// The description is not copied, so the client must keep it valid.
		static STFResult RegisterChannel (uint32 id, bool enabled, const char * description);

		static STFResult EnableChannel (uint32 id, bool enabled);
		static STFResult IsChannelEnabled (uint32 id);
		static STFResult SetAllChannels (bool enabled);

		// Creates an STFList of STFDebugLogChannelInfo. Must be deleted manually.
		static STFResult CreateChannelList (STFList *& list);

		// This function is a last resort if no draining unit is available.
		static STFResult WriteBufferContent (char *filename);

	public:
		// The following functions are used by the draining unit.

		/// @brief Get access to the next data to drain and lock it.
		///
		/// @param maxBytes [in]  The maximum size that the chunk should have.
		/// @param data     [out] Pointer to the data chunk.
		/// @param numBytes [out] Size of the chunk.
		///
		/// @return         Result value, indicates success or failure.
		/// @retval STFRES_OK                  "data" and "numBytes" are valid
		/// @retval STFRES_OBJECT_EMPTY        There is no new data.
		///
		static STFResult LockNextBufferChunk (uint32 maxBytes, char *& data, uint32 & numBytes);

		/// @brief Unlock and free the drained chunk.
		///
		/// @param data     [in] Pointer to the data chunk.
		/// @param numBytes [in] Size of the chunk.
		///
		/// @return         Result value, indicates success or failure.
		/// @retval STFRES_OK
		/// @retval STFRES_INVALID_PARAMETERS  The parameters do not fit the locked chunk.
		///
		static STFResult UnlockBufferChunk (char * data, uint32 numBytes);
	};

STFResult InitializeSTFDebugLog (uint32 bufferSize = STFDEBUGLOG_DEFAULT_BUFFERSIZE);

/// The unique debug log manager object.
//extern STFDebugLogManager * STFDebugLogManager;



#ifndef _DEBUG
//lint -emacro( (681,960), BREAKPOINT )
#define BREAKPOINT		do {} while (0)
#endif // !_DEBUG

#ifdef _DEBUG
#define DEBUG_EXECUTE(x) x
#else
//lint -emacro( (681,960), DEBUG_EXECUTE )
#define DEBUG_EXECUTE(x) do {} while (0)
#endif

#ifndef ASSERT
//lint -emacro( (681,960), ASSERT )
#define ASSERT(cond) if (!(cond)) {DP("ASSERTION FAILED: (%s, %d) : "  #cond "!\n", __FILE__, __LINE__); BREAKPOINT;} else while (0)
#endif

#if !UPDATE_UTILITY_BUILD
#define DEBUG__(x)		x
#else
#define DEBUG__(x)
#endif


#endif
