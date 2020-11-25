///
///	@brief	VDR Video Display public interfaces definitions
///

#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "STF/Interface/Types/STFResult.h"
#include "STF/Interface/Types/STFTime.h"
#include "VDR/Interface/Base/IVDRBase.h"
#include "VDR/Interface/Unit/IVDRTags.h"
#include "VDR/Interface/Unit/Video/Display/IVDRVideoDisplayTypes.h"
#include "VDR/Interface/Unit/IVDRUnit.h"

///////////////////////////////////////////////////////////////////////////////
// IVDRGfxBitmap Interface
///////////////////////////////////////////////////////////////////////////////

/// IVDRGfxBitmap interface ID
static const VDRIID VDRIID_VDR_GFX_BITMAP = 0x0000004b;


/*!
	@brief IVDRGfxBitmap Interface definition
*/

class IVDRGfxBitmap : public virtual IVDRBase
	{
	public:

		/*!
			Get bitmap description.
			@param bmpDesc: Pointer to VDRGfxBitmapDesc structure. Upon successful return, contains bitmap description data.
		*/
		virtual STFResult GetBitmapDesc(VDRGfxBitmapDesc* bmpDesc) = 0;

		/*!
			Get bitmap description.
			@param bmpPix: Pointer to VDRGfxBitmapPixels structure. Upon successful return, contains data ranges pointing to
						memory where pixel data are stored.
		*/
		virtual STFResult GetBitmapPixels(VDRGfxBitmapPixels* bmpPix) = 0;

				/*!
			Get bitmap description.
			@param bmpDesc: Pointer to VDRGfxBitmapDesc structure. Upon successful return, contains bitmap description data.
		*/
		virtual STFResult SetBitmapDesc(VDRGfxBitmapDesc* bmpDesc) = 0;


	};



///////////////////////////////////////////////////////////////////////////////
// IVDRGraphics2D Interface
///////////////////////////////////////////////////////////////////////////////


/// Public Unit ID of 2D Graphics Unit
static const VDRUID VDRUID_GRAPHICS_2D_STREAM	= 0x0000000c;
static const VDRUID VDRUID_GRAPHICS_2D_PROXY = 0x0000002c;
/// Public Unit ID of VBI Proxy Unit
static const VDRUID VDRUID_VBI_PROXY = 0x0000006b;




/// IVDRGraphics2D interface ID
static const VDRIID VDRIID_VDR_GRAPHICS_2D = 0x0000001a;

/*!
	@brief Graphics 2D Interface definition
*/

class IVDRGraphics2D : public virtual IVDRBase
	{
	public:

		/*!
			Called to start drawing sequence. @see EndDrawing
		*/
		virtual STFResult BeginDrawing(void) = 0;

		/*!
			Called to mark an end of drawing sequence. Although there is no firm limitation of how long drawing sequence
			should be, it typically represents drawing of one bitmap. All the bitmap blits to same bitmap should be inside
			single sequence so that blitting can be properly optimized. @see BeginDrawing
		*/

		virtual STFResult EndDrawing(void) = 0;

		/*!
			When colorFormats is NULL, it does nothing and returns number of formats available. This value should be used
			to allocate enough memory to hold all the color format entries.
			When colorFormats is not NULL, fills out memory pointed to by colorFormats with an array of VDRGfxColorFormat values
			and also returns number of formats available. Application is responsible for allocating enough memory to hold
			all the entries.
		*/
		virtual uint32 GetColorFormats(VDRGfxColorFormat* colorFormats) = 0;


		/*!
			Allocates memory to hold pixel data of the specified width, height and color format, and returns pointer to
			IVDRGfxBitmap inteface (parameter bmp) that can be used to access bitmap data. Bitmap is deallocated when
			IVDRGfxBitmap interface is released. @see IVDRBase.
		*/
		virtual STFResult CreateBitmap(IVDRGfxBitmap** bmp, int32 width, int32 height, VDRGfxColorFormat colorFormat) = 0;

		/*!
			In destBmp, fills rectangle rect with fillColor value.
		   Content of fillColor depends on destination bitmap color format.
			@param rect
			@param fillColor:
				@li for destination bitmaps with non-CLUT color format: 24 bit RGB value
				@li destination bitmaps with non-CLUT color format with per-pixel alpha: 32 bit ARGB value
				@li destination bitmaps with CLUT color format: index into Color LookUp Table (CLUT)
			@param destBmp
		*/
		virtual STFResult FillRectangle(VDRGfxRect* rect, VDRGfxColor fillColor, IVDRGfxBitmap* destBmp) = 0;


		/*!
			In destBmp, draws a border borderLines thick inside rectangle rect. If borderOnly is FALSE, fills remaining area
			inside rectangle rect with fillColor value, otherwise leavs it transpartent. Color of top-left edges is defined
			by borderTopLeftColor, and of bottom-left edges by borderBottomRightColor.
			@param rect
			@param fillColor: same as fillColor parameter in FillRectangle()
			@param borderOnly
			@param borderLines
		   @param borderTopLeftColor: same as fillColor parameter in FillRectangle()
			@param borderBottomRightColor: same as fillColor parameter in FillRectangle()
			@param destBmp
		*/
		virtual STFResult FillRectangleWithBorder(VDRGfxRect* rect, VDRGfxColor fillColor, bool borderOnly, int32 borderLines,
														VDRGfxColor borderTopLeftColor, VDRGfxColor borderBottomRightColor, IVDRGfxBitmap* destBmp) = 0;

		/*!
			In destBmp, outputs text pointed to by text variable, with font and textColor. Text will be rendered clipped
			by rectangle rect, and will appear on (textX, textY) offset from top left corner of rectangle. The rectangle is
			filled by rectColor.
			@param text
			@param textX
			@param textY
			@param font
			@param textColor: same as fillColor parameter in FillRectangle()
			@param rect
			@param rectColor: same as fillColor parameter in FillRectangle()
			@param destBmp
		*/
		virtual STFResult FillTextRectangle(uint16* text, int32 textX, int32 textY, VDRGfxFont* font, VDRGfxColor textColor,
												VDRGfxRect* rect, VDRGfxColor rectColor, IVDRGfxBitmap* destBmp) = 0;

		/*!
			Blits srcRect area of srcBmp to destRect area of destBmp, performing blending of source and destination bitmap defined
			by alpha. Value 255 for alpha means only source bitmap pixels are visible, 0 means only destination bitmap pixels
			are visible). Different color formats for destBmp and srcBmp are supported, with following limitation: when destBmp has
			CLUT color format, srcBmp must have the same color format.
			@param srcBmp
			@param destBmp
			@param srcRect
			@param destRect
			@param alpha
		*/
		virtual STFResult BlitBitmap(IVDRGfxBitmap* srcBmp, IVDRGfxBitmap* destBmp, VDRGfxRect* srcRect, VDRGfxRect* destRect, uint8 alpha) = 0;


		/*!
			Starts set of bitmaps that will be displayed upon return of FinishAndShowDisplayBitmapsSet() function. Display bitmaps set
			is formed by calling DisplayBitmap() for every bitmap that should be displayed. When all the bitmaps are added to set,
			FinishAndShowDisplayBitmapsSet() has to be called to signal end of display set to the driver, and to request showing display
			bitmaps set on the screen. This bitmap set entirely replaces currently displayed bitmap set (if any).
			@see FinishAndShowDisplayBitmapsSet, DisplayBitmap
		*/
		virtual STFResult StartDisplayBitmapsSet(void) = 0;


		/*!
			Singnals end of display bitmaps set and shows it on the screen. @see StartDisplayBitmapsSet, DisplayBitmap
		*/
		virtual STFResult FinishAndShowDisplayBitmapsSet(void) = 0;


		/*!
			Adds srcRect area of dispBmp bitmap to the set of currently displayed bitmap. This srcRect source area will be displayed
			inside destRect area of the screen. The change will take effect on next VBLANK following this function call.
			Wheather dispBmp will actually be visible or not depends on Video Mixer settings.
			@param dispBmp
			@param srcRect
			@param destRect: Destination rectangle, relative to output screen size (can be obtained by IVDRVideoMixer::GetInputSize()).
			@param alpha: Opacity value for this bitmap
			@param zOrder: Position of this bitmap on z-axis, relative to other bitmaps within the same display set (set of displayed
						bitmaps enclosed with StartDisplaySet() and FinishAndShowDisplaySet()) @see StartDisplaySet, FinishAndShowDisplaySet)
		*/
		virtual STFResult DisplayBitmap(IVDRGfxBitmap* dispBmp, VDRGfxRect* srcRect, VDRGfxRect* destRect, uint8 alpha, uint32 zOrder) = 0;


		/*!
			Sets srcRect area of bmp bitmap as one that is currently used for cursor. This source area will be displayed inside
			destRect area of the screen. The change will take effect on next VBLANK following this function call.
			@param cursorBmp
			@param srcRect
			@param destRect: Destination rectangle, relative to output screen (can be obtained by IVDRVideoMixer::GetInputSize()).
		*/
		virtual STFResult SetCursorBitmap(IVDRGfxBitmap* cursorBmp, VDRGfxRect* srcRect, VDRGfxRect* destRect) = 0;

		/*!
			Determines if the bitmap is free to be used for modifications. For instance, bitmap should not be modified if it is
			currently on display, or it is part of the blit that is still in progress. This function should be called prior to
			modifying bitmap content (writing to bitmap pixel data) to make sure no visual artifacts would be caused.

			@retval STFRES_FALSE Bitmap is not busy and can be safely used for modifications without visual artifacts being caused
			@retval STFRES_TRUE Bitmap is busy and cannot be used for modifications without visual artifacts being caused.
		*/
		virtual STFResult IsBusy(IVDRGfxBitmap* bmp) = 0;


		/*!
			Moves cursor to position specified by x and y. Change takes effect on next VBLANK.
			@param x: Top left corner cursor bitmap's horizontal position in pixels, relative to left screen edge
			@param y: Top left corner cursor bitmap's vertical postion in lines, relative to upper screen edge
		*/
		virtual STFResult MoveCursorTo(int32 x, int32 y) = 0;
	};


///////////////////////////////////////////////////////////////////////////////
// IVDRVideoMixer and IVDR2dEffect Interface
///////////////////////////////////////////////////////////////////////////////

//
//! IVDRVideoMixer and IVDR2dEffect Interface related types and definitions
//

//@{
//! 2D effect flags

#define VDR_MIX_INPUT_ON_AT_START			0x00000001
#define VDR_MIX_INPUT_OFF_AT_END				0x00000002
#define VDR_WITH_PREVIOUS_FRAME				0x00000004
//@}


/*!
	@brief Enumerator Eff2dWipeDirection:

	Note: Diagonal wipes are always parallel to frame diagonals.
*/
enum Eff2dWipeDirection
	{
	VDR_WIPE_VERTICAL = 0, //!< Vertical wipe
	VDR_WIPE_HORIZONTAL, //!< Horizontal wipe
	VDR_WIPE_DIAGONAL_TOP_LEFT, //!< Diagonal wipe going from top-left corner to the bottom-right corner
	VDR_WIPE_DIAGONAL_TOP_RIGHT, //!< Diagonal wipe going from top-right corner to the bottom-left corner
	VDR_WIPE_DIAGONAL_BOTTOM_RIGHT, //!< Diagonal wipe going from bottom-right corner to the top-left corner
	VDR_WIPE_DIAGONAL_BOTTOM_LEFT //!< Diagonal wipe going from bottom-left corner to the top-right corner
	};


/*!
	@brief Structure Eff2dInterval, describes effect start and end time
*/
struct Eff2dInterval
	{
	STFHiPrec64BitTime		startTime; /*!< @brief Effect start time.
														 Value of 0 means "start immediately" */

	STFHiPrec64BitDuration	duration; /*!< @brief Effect duration
														Value of 0 means effect goes to final state immediately upon start -
														there is no visible transition.*/
	};


/*!
	@brief Structure Eff2dWipeParam, describes wipe 2D effect

	Depending on Eff2dWipeParam::wipeDirection, Eff2dWipeParam::startPosition and
	Eff2dWipeParam::endPosition have following meaning:

		- wipeDirection WIPE_VERTICAL: start and end y-coordinate for wipe edge.
		Valid values are -1 to frame height. Values -1 and frame height mean wipe
		is out of screen - pixels of the frame wipe is applied to are not visible.

		- wipeDirection is WIPE_HORIZONTAL: start and end x-coordinate for wipe edge.
		Valid values are -1 to frame width.

		- wipeDirection is WIPE_DIAGONAL_TOP_LEFT: startPosition is x-coordinate of
		wipe position on the upper frame border, endPosition is x-coordinate of
		wipe position on the lower frame border. Valid values are -1 to frame width.

		- wipeDirection is WIPE_DIAGONAL_TOP_RIGHT: startPosition is x-coordinate of
		wipe position on the upper frame border, endPosition is x-coordinate of
		wipe position on the lower frame border. Valid values are -1 to frame width.

		- wipeDirection is WIPE_DIAGONAL_BOTTOM_RIGHT: startPosition is x-coordinate of
		wipe position on the lower frame border, endPosition is x-coordinate of
		wipe position on the upper frame border. Valid values are -1 to frame width.

		- wipeDirection is WIPE_DIAGONAL_BOTTOM_LEFT: startPosition is x-coordinate of
		wipe position on the lower frame border, endPosition is x-coordinate of
		wipe position on the upper frame border. Valid values are -1 to frame width.
*/

struct Eff2dWipeParam
	{
	Eff2dInterval time; //!< Wipe start and end time, see Eff2dInterval
	Eff2dWipeDirection wipeDirection; //!< Wipe direction, see Eff2dWipeDirection
	int32 startPosition; //!< Wipe start postion, depends on wipeDirection value - see struct. details
	int32 endPosition; //!< Wipe start postion, depends on wipeDirection value - see struct. details
	};


/*!
	@brief Structure Eff2dBlendParam, describes blend effect
*/
struct Eff2dBlendParam
	{
	Eff2dInterval time; //!< Wipe start and end time, see Eff2dInterval
	uint32 startAlpha; //!< alpha value at effect start
	uint32 endAlpha; //!< alpha value at effect end
	};


/*!
	@brief Structure Eff2dZoomParam, describes zoom & move effect
*/
struct Eff2dZoomParam
	{
	Eff2dInterval time; //!< wipe start and end time, see comment on Eff2dInterval above
	VDRGfxRect startSrcRect; //!< start source rectangle
	VDRGfxRect endSrcRect; //!< end source rectangle
	VDRGfxRect startDestRect; //!< start destination rectangle
	VDRGfxRect endDestRect; //!< end destination rectangle
	int32 pathCurveWeight; /*!< @brief describes how much is the path along which rectangle moves curved.

								0 menans linear path, 255 means "corner-shaped" path */
	};


/*!
	@brief Enumerator VMixOutput, describes set of available outputs

	Output VDR_VMIX_OUT_MAIN is the one that contains mix of all the input signals.
	Output VDR_VMIX_OUT_AUX contains mix of only streaming signals.
*/
enum VMixOutput
	{
	VDR_VMIX_OUT_MAIN = 0, //!< normally contains mix of all the input signals
	VDR_VDR_VMIX_OUT_AUX //!< normally contains mix of inputs for dedicated for external recording (e.g. VHS)
	};


/*!
	@brief Enumerator VMixInputType

	VDR_VMIX_IN_STREAMING mixer inputs accept connection from streaming units, VDR_VMIX_IN_NON_STREAMING from non-streaming.
*/
enum VMixInputType
	{
	VDR_VMIX_IN_NON_STREAMING = 0, //!< Input is of non-streaming type
	VDR_VMIX_IN_STREAMING //!< Input is of streaming type
	};


/*!
	@brief Enumerator VMixInput
*/
enum VMixInput
	{
	VDR_VMIX_IN_VIDEO1 = 0, //!< Video 1 input
	VDR_VMIX_IN_VIDEO2, //!< Video 2 input
	VDR_VMIX_IN_QSIF, //!< QSIF input
	VDR_VMIX_IN_SUBPIC, //!< Subpicture input
	VDR_VMIX_IN_RTT, //!< Real Time Text input
	VDR_VMIX_IN_CURSOR, //!< Cursor input
	VDR_VMIX_IN_GUI, //!< GUI input
	VDR_VMIX_IN_BACKGROUND //!< Background input
	};



/// Public Unit ID of Video Mixer Unit
static const VDRUID VDRUID_VIDEO_MIXER	= 0x00000004;


//
//! Video Mixer Unit Interface definition
//

class IVDRVideoMixer : public IVDRBase
	{
	public:

		/*!
			When colorFormats is NULL, it does nothing and returns number of output formats available. This value should be used
			to allocate enough memory to hold all the color format entries.
			When colorFormats is not NULL, fills out memory pointed to by colorFormats with an array of VDRGfxColorFormat values
			and also returns number of formats available. Application is responsible for allocating enough memory to hold
			all the entries.
			@param colorFormats
		*/
		virtual int32 GetOutputColorFormats(VDRGfxColorFormat* colorFormats) = 0;


		/*!
			Sets parameters of output mixOut.
			@param outColorFormat: output color format
			@param outScanLayout: Defines structure of output buffers. If set to SF_FIELD, lines of one field are stored continuously;
						if set to SF_FRAME, top and bottom field lines are stored in consecutive order - merged into a frame, that is.
		*/
		virtual STFResult SetOutput(VMixOutput mixOut, VDRGfxColorFormat outColorFormat, VDRGfxScanLayout outScanLayout) = 0;

		/*!
			Enables/disables output:
				@param mixOut: output to enable/disable
				@param enable: when TRUE, output will be enabled, when FALSE, output will be disabled
		*/
		virtual STFResult EnableOutput(VMixOutput mixOut, bool enable) = 0;


		/*!
			Returns width and height of the output.
				@param width: pointer to an integer to store output width to
				@param height: pointer to an integer to store output height to

			Output size depends on currently set output video standard (PAL or NTSC), display type and content currently
			being played (SDTV, HDTV). Viewport rectangle for placement of input destination rectangles is then defined as:

				- left: 0
				- top: 0
				- right: width
				- bottom: height
		*/
		virtual STFResult GetOutputSize(int* width, int* height) = 0;


		/*!
			Sets parameters of input mixIn

				@param inType: VMIX_IN_STREAMING inputs appear on both VDR_VMIX_OUT_MAIN and VDR_VMIX_OUT_AUX output;
							VMIX_IN_NON_STREAMING inputs appear only on VDR_VMIX_OUT_AUX output.
				@param alpha: defines opacity of the input (0 - fully transparent, 255 - fully opaque). This value is combined
							with alpha value possibly already associated with input frame data (e.g. decoded subpicture pixels
							can be transparent) to form final alpha applied to the current frame data being processed.
				@param zOrder: the input with highest Z-order value is the top-most one.
				@param srcRect: pointer to rectangle defining area to be displayed inside input frame, relative to rectangle
							obtained by GetInputSize() call. When NULL, default value for input source is used, which is whole
							input frame area.
				@param destRect- pointer to rectangle containing destination area inside output frame, relative to viewport rectangle
							defined by values returned by GetOutputSize(). When NULL, default value is used, which is the one decided by
							the data source (e.g. decoders).
		*/
		virtual STFResult SetInput(VMixInput mixIn, VMixInputType inType, uint8 alpha, uint8 zOrder, VDRGfxRect* srcRect, VDRGfxRect* destRect) = 0;


		/*!
			Returns current source and destination rectangles.
				@param mixIn: input whose rectangles are queried
				@param width: pointer to integer to store input width to
				@param height: pointer to integer to store input height to
		*/
		virtual STFResult GetInputSize(VMixInput mixIn, int* width, int* height) = 0;

		/*!
			Enables/disables input.
				@param mixIn: input to enable/disable
				@param enable: TRUE enables input, FALSE disables
		*/
		virtual STFResult EnableInput(VMixInput mixIn, bool enable) = 0;


		/*!
			Sets background color
			@param backgroundColor
		*/
		virtual STFResult SetBackgroundColor(VDRGfxColorRGB backgroundColor) = 0;


		/*!
			Apply wipe effect on Video Mixer input mixIn.

			In general, one effect involves two "sources": foreground and background. As names suggest, foreground source
			is in front of background source. Effect actually modifies only foreground source, typically showing more
			and more of the background source as the effect progresses (but it can also show less of the background
			source if effect is set up so).

			"Sources" can be either two mixer inputs, or successive frames of the same mixer input. This is controlled
			by 'flags' parameter, see WipeMixerInput().

				@param mixIn: ID of input to apply wipe to
				@param wipe: describes wipe effect, see Eff2dWipeParam
				@param flags: effect flags
					@li MIX_INPUT_ON_AT_START: at start of the effect, all the mixer inputs involved in the effect are
							automatically enabled
					@li MIX_INPUT_OFF_AT_END: at the end of the effect, source input is automatically disabled
					@li WITH_LAST_FRAME: do effect using frame that was delivered prior to the frame effect
							starts at as foreground source, and frames that are comming after effect start as background
							source. If this flag is not set, background source is input that is in Z-order below mixer
							input on which is effect applied to.
		*/
		virtual STFResult WipeMixerInput(VMixInput mixIn, Eff2dWipeParam* wipe, uint32 flags) = 0;


		/*!
			Apply blend effect on Video Mixer input mixIn. See WipeMixerInput() for general effect comments.
				@param mixIn: ID of input to apply blending to
				@param blend: describes blend effect, see Eff2dZoomParam
				@param flags: effect flags, same as in WipeMixerInput()
		*/
		virtual STFResult BlendMixerInput(VMixInput mixIn, Eff2dZoomParam* blend, uint32 flags) = 0;


		/*!
			Apply zoom in/out and move effect to all the Video Mixer inputs specified in a list pointed to by
			mixInput.  See WipeMixerInput() for general effect comments.
				@param mixInputs: pointer to list of inputs zoom will be applied to
				@param numInputs: number of inputs in the list pointed to by mixInputs
				@param zoomMove: describes zoom & move effect, see Eff2dZoomParam. Both destination AND source
							rectangles are relative to OUTPUT viewport rectangle (see GetOutputSize()), in order to simplify
							combining multiple sources in a single effect.
				@param flags: effect flags, same as in WipeMixerInput()
		*/
		virtual STFResult ZoomMoveMixerInputs(VMixInput* mixInputs, int32 numInputs, Eff2dZoomParam* zoomMove, uint32 flags) = 0;
	};


//
//! 2D Effect Unit Interface definition
//

class IVDR2dEffect : public IVDRBase
	{
	public:

		/*!
			Apply wipe effect.
			Effect is applied as successive blits from srcBmp to destBmp, using specified srcRect and destRect.
				@param srcBmp: source bitmap
				@param destBmp: destination bitmap
				@param srcRect: source rectangle
				@param destBmp: destination rectangle
				@param wipe: describes wipe effect, see Eff2dWipeParam
		*/
		virtual STFResult Wipe(IVDRGfxBitmap* srcBmp, IVDRGfxBitmap* destBmp, VDRGfxRect* srcRect, IVDRGfxBitmap* destRect, Eff2dWipeParam* wipe) = 0;

		/*!
			Apply blend effect:
			Effect is applied as successive blits from srcBmp to destBmp, using specified srcRect and destRect.
				@param srcBmp: source bitmap
				@param destBmp: destination bitmap
				@param srcRect: source rectangle
				@param destBmp: destination rectangle
				@param blend: describes blend effect, see Eff2dZoomParam
		*/
		virtual STFResult Blend(IVDRGfxBitmap* srcBmp, IVDRGfxBitmap* destBmp, VDRGfxRect* srcRect, IVDRGfxBitmap* destRect, Eff2dZoomParam* blend) = 0;

		/*!
			Apply zoom in/out and move effect:
			Effect is applied as successive blits from srcBmp to destBmp, using specified srcRect and destRect.
				@param srcBmp: source bitmap
				@param destBmp: destination bitmap
				@param destClipRect: changes in destination bitmap are limited to this rectangle.
				@param zoomMove: describes zoom & move effect, see Eff2dZoomParam.
		*/
		virtual STFResult ZoomMove(IVDRGfxBitmap* srcBmp, IVDRGfxBitmap* destBmp, VDRGfxRect* destClipRect, Eff2dZoomParam* zoomMove) = 0;
	};

#endif // #ifndef VIDEODISPLAY_H

