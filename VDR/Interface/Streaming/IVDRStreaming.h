#ifndef IVDRSTREAMING_H
#define IVDRSTREAMING_H

///
/// @brief      VDR Streaming Interfaces & Definitions
///

#include "STF/Interface/Types/STFTime.h"
#include "VDR/Interface/Base/IVDRBase.h"


#include "VDR/Interface/Base/IVDRMessage.h"
#include "VDR/Interface/Unit/IVDRTags.h"
#include "VDR/Interface/Memory/IVDRMemoryPoolAllocator.h"

#include "STF/Interface/STFDataManipulationMacros.h"


///////////////////////////////////////////////////////////////////////////////
// Constants definitions
///////////////////////////////////////////////////////////////////////////////

/// Maximum number of Data Ranges and tags possible in a Streaming Data Packet
///
/// We limit this so that we can use a fixed size array for performance
/// reasons. It is no real limitation as simply another Data Packet
/// can be sent in case more Data Ranges or TAGs need to be sent.
#define VDR_MAX_TAG_DATA_RANGES_PER_PACKET	16


///////////////////////////////////////////////////////////////////////////////
// Error codes definitions
///////////////////////////////////////////////////////////////////////////////

/// Returned when Streaming Unit/Chain is in the wrong state for the operation to succeed
static const STFResult STFRES_ILLEGAL_STREAMING_STATE = 0x80000000;

/// Returned when the connector is not connected with another
static const STFResult STFRES_NOT_CONNECTED = 0x80000001;

/// The streaming chain is in a transitional state, so it does not accept a Streaming Command
static const STFResult STFRES_INVALID_STREAMING_STATE_FOR_COMMAND = 0x88844017;

/// The direction parameter of VDR_STRMCMD_BEGIN has an invalid value (must be either VDR_STRMDIR_FORWARD
/// or VDR_STRMDIR_BACKWARD).
static const STFResult STFRES_INVALID_STREAMING_DIRECTION = 0x88844018;

/// The speed parameter of VDR_STRMCMD_DO has an invalid value.
static const STFResult STFRES_INVALID_STREAMING_SPEED = 0x88844019;

/// The step time parameter of VDR_STRMCMD_STEP has an invalid value;
static const STFResult STFRES_INVALID_STREAMING_STEPTIME = 0x8884401a;

/// The streaming command was not recognized (invalid value).
static const STFResult STFRES_INVALID_STREAMING_COMMAND = 0x8884401b;

/// Warning which is returned by IVDStreamingUnit::Send Command when a command 
/// is already being processed.
static const STFResult STFRES_PROCESSING_COMMAND = 0x47044002;


///////////////////////////////////////////////////////////////////////////////
// Range Type definitions
///////////////////////////////////////////////////////////////////////////////

static const VDRRID VDRRID_AUDIO				=	0x00002000;
static const VDRRID VDRRID_AUDIO_SOF		=	0x00003000;
static const VDRRID VDRRID_VIDEO				=	0x00004000;
static const VDRRID VDRRID_VIDEO_SOF		=	0x00005000;
static const VDRRID VDRRID_SUBPIC			=	0x00006000;
static const VDRRID VDRRID_SUBPIC_SOF		=	0x00007000;
static const VDRRID VDRRID_STILL				=	0x00008000;
static const VDRRID VDRRID_ENCRYPTIONINFO	=	0x00009000;


///////////////////////////////////////////////////////////////////////////////
// Flag definitions
///////////////////////////////////////////////////////////////////////////////

//@{
/// Flags definitions for "flags" field of StreamingDataPackets
/// MSCF_...: command flags
/// MSMF_...: marker flags
#define VDR_MSCF_SKIP_UNTIL							MKFLAG(0)	/// Request skip - skipDuration field valid
#define VDR_MSCF_CUT_AFTER								MKFLAG(1)	/// Request cut - cutDuration field valid

#define VDR_MSCF_SEGMENT_START_NOTIFICATION		MKFLAG(7)	/// Notification on Segment Start is requested (VDRMID_STRM_SEGMENT_START and VDRMID_STRM_SEGMENT_START_TIME)
#define VDR_MSCF_SEGMENT_END_NOTIFICATION			MKFLAG(8)	/// Notification on Segment End is requested (VDRMID_STRM_SEGMENT_END)
#define VDR_MSCF_GROUP_START_NOTIFICATION			MKFLAG(9)	/// Notification on Group Start is requested (VDRMID_STRM_GROUP_START)
#define VDR_MSCF_GROUP_END_NOTIFICATION			MKFLAG(10)	/// Notification on Group End is requested (VDRMID_STRM_GROUP_END)

#define VDR_MSMF_SEGMENT_START						MKFLAG(15)	/// Indicates segment start and "segmentNumber" field valid
#define VDR_MSMF_SEGMENT_END							MKFLAG(16)	/// Indicates segment end and "segmentNumber" field valid
#define VDR_MSMF_GROUP_START							MKFLAG(17)	/// Indicates group start and "groupNumber" field valid
#define VDR_MSMF_GROUP_END								MKFLAG(18)	/// Indicates group endand "groupNumber" field valid

#define VDR_MSMF_SINGLE_UNIT_GROUP					MKFLAG(19)	/// Marks a group containing just one unit of presentation

#define VDR_MSMF_DATA_DISCONTINUITY					MKFLAG(20)	/// Mark data discontinuity
#define VDR_MSMF_TIME_DISCONTINUITY					MKFLAG(21)	/// Mark time discontinuity

#define VDR_MSMF_END_OF_STREAM						MKFLAG(22)	/// Marks end of a stream (obsolete???)

#define VDR_MSMF_START_TIME_VALID					MKFLAG(23)	/// Indicates validity of startTime field
#define VDR_MSMF_END_TIME_VALID						MKFLAG(24)	/// Indicates validity of endTime field

#define VDR_MSMF_TAGS_VALID							MKFLAG(25)	/// Indicates availability of at least one Tag - numTags field valid
//@}


/// @brief Data Packet to transport streaming media data
///
/// Contains media data samples together with flags and commands.
/// Depending on the embedded commands, additional parameters describing the
/// properties of the media data are specified.
/// For more details, see document "Advanced STCM Kernel Streaming Architecture".
class VDRStreamingDataPacket
	{
	public:
		/// Specifies the size of this struct
		///
		/// The struct may be extended in the future. For backward compatibility,
		/// it therefore carries its size.
		uint32						size;

		//
		// Identification section
		//

		/// The segment this data packet belongs to
		/// 
		/// A new segment is started with a packet carrying MSMF_SEGMENT_START
		/// and ending with a packet carrying MSMF_SEGMENT_END.
		uint16						segmentNumber;

		/// Group number to maintain the order of the data packets
		/// 
		/// group numbers are ascending for all packets between and including
		/// two packets carrying MSMF_GROUP_START and MSMF_GROUP_END in their 
		/// flags.
		uint16						groupNumber;

		//
		// Flags section
		//

		/// Flags
		/// 
		/// Commands and Stream Flags (MSCF_... and MSMF_...)
		uint32						flags;

		//
		// Parameter section
		//
		
		/// Presentation Start Time
		/// 
		/// Only valid if the packet flag MSMF_START_TIME_VALID is set.
		STFHiPrec64BitTime		startTime;

		/// Presentation End Time
		/// 
		/// Only valid if the packet flag MSMF_END_TIME_VALID is set.
		STFHiPrec64BitTime		endTime;


		/// Skip Duration 
		/// 
		/// Specifies the amount of stream time that should be skipped at the start of the group.
		/// Only valid if the MSCF_SKIP_UNTIL command is set.
		STFHiPrec32BitDuration	skipDuration;


		/// Cut Duration
		/// Specifies the amount of stream time after which all further data of a group
		/// has to be discarded.
		/// Only valid if the MSCF_CUT_AFTER command is set.
		STFHiPrec32BitDuration	cutDuration;

		//
		// Data section
		//

		/// Number of valid ranges in the list
		uint8							numRanges;

		/// Number of valid tags in the list (ranges start after tags)
		uint8							numTags;

		/// Flags indicating frame starts in the data ranges
		uint16						frameStartFlags;

		/// Data ranges and tags
		/// 
		/// The data ranges and tags share a common area in the streaming
		/// packet.  The tags start at the index 0, the data ranges start
		/// at ranges + numTags.  If the tag list is empty, this is
		/// described by numTags == 0 and MSMF_TAGS_VALID not set, not by
		/// a TAGDONE !!!
		union 
			{
			/// TAG List
			/// If packet flag MSMF_TAGS_VALID is set, then a TAG list describing the
			/// properties of the media data contained in the packet is available here.
			/// The list is, as usual, terminated by TAGDONE.
			/// 
			/// The TAG list should usually only be present if there is a 
			/// MSMF_DATA_DISCONTINUITY flag set.
			TAGITEM					tags[VDR_MAX_TAG_DATA_RANGES_PER_PACKET];

			/// List of Data Ranges
			/// 
			/// This list may be empty if just commands or Tags are provided.
			VDRDataRange			ranges[VDR_MAX_TAG_DATA_RANGES_PER_PACKET];
			} tagRanges;


		VDRStreamingDataPacket(void)
			{
			size					= sizeof(VDRStreamingDataPacket);
			numTags				= 0;
			numRanges			= 0;
			frameStartFlags	= 0;
			flags					= 0;
			}
	};



//@{
///////////////////////////////////////////////////////////////////////////////
///@brief Streaming Commands for IVDRStreamingCommand->SendCommand()
///////////////////////////////////////////////////////////////////////////////
///
/// VDRStreamingCommands can be executed asynchronously. By registering a message sink
/// with the IVDRMessageSink interface of the concerned Streaming Unit, a notification on
/// a state transition due to a command completion is possible.
///
/// Additional notes:
///
/// The direction parameter of the BEGIN command must be set to either 
/// VDR_STRMDIR_FORWARD or VDR_STRMDIR_BACKWARD.
///
/// The playback speed parameter of the DO command is an unsigned value with 
/// a value of 0x10000 meaning 1x speed.
///
enum VDRStreamingCommand
	{							// Cmd		         Params 				   State transition of Streaming Chain
	VDR_STRMCMD_NONE,		// No command - just used internally by VDR Layer
	VDR_STRMCMD_BEGIN,	// Begin streaming   (signed: direction)  <Idle->Preparing->Ready, Streaming->Stopping->Ready>
	VDR_STRMCMD_DO,		// Do streaming      (unsigned: speed)    <Ready->Starting->Streaming, Streaming->Starting->Streaming>
	VDR_STRMCMD_STEP,	// Single Step   (unsigned: steps in frames) <Ready->Stepping->Ready>
//	VDR_STRMCMD_STEP_TIME,		// Single Step       (signed: duration in ms) <Ready->Stepping->Ready>
	VDR_STRMCMD_FLUSH		// Flush data        mode                 <Ready->Flushing->Idle, Streaming->Flushing->Idle>
	};
//@}


//@{
/// Definition of Streaming Direction for the parameter of VDR_STRMCMD_BEGIN
static const int32 VDR_STRMDIR_FORWARD		=  1;
static const int32 VDR_STRMDIR_BACKWARD	= -1;
//@}

//@{
/// Definition of Flush Mode for the parameter of VDR_STRMCMD_FLUSH
static const int32 VDR_FLUSH_RESET			=	0;
static const int32 VDR_FLUSH_SEEK			=	1;
//@}


///////////////////////////////////////////////////////////////////////////////
/// @brief States of a Streaming Chain
///////////////////////////////////////////////////////////////////////////////
/// 
/// See state transition diagram in STCM Streaming Architecture Documentation
/// for details.
enum VDRStreamingState
	{
	// Steady States - commands are only accepted in these states
	VDR_STRMSTATE_IDLE,			/// No processing
	VDR_STRMSTATE_READY,			/// Unit ready to receive data, decoding performed, no mixing/presentation
	VDR_STRMSTATE_STREAMING,	/// Unit ready to receive data, decoding performed, mixing presentation

	// Transitional States
	VDR_STRMSTATE_PREPARING,	/// Unit not yet ready to receive data
	VDR_STRMSTATE_FLUSHING,		/// Unit is discarding all data
	VDR_STRMSTATE_STARTING,		/// Startup synchronisation is performed
	VDR_STRMSTATE_STOPPING,		/// Mixing/Presentation is stopped
	VDR_STRMSTATE_STEPPING,		/// A fixed amount of data is dropped

	// Exceptional state
	VDR_STRMSTATE_TERMINATED	/// If this state is reached, no recovery is possible from errors that
										/// occurred during command execution. Reaching this state signals
										/// a major failure which can only be recovered by shutting down
										/// the streaming chain.
	};


//@{
///////////////////////////////////////////////////////////////////////////////
/// @brief Stream Notification Messages
///////////////////////////////////////////////////////////////////////////////
///
/// These messages are sent from downstream units to upstream units
/// or from a Streaming Proxy unit to the application.
/// They provide information about the status of packet processing or quality
/// (e.g. data starvation, flood).
/// Some messages can carry a parameter.
/// Begin/End Segment/group Messages are only received if the corresponding
/// flags are set in the command bitfield (e.g. VDR_MSCF_SEGMENT_START_NOTIFICATION)
/// 
/// Message										Parameter1		Parameter2				Comment
/// 
/// VDRMID_STRM_PACKET_REQUEST 			Connector ID	-							Downstream unit that bounced once is free again and requesting more data packets
/// VDRMID_STRM_PACKET_ARRIVAL 			Connector ID	Number of packets		Packets ready for application to be retrieved from Proxy Unit
/// VDRMID_STRM_STARVING																	Streaming was interrupted due to starvation
/// 
/// VDRMID_STRM_COMMAND_COMPLETED		Command			-							Signals completion of a command
/// 
/// VDRMID_STRM_SEGMENT_START				segmentNumber  -							Segment start has been processed
/// VDRMID_STRM_SEGMENT_START_TIME		Time of segment presentation start  Time of segment start (STFHiPrec64BitTime (Param1: Lower 32 bits, Param2: higher 32 bits)
/// VDRMID_STRM_SEGMENT_END				segmentNumber	-							Segment has been processed
/// VDRMID_STRM_GROUP_START				groupNumber		time delta				Group start has been processed (this delta is to the last timed message)
/// VDRMID_STRM_GROUP_END					groupNumber		time delta				Group end has been processed (this delta is to the last timed message)
///
/// Timed messages in this context are VDRMID_STRM_SEGMENT_START_TIME,
/// VDRMID_STRM_GROUP_START and VDRMID_STRM_GROUP_END.  A typical scenario in
/// video NTSC will thus look like this:
///
/// VDRMID_STRM_SEGMENT_START(segid, 0)
/// VDRMID_STRM_SEGMENT_START_TIME(timelow, timehi)
/// VDRMID_STRM_GROUP_START(0, 0ms)
/// VDRMID_STRM_GROUP_END(0, 33ms)
/// VDRMID_STRM_GROUP_START(1, 0ms)
/// VDRMID_STRM_GROUP_END(1, 33ms)
/// VDRMID_STRM_GROUP_START(2, 0ms)
/// VDRMID_STRM_GROUP_END(2, 33ms)
/// ...
///
/// 
/// VDRMID_STRM_START_POSSIBLE			-					-							Sent in the VDR_STRMSTATE_READY to signal that all mixer inputs
/// 																								in the Streaming Chain have enough data available to start rendering.
/// 																								The VDR_STRMCMD_DO *can* be sent on reception of the message.
/// VDRMID_STRM_START_REQUIRED			-					-							Sent in the VDR_STRMSTATE_READY to signal that the input queues of
/// 																								at least one mixer input in the Streaming Chain are full.
/// 																								The VDR_STRMCMD_DO *must* be sent on reception of the message.
/// 
/// VDRMID VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE		-							A memory pool allocator has blocks available again
/// 
/// VDRMID_STRM_DATA_DISCONTINUITY_PROCESSED				-							A Data Discontinuity was processed
/// 

/// Registered VDRMID_STREAMING range
static const VDRMID VDRMID_STREAMING = 0x0004d000;

static const VDRMID VDRMID_STRM_PACKET_REQUEST						= VDRMID_STREAMING + 0x000;
static const VDRMID VDRMID_STRM_PACKET_ARRIVAL						= VDRMID_STREAMING + 0x001;
static const VDRMID VDRMID_STRM_STARVING								= VDRMID_STREAMING + 0x002;
static const VDRMID VDRMID_STRM_PACKET_ARRIVAL_VOBU				= VDRMID_STREAMING + 0x003;

static const VDRMID VDRMID_STRM_COMMAND_COMPLETED					= VDRMID_STREAMING + 0x010;

static const VDRMID VDRMID_STRM_SEGMENT_START						= VDRMID_STREAMING + 0x020;
static const VDRMID VDRMID_STRM_SEGMENT_START_TIME					= VDRMID_STREAMING + 0x021;
static const VDRMID VDRMID_STRM_SEGMENT_END							= VDRMID_STREAMING + 0x022;
static const VDRMID VDRMID_STRM_GROUP_START							= VDRMID_STREAMING + 0x023;
static const VDRMID VDRMID_STRM_GROUP_END								= VDRMID_STREAMING + 0x024;

static const VDRMID VDRMID_STRM_START_POSSIBLE						= VDRMID_STREAMING + 0x030;
static const VDRMID VDRMID_STRM_START_REQUIRED						= VDRMID_STREAMING + 0x031;

static const VDRMID VDRMID_STRM_ALLOCATOR_BLOCKS_AVAILABLE		= VDRMID_STREAMING + 0x040;

static const VDRMID VDRMID_STRM_DATA_DISCONTINUITY_PROCESSED	= VDRMID_STREAMING + 0x100;

//@}



///////////////////////////////////////////////////////////////////////////////
// "Streaming" Unit Interfaces
///////////////////////////////////////////////////////////////////////////////


/// Streaming Proxy Unit Interface ID
static const VDRIID VDRIID_VDR_STREAMING_PROXY_UNIT = 0x00000018;


/// @brief Streaming Proxy Unit Interface
/// 
/// The Streaming Proxy Unit Interface provides the functions by which the application
/// can interact with the underlying "real" Streaming (Chain) Unit. 
/// On some platforms, the proxy may take care of translating between User and Kernel
/// space. 
class IVDRStreamingProxyUnit : public virtual IVDRBase
	{
	public:
		/// @brief Asynchonously send a command to the Virtual Streaming Unit
		/// 
		/// Completion of the command is signalled by the reception of a state transition
		/// notification via a registered message sink.
		/// Only one command can be handled asynchronously at a time. If another
		/// command is sent while the first is still being processed, an error
		/// result is returned.
		virtual STFResult SendCommand(VDRStreamingCommand com, int32 param) = 0;

		/// @brief Get state the Streaming Unit/Chain is in
		/// 
		/// If the state machine is not in a defined state, e.g. during state transitions,
		/// an error (STFRES_IN_STATE_TRANSITION) is returned. A notification about
		/// the completion of a state transition is sent to a registered message sink. 
		virtual STFResult GetState(VDRStreamingState & state) = 0;

		/// @brief Retrieve the offset of the current stream time relative to the system time
		/// 
		/// The streaming clock queries all clients for the current system time offset,
		/// and combines the result into a single offset. The application can calculate
		/// the current streamTime as
		/// 
		/// STFHiPrec64BitTime			systemTime, streamTime;
		/// STFHiPrec64BitDuration		streamOffset;
		/// 
		/// proxy->GetCurrentStreamTimeOffset(streamOffset);
		/// SystemTimer->GetTime(systemTime);
		/// streamTime = streamOffset + systemTime * speed / 0x10000;
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset) = 0;

		/// @brief Receive newly arrived data on an "application input connector"
		/// 
		/// This function is called by the application after it received a
		/// data arrival notification message for the connector (VDR_STRMMSG_PACKET_ARRIVAL)
		/// specified by \param connectorID to receive data on an input connector.
		/// The application must provide a certain number of Data Packets into which 
		/// the information about the new data is copied. If there are not enough 
		/// packets provided by the application, a warning is returned 
		/// (STFRES_MORE_DATA_AVAILABLE) and \param numPackets is set to
		/// the number of currently available packets. The provided data packets can 
		/// immediately be reused after the call has returned.
		/// 
		/// The call does not block the caller's thread. 
		virtual STFResult GetDataPackets(uint32 connectorID, VDRStreamingDataPacket * packets, 
													uint32 numPackets, uint32 & filledPackets) = 0;

		/// @brief Deliver a certain number of Data Packets to an "application output connector"
		/// 
		/// This function is called by the application. The content of the 
		/// VDRStreamingDataPackets is copied internally to VDR, so that they can be reused
		/// immediately after the call has returned.
		/// 
		/// The call does not block the caller's thread.
		virtual STFResult DeliverDataPackets(uint32 connectorID,	VDRStreamingDataPacket * packets,
														 uint32 numPackets, uint32 & acceptedPackets) = 0;

		/// @brief Request more data on an output connector
		/// 
		/// This function is called by the application to keep a pull model stream
		/// in flow.  It is the renderers responsibility to keep a stream flowing in a
		/// pull model.  Therefore the renderer has to request packets, whenever its
		/// input queue falls below a critical level.  If the application is the
		/// "renderer", it is responsible to call RequestDataPackets in this case.
		virtual STFResult RequestDataPackets(uint32 connectorID) = 0;

		/// @brief Provide an allocator for an output connector of the streaming chain
		virtual STFResult ProvideAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator) = 0;

		/// @brief Request the allocator available at the input connector of a streaming chain
		virtual STFResult RequestAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * & allocator) = 0;
	};


#endif // #ifndef IVDRSTREAMING_H
