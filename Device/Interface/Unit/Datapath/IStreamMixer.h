#ifndef ISTREAMMIXER_H
#define ISTREAMMIXER_H

///
/// @brief      Mixer Input & Output Interfaces
///

#include "VDR/Source/Streaming/IStreaming.h"
#include "VDR/Source/Streaming/IStreamingClocks.h"
#include "VDR/Interface/Unit/Datapath/VDRMixerInputTypes.h"


//@{
///////////////////////////////////////////////////////////////////////////////
/// Mixer Notifications (sent from Stream Mixer to Mixer Input units)
///////////////////////////////////////////////////////////////////////////////
/// Each of these messages concern one of the inputs.
/// VDRMID_STRM_MIXER_STOPPED				: Stopping finished (when input requested a stop of mixing, e.g. at shutdown)
/// VDRMID_STRM_MIXER_PREPARED			: Preparation finished (execution of "BEGIN" command for input)
/// VDRMID_STRM_MIXER_STEPPED				: Single step finished (execution of "STEP" command for input)
/// VDRMID_STRM_MIXER_FLUSHED				: Flushing finished (execution of "FLUSH" command for input)
/// VDRMID_STRM_MIXER_PACKET_REQUEST	: Request more data
/// VDRMID_STRM_MIXER_STARVATION       : No more data available for this input
/// VDRMID_STRM_MIXER_START_POSSIBLE   : Enough data available for input to allow "DO" command
/// VDRMID_STRM_MIXER_START_REQUIRED   : Queues full, "DO" command required
/// VDRMID_STRM_MIXER_SYNCH_REQUEST    : Trigger resynchronisation for this input (goes to Streaming Clock Manager)

static const VDRMID VDRMID_STRM_MIXER_BASE = 0x0004e000;

static const VDRMID VDRMID_STRM_MIXER_STOPPED			= VDRMID_STRM_MIXER_BASE + 1;
static const VDRMID VDRMID_STRM_MIXER_PREPARED			= VDRMID_STRM_MIXER_BASE + 2;
static const VDRMID VDRMID_STRM_MIXER_STEPPED			= VDRMID_STRM_MIXER_BASE + 3;
static const VDRMID VDRMID_STRM_MIXER_FLUSHED			= VDRMID_STRM_MIXER_BASE + 4;
static const VDRMID VDRMID_STRM_MIXER_PACKET_REQUEST	= VDRMID_STRM_MIXER_BASE + 5;
static const VDRMID VDRMID_STRM_MIXER_STARVATION		= VDRMID_STRM_MIXER_BASE + 6;
static const VDRMID VDRMID_STRM_MIXER_START_POSSIBLE	= VDRMID_STRM_MIXER_BASE + 7;
static const VDRMID VDRMID_STRM_MIXER_START_REQUIRED	= VDRMID_STRM_MIXER_BASE + 8;
static const VDRMID VDRMID_STRM_MIXER_SYNCH_REQUEST	= VDRMID_STRM_MIXER_BASE + 9;

//@}



///////////////////////////////////////////////////////////////////////////////
// Stream Mixer Input Interface
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_STREAM_MIXER_INPUT = 0x8000006f;

/// Interface of a Stream Mixer Input
class IStreamMixerInput : public virtual IVDRBase
	{
	public:
		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Mixer Output Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_MIXER_OUTPUT = 0x80000029;


/// Interface of a Stream Mixer Output
class IStreamMixerOutput : public virtual IStreamingDataPacketManager, 
									public virtual IVDRBase
	{
	public:
		/// Send single Streaming Data Packet to Mixer
		virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;

		/// Mixer notification, done on behalf of the mixer thread, thus
		/// should not take a long time to complete.
		virtual STFResult MixerNotification(VDRMID message, uint32 param1, uint32 param2) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Stream Mixer Interface
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_STREAM_MIXER = 0x8000006e;

/// Interface of a generic Stream Mixer
class IStreamMixer : public virtual IVDRBase
	{
	public:
		//
		// Mixer Input registration
		//

		/// Called by the Mixer Inputs to register themselves.
		/// Each Mixer Input registers with a STFMessageSink. Usually, this is a
		/// dispatched message sink based on a dedicated thread for dispatching
		/// the messages. This way, Mixer Notifications can be forwarded upstream
		/// without blocking the mixer's processing thread.
		virtual STFResult RegisterMixerInput(IStreamMixerInput * input,
														 STFMessageSink * inputSink,
														 VDRMixerInputType inputType,
														 uint32 freeParameter,
														 uint32 & streamID) = 0;

		//
		// Mixer Output registration
		//
		/// Called by a Mixer Output unit to register itself and receive an output ID
		virtual STFResult RegisterMixerOutput(IStreamMixerOutput * output, uint32 & outputID) = 0;
		/// Called by a Mixer Output unit to provide a memory pool allocator
		virtual STFResult ReceiveAllocator(uint32 outputID, IVDRMemoryPoolAllocator * allocator) = 0;
		/// Called when mixer output is starting up (executes a BEGIN command)
		virtual STFResult PrepareMixerOutput(uint32 outputID) = 0;
		/// Called when mixer output is executing a FLUSH command
		virtual STFResult FlushMixerOutput(uint32 outputID) = 0;

		//
		// Tag handling
		//
		/// Begin configuration for a mixer input stream
		virtual STFResult BeginConfigureStream(uint32 streamID) = 0;
		/// Get Tag Type IDs for a mixer input stream
		virtual STFResult GetStreamTagIDs (uint32 streamID, VDRTID * & ids) = 0;
		/// Tag configuration for a mixer input stream
		virtual STFResult ConfigureStreamTags(uint32 streamID, TAG * tags) = 0;
		/// Tag updating for a mixer input stream
		virtual STFResult InternalUpdateStreamTags(uint32 streamID) = 0;
		/// Completion of tag configuration for a mixer input stream
		virtual STFResult CompleteConfigureStream(uint32 streamID) = 0;

		//
		// Snychronisation Startup handling
		//
		/// Get startup timing information for a particular mixer input stream
		virtual STFResult GetStreamStartupInfo(uint32 streamID, StreamingClockClientStartupInfo & info) = 0;
		/// Get stream's current stream time as speed adapted delta (used for resynchronisation and to
		/// determine the actual stream time)
		virtual STFResult GetStreamTimeOffset(uint32 streamID, STFHiPrec64BitDuration & systemOffset) = 0;
		/// Set stream's current stream time as speed adapted delta (used for resynchronisation)
		virtual STFResult SetStreamTimeOffset(uint32 streamID, const STFHiPrec64BitDuration & systemOffset) = 0;

		//
		// Data Flow Control
		//
		/// Start mixing for a certain mixer input stream. Called on behalf of the Streaming Clock Manager,
		/// when it has determined the start frame for all input streams.
		/// @param mixFrameNumber [in] start mixer frame number
		/// @param speed [in] desired streaming speed
		virtual STFResult StartStream(uint32 streamID, uint32 mixFrameNumber, int32 speed, const STFHiPrec64BitTime & startTime) = 0;
		/// Stop mixing for a certain mixer input stream
		virtual STFResult StopStream(uint32 streamID) = 0;
		/// Prepare mixing for a certain mixer input stream.
		/// @param direction [in] direction of playback
		virtual STFResult PrepareStream(uint32 streamID, int32 direction) = 0; 
		/// Execute a single step for a certain mixer input stream.
		/// @param numFrames [in] number of frames to step
		virtual STFResult StepStream(uint32 streamID, uint32 numFrames) = 0;
		///  Flush and reset a mixer input stream
		/// @param mode [in] : currently unused
		virtual STFResult FlushStream(uint32 streamID, int32 mode) = 0;

		virtual STFResult ReceivePacket(uint32 streamID, StreamingDataPacket * packet) = 0;

		//
		// Notification from renderers
		//

		/// Receive notifications from the Mixer Output units.
		/// The Mixer can have more than one output. One of the outputs must be the master
		/// that determines the delay of the rendering chain. 
		virtual STFResult MixerOutputNotification(uint32 outputID, VDRMID message, uint32 param1, uint32 param2) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Stream Mixer Client Node
///////////////////////////////////////////////////////////////////////////////

/// This value designates that frames of a mixer input will not be considered for mixing
static const uint32 INFINITE_FRAME_NUMBER = 0xffffffff;


/// Size of the notification queue inside the StreamMixerInputNode
/// Must be a power of 2!
#define STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE 64
#define STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_MASK (STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE - 1)


/// State of a mixer input in the streaming startup phase (= while input chain's state being VDR_STRMSTATE_READY)
enum MixInputStartupState
	{
	MIXSS_INITIAL,				///< Initial state (node is in this state while mixing is running)
	MIXSS_NOT_ENOUGH_DATA,	///< There is not enough data yet to start mixing for the input
	MIXSS_SUFFICIENT_DATA,	///< There is sufficient data queued up to start mixing for the input
	MIXSS_FULL					///< The queues are full, mixing has to be started ASAP
	};

enum MixInputDirection
	{
	MID_UNKNOWN,		///< Direction not set for this stream
	MID_FORWARD,		///< Forward (times increasing)
	MID_BACKWARD		///< Backward (times decreasing)
	};



/// Stream Mixer Input Node
/// Used to store information about a mixer input stream needed by the Stream Mixer in the domain of the Frame Mixer.
/// One such node is created for each mixer input registering at the Stream Mixer.
/// The structure can be extended with frame mixer specific information by inheritance.
struct StreamMixerInputNode
	{
	IVDRMemoryPoolAllocator * allocator;

	IStreamMixerInput	*	input;						/// Reference to the mixer input this node belongs to
	STFMessageSink		*	inputSink;					/// Reference to the message sink of the mixer input, in order to send messages to it
	VDRMixerInputType		inputType;
	uint32					freeParameter;				/// Freely assignable configuration parameter (coming from board config of StreamMixerInput)

	bool                 startStreamTimeValid;   /// The start time in this structure is valid
	STFHiPrec64BitTime	startStreamTime;			/// Stream time of the start of the input stream, later the adapted during resynchronisation
	STFHiPrec64BitTime	reqStartStreamTime;		/// New adapted start stream time requested by the Straming Clock Manager during resynchronisation
	uint32					startFrameNumber;			/// Mixer frame number of the start frame. Set to INFINITE if stream not active. 
																/// At sync startup set by the Streaming Clock Manager.
	uint32					frameNumber;				/// Current mixer frame number for this input (used to compare with the mixer's current frame number)

	uint32					speed;						/// Current speed of the input stream.

	// command notification flags (From mixer input to stream mixer)
	//??? Could be reworked to be a flag set
	bool						commandStop,				/// Stop is requested for this input stream
								commandResynch,			/// Resynchronisation is requested for this input stream
								commandPrepare;			/// Preparation is requested for this input stream
	bool						configurePending;			/// A tag configuration is pending for this input stream

	// up stream notification flags (from frame mixer to stream mixer)
	//??? Could be reworked to be a flag set
	bool						starvation,					/// Signal starvation for this input stream
								packetRequest,				/// Signal packet request for this input stream
								packetBounced;				/// Notify that a packet has bounced, thus a packet Request is required
														
	MixInputStartupState	startupState;				/// State used to determine streaming startup condition for this input stream
	MixInputDirection		direction;					/// Direction of this stream

	STFHiPrec64BitTime	lastTimedMessageTime;	/// This is the time of the last timed message (group/segment start/end notification)

	uint32					receivedStreamFrames;	/// # of frames we received on this input so far (needed for startup)	

	protected:
		/// Queue of pending timed upstream notifications (group/segment start/end)
		struct QueueEntry
			{
			STFHiPrec64BitTime	dueTime;
			STFMessage				message;
			} pendingNotifications[STREAMMIXERINPUTNODE_NOTIFICATIONQUEUE_SIZE];

		uint32				notificationTail;			/// Queue entry which is due next
		uint32				notificationHead;			/// next queue entry to fill

	public:
		// Constructor to reset fields
		StreamMixerInputNode(void);

		uint32 AvailNotificationQueue(void);

		/// Insert a pending notification into the queue
		STFResult InsertPendingNotification(VDRMID message, const STFHiPrec64BitTime & dueTime, uint32 param1, uint32 param2);

		STFResult GetFirstDueMessage(const STFHiPrec64BitTime & time, STFMessage & message);

		STFResult ClearNotificationQueue(void);

		
		/// Set the input type of this node (can be overridden for input type specific initialisation)
		virtual STFResult SetInputType(VDRMixerInputType inputType) { this->inputType = inputType; STFRES_RAISE_OK; }

		/// Set the freely assignable parameter coming from the board config of the corresponding StreamMixerInput
		virtual STFResult SetFreeParameter(uint32 freeParameter) { this->freeParameter = freeParameter; STFRES_RAISE_OK; }
	};



/// Stream Mixer Output Node
/// Used to store information about a mixer output needed by the Stream Mixer
/// in the domain of the Frame Mixer. One such node is created for each mixer output
/// registering at the Stream Mixer.
/// The structure can be extended with frame mixer specific information by inheritance.
struct StreamMixerOutputNode
	{
	IVDRMemoryPoolAllocator *	allocator;				/// Pool allocator coming from the Rendering chain connected to this mixer output
	IStreamMixerOutput		*	output;					/// Reference to the mixer output unit

	bool								streaming,				/// Indicates that this output is ready for streaming
										firstOutputPacket;	/// Indicates that the next packet to be output is the first of a new segment
	bool								renderStarted;			/// Indicates that the connected render has already started (we have received SEGMENT_START back)

	bool								commandFlush,			/// Tells the Stream Mixer's thread to flush the output
										commandPrepare;		/// Tells the Stream Mixer's thread to prepare the output

	StreamMixerOutputNode(void);
	};

///////////////////////////////////////////////////////////////////////////////
// Frame Mixer Interface
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_FRAME_MIXER = 0x80000070;


/// Startup request that the mixer can return on reception of data for one input
/// This is used in the streaming startup phase to generate the startup messages
/// upstream, that will trigger the start of streaming in the input chain.
enum StreamMixerStartupRequest
	{
	MIXSUPREQ_NONE,				///< Nothing needs to be done
	MIXSUPREQ_START_POSSIBLE,	///< A VDRMID_STRM_START_POSSIBLE must be generated for the given input
	MIXSUPREQ_START_REQUIRED	///< A VDRMID_STRM_START_REQUIRED must be generated for the input
	};


/// Interface of a Frame Mixer
/// Frame Mixers are always specific for their purpose and targetted HW platform.
class IFrameMixer : virtual public IVDRBase
	{
	public:
		/// @brief Get a input node info structure for a given Stream Mixer Input
		virtual STFResult GetInputNode(uint32 inputID, StreamMixerInputNode * & node) = 0;

		/// @brief Called by the Stream Mixer to send data of a client Stream Mixer Input.
		/// If the client input chain is in the preparation phase, this call will return
		/// the request to send one of the start messages. Only the specific frame
		/// mixer can decide what amount of data is needed to trigger a start message!
		virtual STFResult SendInputPacket(uint32 inputID, StreamingDataPacket * packet, StreamMixerStartupRequest & req) = 0;

		/// @brief Get the duration of one mixer frame (used for timing calculations in the Stream Mixer)
		virtual STFResult GetFrameDuration(STFHiPrec64BitDuration & mixerFrameDuration) = 0;

		/// @brief Mix one mixer frame. Per mixer output one packet will be handed in.
		virtual STFResult MixFrame(StreamingDataPacket ** packets) = 0;

		/// @brief Called by the Stream Mixer to provide a Memory Pool Allocator for a specific output
		virtual STFResult ReceiveAllocator(uint32 outputID, IVDRMemoryPoolAllocator * allocator) = 0;

		/// @brief Prepare a stream for mixing (e.g. provide allocator to input)
		virtual STFResult PrepareStream(uint32 inputID) = 0;

		/// @brief Step a frame a certain amount of time
		virtual STFResult StepStream(uint32 inputID, uint32 numFrames) = 0;

		/// @brief Flush all pending input data of a given client stream
		virtual STFResult FlushStream(uint32 inputID, int32 mode) = 0;

		/// @brief Get the supported tags from the frame mixer
		virtual STFResult GetStreamTagIDs (uint32 inputID, VDRTID * & ids) = 0;

		/// @brief Configure tags of one input stream
		virtual STFResult ConfigureStreamTags(uint32 inputID, TAG * tags) = 0;

		/// Internal Update function to program just the required changes
		virtual STFResult InternalUpdateStreamTags(uint32 streamID) = 0;
//		/// Complete configuration of one input stream
//		virtual STFResult CompleteConfigureStream(uint32 inputID) = 0;

		/// @brief Forward timing information from the renderer to the frame mixer
		/// The renderer periodically sends an upstream message to the stream mixer, containing these information. The frame mixer also
		/// needs them to calculate time stamps. This funciton is called form the stream mixer every time the stream mixer
		/// receives this information from the renderer.
		/// @param renderTime [in] current renderer time
		/// @param renderFrame [in] current frame number of the renderer
		/// @returns Error code
		/// @retval  STFRES_OK no error
		virtual STFResult SetRendererInformation(const STFHiPrec64BitTime & renderTime, uint32 renderFrame) = 0;

		/// @brief Retrieve the current stream time at the input
		/// This is either the last valid timestamp that has been received (start or end) or the last valid timestamp plus a number of 
		/// frame durations.
		virtual STFResult GetCurrentInputStreamTime(uint32 inputID, STFHiPrec64BitTime & inputTime) = 0;

		/// @brief Called by the Stream Mixer to start a frame mixer
		virtual STFResult BeginOutput(uint32 outputID) = 0;

		/// @brief Called by the Stream Mixer to flush a frame mixer
		virtual STFResult FlushOutput(uint32 outputID) = 0;
			

	};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EVERYTHING DECLARED AFTER THIS LINE IS OBSOLETE AND MUST NOT BE USED ANY MORE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Mixer Input Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_MIXER_INPUT = 0x8000002b;


class IMixerInput	: public virtual IVDRBase
	{
	public:
		//! Send Upstream Notification
		virtual STFResult MixerUpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;
	};

class IMixerOutput : public virtual IStreamingDataPacketManager, public virtual IVDRBase
	{
	public:
		//! Send single Streaming Data Packet to Mixer
		virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;
	};




///////////////////////////////////////////////////////////////////////////////
// Mixer Input Channel Command
///////////////////////////////////////////////////////////////////////////////

enum MixerInputChannelCommand
	{
	MIXIN_CMD_BEGIN,
	MIXIN_CMD_END,
	MIXIN_CMD_START,
	MIXIN_CMD_STOP,
	MIXIN_CMD_PAUSE,
	MIXIN_CMD_FLUSH
	};



///////////////////////////////////////////////////////////////////////////////
// Mixer Channel Info
///////////////////////////////////////////////////////////////////////////////

class MixerInputChannelInfo
	{
	public:
		STFHiPrec64BitTime	channelDelay;	// The time it takes from input to output
	};



///////////////////////////////////////////////////////////////////////////////
// Mixer Channel Input Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_MIXER_INPUT_CHANNEL = 0x80000027;


//!Mixer Channel Input Interface
/*!
	This interface is offered by a (physical) Mixer Unit to allow
	Mixer Input Units to provide and control the flow of data and to
	maintain proper timing.

	The channel is queuing the data packets, and is also managing
	data packets for the corresponding Mixer Input Unit.
*/

class IMixerInputChannel : public virtual IStreamingDataPacketManager, public virtual IVDRBase
	{
	public:

		//! Send single Streaming Data Packet to Mixer
		// ??? Attention: This function will probably be declared obsolete! Do not use any more:
		virtual STFResult SendPacket(StreamingDataPacket * packet) = 0;
		
		/// Send individual data ranges to the Mixer Input Channel
		virtual STFResult SendData(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset) = 0;

		//! Send Mixer Channel Control Command to Mixer
		virtual STFResult DoCommand(MixerInputChannelCommand command) = 0;

		//! Get Channel Information
		virtual STFResult GetChannelInfo(MixerInputChannelInfo & info) = 0;

		//! Called by the Mixer Unit to request packets from the queue.
		virtual STFResult RequestDataPackets(void) = 0;

		//! Receive upstream notifications from the mixer unit
		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;

		//! Set the Mixer input unit which is the source of data for the channel
		virtual STFResult SetSourceInput(IMixerInput * input) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Mixer Channel Output Interface
///////////////////////////////////////////////////////////////////////////////


static const VDRIID VDRIID_MIXER_OUTPUT_CHANNEL = 0x80000028;


class IMixerOutputChannel : public virtual IVDRBase,
									 public virtual IMixerOutput
	{
	public:
		/// Get the currently active data range to be filled with data
		virtual STFResult GetCurrentDataRange(VDRDataRange *& range) = 0;
		
		/// Put the current data range into a Streaming Data Packet to be sent downstream
		virtual STFResult PutCurrentDataRange(void) = 0;
		
		/// Put an explicit data range into a Streaming Data Packet to be sent downstream
		virtual STFResult PutDataRange(const VDRDataRange & range) = 0;

		//! Provide an allocator to the attached output connector
		virtual STFResult ProvideAllocator(IVDRMemoryPoolAllocator * allocator) = 0;

		//! Set  Mixer output receiving the data of this channel
		virtual STFResult SetTargetOutput(IMixerOutput * output) = 0;

		//! Notification from the corresponding Mixer Output Unit, e.g. to request packets
		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2) = 0;
	};



///////////////////////////////////////////////////////////////////////////////
// Mixer Interface
///////////////////////////////////////////////////////////////////////////////

static const VDRIID VDRIID_MIXER = 0x80000026;

class IMixer : public virtual IVDRBase
	{
	public:
		virtual STFResult GetInputChannel(uint32 channelID, IMixerInputChannel *& channel) = 0;
		virtual STFResult GetOutputChannel(uint32 channelID, IMixerOutputChannel *& channel) = 0;

		//! This function is only to be called from a MixerInputChannel
		virtual STFResult ReceivePacket(uint32 channelID, StreamingDataPacket * packet) = 0;

		virtual STFResult SendData(uint32 channeldID, const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset) = 0;

		//! This function is only to be called from a MixerInputChannel
		virtual STFResult SignalPacketArrival(uint32 channelID, uint32 numPackets) = 0;

		//! This function is only to be called from a MixerOutputChannel
		virtual STFResult UpstreamNotification(uint32 channelID, VDRMID message, uint32 param1, uint32 param2) = 0;
	};


#endif	// #ifndef IMIXER_H
