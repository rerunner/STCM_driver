#ifndef SDL2VIDEORENDERER_H
#define SDL2VIDEORENDERER_H

extern "C"
{
#include "SDL2/SDL.h"
#include <unistd.h>
}

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/Types/STFTime.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"
#include "VDR/Interface/Unit/Video/Decoder/IVDRVideoDecoderTypes.h"


///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

class SDL2VideoRendererUnit : public SharedPhysicalUnit
{
	friend class VirtualSDL2VideoRendererUnit;

protected:
	enum
		{
		SDL2_VIDEO_RENDERER_THREAD_PRIORITY = 0,
		SDL2_VIDEO_RENDERER_THREAD_STACKSIZE = 1,
		SDL2_VIDEO_RENDERER_THREAD_NAME = 2
		};
	uint32 threadPriority;
	uint32 threadStackSize;
	char *threadName;

public:
	SDL2VideoRendererUnit(VDRUID unitID) : SharedPhysicalUnit(unitID) {}

	//
	// IPhysicalUnit interface implementation
	//
	virtual STFResult CreateVirtual(IVirtualUnit * & unit, IVirtualUnit * parent = NULL, IVirtualUnit * root = NULL);

	virtual STFResult Create(uint64 * createParams);
	virtual STFResult Connect(uint64 localID, IPhysicalUnit * source) {STFRES_RAISE_OK;}
	virtual STFResult Initialize(uint64 * depUnitsParams) {STFRES_RAISE_OK;}
};


/// Video Renderer Unit terminates the Video streaming chain (sink)
class VirtualSDL2VideoRendererUnit : public VirtualThreadedStandardStreamingUnit
{
private:
	bool					Preparing; ///< Waiting for stream properties to arrive
	STFSignal				wakeupSignal; ///< This signal is set when the thread should wake up
	STFHiPrec64BitTime		pendingStartTime, pendingEndTime; ///< Do we have pending timestamps?
	bool					startTimeValid, endTimeValid; ///< times start are pending but not yet associated with a frame
	STFHiPrec32BitDuration	frameDuration; ///< The duration of one frame (depends on NTSC/PAL/whatever)

	SequenceHeaderExtension *seqHeaderExtInfo; ///< See IVDRVideoDecoderTypes.h
	SDL_Renderer		*renderer;
	SDL_Window			*screen;
	SDL_Texture		*texture;

	SDL2VideoRendererUnit *physicalSDL2VideoRendererUnit;

	STFHiPrec64BitTime oldTime, pollingTime;
	int32 deltaTime;

protected:

	virtual STFResult Render(const VDRDataRange & range, uint32 & offset);
	virtual STFResult ConfigureRenderer();

	//
	// Data range parsing
	//
	virtual STFResult ParseRanges(const VDRDataRange * ranges, uint32 num, uint32 & range, uint32 & offset);

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
	virtual STFResult ParseStartTime(const STFHiPrec64BitTime & time);
	virtual STFResult ParseEndTime(const STFHiPrec64BitTime & time);
	virtual STFResult ParseCutDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}
	virtual STFResult ParseSkipDuration(const STFHiPrec32BitDuration & duration) {STFRES_RAISE_OK;}

	/// Returns if input data is currently being used for processing.
	virtual bool InputPending(void) {return false;}

public:
	/// Specific constructor.
	/// @param physical: Pointer to interface of corresponding physical unit
	VirtualSDL2VideoRendererUnit (SDL2VideoRendererUnit * physical);
	~VirtualSDL2VideoRendererUnit();

	//
	// IVirtualUnit
	//
	virtual STFResult PreemptUnit(uint32 flags);

	//
	// VirtualUnitCollection pure virtual function overrides
	//
	virtual STFResult Initialize(void);

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
		return STFString("VirtualSDL2VideoRendererUnit ") + STFString(physical->GetUnitID());
	}
#endif
};


#endif // #ifndef SDL2VIDEORENDERER_H
