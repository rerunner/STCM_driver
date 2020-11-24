#ifndef STREAMMIXER_H
#define STREAMMIXER_H

///
/// @brief      Generic Stream Mixer Implementation
///

#include "Device/Interface/Unit/Datapath/IStreamMixer.h"
#include "STF/Interface/Types/STFQueue.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Base/VDRBase.h"
#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"
#include "VDR/Interface/Unit/Audio/IVDRAudioTypes.h"
///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input Control
///////////////////////////////////////////////////////////////////////////////
///
///
///
class StreamMixerInputControl : public ExclusivePhysicalUnit
	{
	friend class VirtualStreamMixerInputControl;

	protected:
		IPhysicalUnit	* physicalMixerInput;

	public:
		StreamMixerInputControl(VDRUID unitID) : ExclusivePhysicalUnit(unitID) {}

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);
		virtual STFResult GetTagIDs (VDRTID * & ids);

	};
					

		 
///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Input Control
///////////////////////////////////////////////////////////////////////////////
///
///
///
class VirtualStreamMixerInputControl : public VirtualStreamingUnit 
	{
	friend class StreamMixerInputControl;

	protected:
		StreamMixerInputControl		*	physicalInput;

	public:
		VirtualStreamMixerInputControl(StreamMixerInputControl * physicalInput) : VirtualStreamingUnit(physicalInput)
			{
			this->physicalInput	= physicalInput;
			}

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void);
		virtual STFResult InternalConfigureTags (TAG * tags);

#if _DEBUG
		// Debugging facilities
		virtual STFString GetInformation(void)
			{
			return STFString("VirtualStreamMixerInputControl ") + STFString(physical->GetUnitID(), 8, 16);
			}
#endif
	};



class VirtualStreamMixerInput;


///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Input Unit
///////////////////////////////////////////////////////////////////////////////

class MixerInputMessageSink : public DispatchedSTFMessageSink
	{
	private:
		DirectSTFMessageSink * targetSink;

	public:
		MixerInputMessageSink(DirectSTFMessageSink * targetSink, STFMessageDispatcher * dispatcher)
			: DispatchedSTFMessageSink(dispatcher)
			{
			this->targetSink = targetSink;
			}

		virtual STFResult ReceiveMessage(STFMessage & message)
			{
			STFRES_RAISE(targetSink->SendMessage(message, false));
			}
	};


/// Physical Stream Mixer Input unit.
/// Connected by a Physical-to-Physical connection with the Physical Stream
/// Mixer unit. Represents one client stream to the Stream Mixer.
class StreamMixerInput : public virtual IStreamMixerInput,
								 public ExclusivePhysicalUnit,
								 public DirectSTFMessageSink
	{
	friend class VirtualStreamMixerInput;

	protected:
		uint32							inputID;				/// ID of this mixer input used for referencing in the node array
		IStreamMixer				*	mixer;				/// Reference to the mixer
		uint32							basePriority,		/// Priority of this input during "normal" speed streaming
											trickPriority;		/// Priority of this input during "trick mode" speed streaming
		bool								useAllocator;		/// Specifies if this input requires an allocator from the mixer
		uint32							freeParameter;		/// Freely assignable parameter (to be forwarded to the Stream Mixer)

		IVDRMemoryPoolAllocator *	allocator;			/// Allocator coming from the mixer

		VirtualStreamMixerInput * 	curVirtualInput;	/// Currently active virtual input unit

		MixerInputMessageSink	* messageSink;			/// Sink to receive messages from mixer
		TriggeredWaitableQueuedSTFMessageProcessingDispatcher	* dispatcher;	/// Dispatcher used to forward messages upstream
		ThreadedSTFMessageDispatcher * threadedDispatcher;							/// Thread based dispatcher based on "dispatcher" above
		VDRMixerInputType				inputType;

		/// Called from the Virtual Stream Mixer Input to signal activation.
		/// The memory pool allocator is then provided to the virtual unit.
		STFResult SetVirtualStreamMixerInput(VirtualStreamMixerInput * unit);

	public:
		StreamMixerInput(VDRUID unitID) : ExclusivePhysicalUnit(unitID)
			{
			allocator			= NULL;
			curVirtualInput	= NULL;
			inputID				= 0;
			mixer					= NULL;
			messageSink			= NULL;
			}
		
		virtual ~StreamMixerInput();

		//
		// IStreamMixerInput interface implementation
		//
		virtual STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

		virtual STFResult GetTagIDs (VDRTID * & ids);
		virtual STFResult InternalConfigureTags (TAG * tags);
		virtual STFResult InternalBeginConfigure();
		virtual STFResult InternalCompleteConfigure();


		// Override from DispatchedSTFMessageSink
		virtual STFResult ReceiveMessage(STFMessage & message);
	};



///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Input Streaming Unit
///////////////////////////////////////////////////////////////////////////////

class StreamMixerInput;

/// Virtual Stream Mixer Input Streaming Unit
/// Standard implementation of a Virtual Mixer Input Streaming Unit. Virtual
/// Mixer Input Units are always the last of a decoding streaming chain.
class VirtualStreamMixerInput : public VirtualStreamingUnit, 
										  public virtual IStreamingClockClient
	{
	friend class StreamMixerInput;

	protected:
		StreamMixerInput		*	physicalInput;		/// Reference to the physical mixer input counterpart
		VDRStreamingState			previousState;		/// Previous streaming state used during state transitions
		VDRStreamingCommand		pendingCommand;	/// Current streaming command to be executed
		int32							pendingParam;		/// Parameter of currently pending streaming command
		IStreamingClock		*	streamingClock;	/// Interface to the Streaming Clock manager of the chain containing this mixer input
		uint32						clockID;				/// The ID assigned by the Streaming Clock Manager for this mixer input
		int32							speed, direction;	/// Current speed and direction of streaming


		UnqueuedInputConnector	inputConnector;	/// input connector

		/// ???
//		STFResult StopStreamingCommand(void);

		/// Receive memory pool allocator from mixer. Called by the physical Stream Mixer Input unit.
		STFResult ReceiveAllocator(IVDRMemoryPoolAllocator * allocator)
			{
			STFRES_RAISE(inputConnector.ProvideAllocator(allocator));
			}

		/// Function receiving messages from the Physical Unit's message dispatcher.
		STFResult MixerNotification(VDRMID message, uint32 param1, uint32 param2);

	public:
		VirtualStreamMixerInput(StreamMixerInput * physicalInput) : VirtualStreamingUnit(physicalInput),
																						inputConnector(0, this)
			{
			this->physicalInput	= physicalInput;

			streamingClock			= NULL;

			AddConnector((IStreamingConnector*)&inputConnector);		// => ID 0
			}

		//
		// IStreamingUnit Streaming Command Handling
		//
		virtual STFResult CompleteConnection(void);
		virtual STFResult PropagateStreamingClock(IStreamingClock * streamingClock);

		virtual STFResult PrepareStreamingCommand(VDRStreamingCommand command, int32 param, VDRStreamingState targetState);
		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);
		virtual STFResult CompleteStreamingCommand(VDRStreamingCommand command, VDRStreamingState targetState);
		virtual STFResult GetStreamTagIDs(uint32 connectorID, VDRTID * & ids);

		//
		// IStreamingUnit functions
		//
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);

		//
		// IStreamingClockClient
		//
		virtual STFResult SetStartupFrame(uint32 frameNumber, const STFHiPrec64BitTime & startTime);
		virtual STFResult GetCurrentStreamTimeOffset(STFHiPrec64BitDuration & systemOffset);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void);

		//
		// IVirtualUnit
		//
		virtual STFResult PreemptUnit (uint32 flags);

#if _DEBUG
		// Debugging facilities
		virtual STFString GetInformation(void)
			{
			return STFString("VirtualStreamMixerInput ") + STFString(physical->GetUnitID(), 8, 16);
			}
#endif
	};



///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualStreamMixerOutput;

class StreamMixerOutput : public ExclusivePhysicalUnit,
								  public virtual IStreamMixerOutput
	{
	friend class VirtualStreamMixerOutput;

	protected:
		// The required physical subunit (interface)
		IStreamMixer		*	streamMixer;	/// Reference to the stream mixer this output is connected to
		uint32					outputID;		/// ID of this Mixer Output, assigned by the Stream Mixer
		STFInterlockedStack	packetStore;	/// Store of empty Streaming Data Packets to be filled and sent by the mixer
		
		VirtualStreamMixerOutput	*	curOutputUnit;	/// Currently active virtual mixer output unit

		void SetVirtualStreamMixerOutput(VirtualStreamMixerOutput * unit)
			{
			this->curOutputUnit = unit;
			}
		
	public:
		StreamMixerOutput(VDRUID unitID) : ExclusivePhysicalUnit(unitID)
			{
			streamMixer = NULL;
			}

		virtual ~StreamMixerOutput();

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
	
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

		//
		// IStreamMixerOutput interface implementation
		//
		virtual STFResult GetEmptyDataPacket(StreamingDataPacket *& packet);
		virtual STFResult ReturnDataPacket(StreamingDataPacket * packet);
		virtual STFResult SendPacket(StreamingDataPacket * packet);
		virtual STFResult MixerNotification(VDRMID message, uint32 param1, uint32 param2);

		//
		// IStreamingDataPacketManager interface implementation
		//

#if _DEBUG
		// Debugging facilities of various base classes.
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
		virtual STFString GetInformation(void)
			{
			return STFString("StreamMixerOutput ") + STFString(GetUnitID(), 8, 16);
			}
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Output Unit
///////////////////////////////////////////////////////////////////////////////

class VirtualStreamMixerOutput : public VirtualStreamingUnit
	{
	friend class StreamMixerOutput;

	private:
		StreamMixerOutput	*	physicalOutput;

	protected:
		
		//
		// "External" connectors
		//
		StreamingOutputConnector	pcmOutputConnector;			/// PCM data output connector
		StreamingPoolAllocator		outputPoolAllocator;			/// Current memory pool allocator, coming from the rendering chain


	public:
		VirtualStreamMixerOutput(StreamMixerOutput * physicalOutput);

		//
		// IStreamingUnit interface implementation
		//
		virtual STFResult ReceivePacket(uint32 connectorID, StreamingDataPacket * packet);
		virtual STFResult SignalPacketArrival(uint32 connectorID, uint32 numPackets);
		virtual STFResult UpstreamNotification(uint32 connectorID, VDRMID message, uint32 param1, uint32 param2);
		virtual STFResult ReceiveAllocator(uint32 connectorID, IVDRMemoryPoolAllocator * allocator);

		virtual STFResult BeginStreamingCommand(VDRStreamingCommand command, int32 param);

		virtual STFResult MixerNotification(VDRMID message, uint32 param1, uint32 param2);

		//
		// ITagUnit interface implementation
		//
		virtual STFResult	InternalUpdate(void);

		//
		// IVirtualUnit
		//
		virtual STFResult PreemptUnit(uint32 flags);

#if _DEBUG
		//
		// IStreamingUnitDebugging functions
		//
		virtual STFString GetInformation(void)
			{
			return STFString("VirtualStreamMixerOutput ") + STFString(physical->GetUnitID(), 8, 16);
			}
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Physical Stream Mixer Unit
///////////////////////////////////////////////////////////////////////////////


/// Used for by asynchronous consumer/producer to exchange information about
/// render frame times.
struct ASyncRenderFrameTime
	{
	uint32					frame0;
	STFHiPrec64BitTime	time0;
	uint32					frame1;
	STFHiPrec64BitTime	time1;
	uint32					frame2;

	ASyncRenderFrameTime()
		{
		frame0	= 0;
		frame1	= 0;
		frame2	= 0;
		}
	};


/// Forward declaration
class VirtualStreamMixer;

/// Physical Stream Mixer Unit
/// Generic implementation of a Stream Mixer. Handles mixing of several
/// input streams by making use of a frame mixer. The Frame Mixer is
/// received during system construction time by a Physical-to-Physical
/// unit reference.
class StreamMixer : public virtual IStreamMixer,
						  public ExclusivePhysicalUnit, 
						  protected STFThread
	{
	friend class VirtualStreamMixer;

	protected:
		IPhysicalUnit					*	physicalFrameMixer;		/// Reference to the specific frame mixer's physical unit
		IVirtualUnit					*	virtualFrameMixer;		/// Reference to the specific frame mixer's virtual unit
		IFrameMixer						*	frameMixer;					/// Reference to the virtual frame mixer's mixer interface

		uint32								registeredInputs;			/// Number of registered mixer inputs

		uint32								registeredOutputs;		/// Number of registered mixer outputs
		uint32								streamingOutputs;			/// Number of currently active ("streaming") outputs

		uint32								totalOutputs;				/// Size of array holding the mixer output nodes
		StreamMixerOutputNode		*	mixerOutputs;				/// Array of mixer output nodes (1 node representing one mixer output)
		StreamingDataPacket			**	outputPackets;				/// Array of pointers to current streaming data packet (1 for each output)

		STFHiPrec64BitDuration			mixerFrameDuration;		/// Duration of one mixer (= output) frame
		volatile uint32					mixerFrameNum;				/// Current mixer (= output) frame number

//??? Does not compile with volatile specified!
		ASyncRenderFrameTime				asyncRenderFrameTime;	/// Current render frame time (coming from renderer)
//		volatile ASyncRenderFrameTime	asyncRenderFrameTime;
		STFMutex								configMutex;				/// Protection during tag configuration
		volatile bool						configWriteLock,			/// Protection during tag configuration
												configReadLock;			/// Protection during tag configuration
		STFSignal							configSignal;				/// Protection during tag configuration
		int									configStackLevel;			/// Protection during tag configuration

		uint32								outputSegmentNumber;		/// Number of current segment being output. Incremented when mixer is stopped & restarted.
		uint32								renderStartFrame;			/// Number of first frame sent at mixing start (??? when is this reset?)

		uint32 lastNum; // temp for debug


#if DIAGNOSTIC_STREAM_MIXER_STATISTICS
		DiagnosticStreamMixerStats		* diagnosticStats;
#endif // DIAGNOSTIC_STREAM_MIXER_STATISTICS

		// STFThread overrides
		virtual void ThreadEntry(void);
		virtual STFResult NotifyThreadTermination(void);

		STFResult SetRenderFrameTime(STFHiPrec64BitTime renderTime, uint32 renderFrame);
		STFResult GetRenderFrameTime(STFHiPrec64BitTime & renderTime, uint32 & renderFrame);

	public:
		StreamMixer(VDRUID unitID);
		virtual ~StreamMixer();

		//
		// ITagUnit interface implementation
		//
		virtual STFResult GetTagIDs(VDRTID * & ids);

		//
		// IVDRBase functions
		//
		virtual STFResult QueryInterface(VDRIID iid, void *& ifp);

		//
		// IPhysicalUnit interface implementation
		//
		virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);
		virtual STFResult Create(uint64 * createParams);
		virtual STFResult Connect(uint64 localID, IPhysicalUnit * source);
		virtual STFResult Initialize(uint64 * depUnitsParams);

		//
		// IStreamMixer implementation
		//
		virtual STFResult RegisterMixerInput(IStreamMixerInput * input,
														 STFMessageSink * inputSink,
														 VDRMixerInputType inputType,
														 uint32 freeParameter,
														 uint32 & inputID);

		virtual STFResult RegisterMixerOutput(IStreamMixerOutput * output, uint32 & outputID);
		virtual STFResult ReceiveAllocator(uint32 outputID, IVDRMemoryPoolAllocator * allocator);

 		virtual STFResult PrepareMixerOutput(uint32 outputID);
		virtual STFResult FlushMixerOutput(uint32 outputID);

		virtual STFResult BeginConfigureStream(uint32 inputID);
		virtual STFResult GetStreamTagIDs (uint32 inputID, VDRTID * & ids);
		virtual STFResult ConfigureStreamTags(uint32 inputID, TAG * tags);
		virtual STFResult InternalUpdateStreamTags(uint32 inputID);
		virtual STFResult CompleteConfigureStream(uint32 inputID);

		virtual STFResult GetStreamStartupInfo(uint32 inputID, StreamingClockClientStartupInfo & info);
		virtual STFResult GetStreamTimeOffset(uint32 inputID, STFHiPrec64BitDuration & systemOffset);
		virtual STFResult SetStreamTimeOffset(uint32 inputID, const STFHiPrec64BitDuration & systemOffset);

		virtual STFResult StartStream(uint32 inputID, uint32 mixFrameNumber, int32 speed, const STFHiPrec64BitTime & startTime);
		virtual STFResult StopStream(uint32 inputID);
		virtual STFResult PrepareStream(uint32 inputID, int32 direction); 
		virtual STFResult StepStream(uint32 inputID, uint32 numFrames);
		virtual STFResult FlushStream(uint32 inputID, int32 mode);

		virtual STFResult ReceivePacket(uint32 inputID, StreamingDataPacket * packet);

		virtual STFResult MixerOutputNotification(uint32 outputID, VDRMID message, uint32 param1, uint32 param2);
	};



///////////////////////////////////////////////////////////////////////////////
// Virtual Stream Mixer Unit
///////////////////////////////////////////////////////////////////////////////

/// Generic Virtual Stream Mixer Unit
class VirtualStreamMixer : public VirtualUnit
	{
	friend class StreamMixer;

	protected:
		StreamMixer * physicalMixer;

	public:
		VirtualStreamMixer(StreamMixer * physicalMixer) : VirtualUnit(physicalMixer)
			{
			this->physicalMixer = physicalMixer;
			}

		//
		// IVDRTagUnit interface implementation override
		//
		virtual STFResult ConfigureTags(TAG * tags);
	};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EVERYTHING DECLARED AFTER THIS LINE IS OBSOLETE AND MUST NOT BE USED ANY MORE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Mixer Input Channel
///////////////////////////////////////////////////////////////////////////////

//!Mixer Input Channel
/*!
	This interface is offered by a (physical) Mixer Unit to allow
	Mixer Input Units to provide and control the flow of data and to
	maintain proper timing.
*/

class MixerInputChannel : public virtual IMixerInputChannel,
								  public VDRBase
	{
	protected:
		uint32 channelID;
		uint32 numPackets;
		uint32 threshold;

		IMixer		*	mixer;
		IMixerInput	*	input;

		STFInterlockedStack	packetStore;

		// We need a special IRQ safe queue here! The Standard STFFixedQueue is not. Change later!
		//STFFixedIRQSafeQueue * queue;
		STFFixedQueue	*	queue;


	public:
		MixerInputChannel(uint32 channelID, 
								uint32 numPackets,
								uint32 queueSize,
								uint32 threshold,
								IMixer * mixer);

		virtual ~MixerInputChannel();

		//
		// IMixerInputChannel
		//

		//??? The following three functions will probably be declared obsolete! Do not use any more:
		virtual STFResult GetEmptyDataPacket(StreamingDataPacket *& packet);
		virtual STFResult ReturnDataPacket(StreamingDataPacket * packet);
		virtual STFResult SendPacket(StreamingDataPacket * packet);		
		
		virtual STFResult SendData(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);
		virtual STFResult RequestDataPackets(void);
		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2);

		virtual STFResult DoCommand(MixerInputChannelCommand command);
		virtual STFResult GetChannelInfo(MixerInputChannelInfo & info);

		//! Set the Mixer input unit which is the source of data for the channel
		virtual STFResult SetSourceInput(IMixerInput * input);

		//
		// IStreamingDataPacketManager interface implementation
		//

#if _DEBUG
		// Debugging facilities of various base classes.
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
		virtual STFString GetInformation(void)
			{
			return STFString("MixerInputChannel ");
			}
#endif
	};


///////////////////////////////////////////////////////////////////////////////
// Mixer Output Streaming Formatter
///////////////////////////////////////////////////////////////////////////////

class MixerOutputChannelStreamingFormatter : public StreamingFormatter
	{
	protected:
		IMixerOutputChannel	*	outputChannel;

		STFResult GetEmptyPacket(StreamingDataPacket * & packet);
		STFResult SendPacket(StreamingDataPacket * packet);
		STFResult FlushPacket(StreamingDataPacket * packet);

	public:
		MixerOutputChannelStreamingFormatter(void);
		virtual ~MixerOutputChannelStreamingFormatter(void);

		//
		// Will _not_ AddRef mixer output, assumes liveness of connection
		// during livetime of streaming unit
		//
		STFResult SetMixerOutputChannel(IMixerOutputChannel * outputChannel)
			{
			this->outputChannel = outputChannel;
			STFRES_RAISE_OK;
			}
	};


///////////////////////////////////////////////////////////////////////////////
// Mixer Output Channel
///////////////////////////////////////////////////////////////////////////////

class MixerOutputChannel : public virtual IMixerOutputChannel,
									public VDRBase
	{
	protected:
		uint32 channelID;
		IMixer * mixer;

		IVDRMemoryPoolAllocator * allocator;

		IMixerOutput * output;

		VDRMemoryBlock *	curMemoryBlock;
		VDRDataRange		curRange;

		//??? It is possible that this will be removed later and replaced by MixerOutputChannel
		// inheriting from StreamingFormatter. In this case, however, the functions
		// GetEmptyDataPacket, ReturnDataPacket and SendPacket will have to be removed from the
		// definition of IMixerOutputChannel.
		MixerOutputChannelStreamingFormatter	formatter;

	public:

		MixerOutputChannel(uint32 channelID, IMixer * mixer, uint8 rangesThreshold);
		virtual ~MixerOutputChannel();


		//
		// IMixerOutputChannel
		//
		virtual STFResult GetCurrentDataRange(VDRDataRange *& range);
		virtual STFResult PutCurrentDataRange(void);
		virtual STFResult PutDataRange(const VDRDataRange & range)
			{
			STFRES_RAISE(formatter.PutRange(range));
			}
		
		//??? The following three functions will probably be declared obsolete! Do not use them any more:
		virtual STFResult GetEmptyDataPacket(StreamingDataPacket *& packet);
		virtual STFResult ReturnDataPacket(StreamingDataPacket * packet);
		virtual STFResult SendPacket(StreamingDataPacket * packet);

		virtual STFResult UpstreamNotification(VDRMID message, uint32 param1, uint32 param2);

		virtual STFResult ProvideAllocator(IVDRMemoryPoolAllocator * allocator);

		virtual STFResult SetTargetOutput(IMixerOutput * output);

		//
		// IStreamingDataPacketManager interface implementation
		//

#if _DEBUG
		// Debugging facilities of various base classes.
		virtual STFResult PrintDebugInfo(uint32 id = LOGID_ERROR_LOGGING);
		virtual STFString GetInformation(void)
			{
			return STFString("MixerOutputChannel ");
			}
#endif
	};

#endif	// #ifndef STREAMMIXER_H
