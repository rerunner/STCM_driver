#ifndef SDL2VIDEORENDERER_H
#define SDL2VIDEORENDERER_H

#include "VDR/Source/Streaming/StreamingUnit.h"
#include "VDR/Source/Streaming/BaseStreamingUnit.h"
#include "VDR/Source/Streaming/StreamingFormatter.h"
#include "VDR/Source/Unit/PhysicalUnit.h"
#include "STF/Interface/STFSynchronisation.h"
#include "VDR/Source/Streaming/StreamingDiagnostics.h"

extern "C"
{
#include "SDL2/SDL.h"
}

///////////////////////////////////////////////////////////////////////////////
// Streaming Terminator Unit
///////////////////////////////////////////////////////////////////////////////

class SDL2VideoRendererUnit : public SharedPhysicalUnit
{
	friend class VirtualSDL2VideoRendererUnit;

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


/// Video Renderer Unit terminates the Video streaming chain (sink)
class VirtualSDL2VideoRendererUnit : public VirtualNonthreadedStandardStreamingUnit
{
private:
	struct WindowProperties_s
		{
		int width;
		int height;
		} WindowProperties;

	struct ScreenProperties_s
		{
		int width;
		int height;
		} ScreenProperties;

	bool	Preparing;

protected:

	virtual STFResult Render(const VDRDataRange & range, uint32 & offset);
	virtual STFResult ConfigureRenderer();

	//
	// Data range parsing
	//
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
