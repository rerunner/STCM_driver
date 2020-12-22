///
///	@brief	Video Display private data types, definitions and simple helper classes
///

#ifndef VIDEODISPLAYTYPES_H
#define VIDEODISPLAYTYPES_H


#include "VDR/Interface/Unit/Video/Display/IVDRVideoDisplayTypes.h"
#include "VDR/Interface/Unit/Video/Display/IVDRVideoTypes.h"
#include "Device/Interface/Unit/Video/IMPEGVideoTypes.h"


/*!
	\brief Enumerator VDispVideoScanMode
*/
enum VDispVideoScanMode
	{
	VSMODE_INTERLACED = 0, //!< Two fields in bitmap are from two different points in time
	VSMODE_PROGRESSIVE //!< Both fields in bitmap belong to the same point in time
	};

///{

//! Values for VDispBitmap.flags field

#define VDISPBMP_BITMAP_VALID			0x00000001	// Bitmap structure valid
#define VDISPBMP_DISABLE_DISPLAY		0x00000002
#define VDISPBMP_SHOW_AS_LETTERBOX		0x00000004
#define VDISPBMP_SHOW_ON_MAIN_OUT		0x00000008
#define VDISPBMP_SHOW_ON_AUX_OUT		0x00000010
#define VDISPBMP_TOP_FIELD_FIRST		0x00000020	// If set, the top field is ealier in time than the bottom field
#define VDISPBMP_REPEAT_FIELD			0x00000040	// If set, the frame is shown for three field times, the first hown field is repeated
#define VDISPBMP_COLOR_FILL				0x00000080
#define VDISPBMP_OBEY_FIELD_POLARITY	0x00000400	// independent from the frame being marked progressive we use this flag to obey VDISPBMP_TOP_FIELD_FIRST
#define VDISPBMP_RENDER_DURATION_VALID	0x00000800	///< Debug facility: If this bit is set the renderDuration field is valid
#define VDISPBMP_VBI_BITMAP				0x00001000	// VBI bitmap can be treated special at mixer so it does not go through blitter


///}

/*!
	\brief Constants related to VDispBitmapDesc::pixelAspectRatio

	Pixel aspect ratio is defined as pixelWidth/pixelHeight.

	Such obtained aspect ratio is farther multiplied by 675 - that means, value
	675 is aspect ratio 1. This value is chosed because it avoids fractional
	errors and 32-bit overflows for all possible aspect ratio calculations.

*/
///{
#define VD_PIX_ASPECT_RATIO_1_BY_1						675	///< Pixel aspect ratio 1:1		= 1.000 - square pixel
#define VD_PIX_ASPECT_RATIO_4_BY_3						900	///< Pixel aspect	ratio 4:3		= 1.333
#define VD_PIX_ASPECT_RATIO_NTSC_4_BY_3				600	///< Pixel aspect ratio 24:27		= 0.889 (pixel in 4:3 NTSC frame)
#define VD_PIX_ASPECT_RATIO_PAL_4_BY_3					720	///< Pixel aspect ratio 144:135	= 1.067 (pixel in 4:3 PAL frame)
#define VD_PIX_ASPECT_RATIO_NTSC_16_BY_9				800	///< Pixel aspect ratio 32:27		= 1.185 (pixel in 16:9 NTSC frame)
#define VD_PIX_ASPECT_RATIO_PAL_16_BY_9				960	///< Pixel aspect ratio 192:135	= 1.422 (pixel in 16:9 PAL frame)
///}

/*!
	\brief Structure VDispBitmapDesc

  Holds all the bitmap description information nedded for displaying the bitmap.
*/

struct VDispBitmapDesc
	{
	VDRGfxBitmapDesc gfxDesc;          //!< Basic bitmap description

//@{

//! Additional bitmap paramters used within the driver

	uint8                   alpha;                  //!< Alpha (transparency) value associated with this bitmap. 0 - fully transparent, 255 - fully opaque.
	uint32                  flags;                  //!< Different flags
	int16                   letterboxStripeHeight;  //!< Height of the letterbox stripe if bitmap should be shown in this mode, that is if VDISPBMP_SHOW_AS_LETTERBOX flag is set.
	uint32                  pixelAspectRatio;       //!< Pixel aspect ratio (pixel width / height * 675) - exact range of values still to be defined
	VDRGfxColorRGB          fillColor;              //!< Color value to fill bitmap destination area with when flag VDISPBMP_COLOR_FILL is set
	VDRGfxColorRGB          srcClrKeyLower;         //!< Lower limit for source color keying
	VDRGfxColorRGB          srcClrKeyUpper;         //!< Upper limit for source color keying
	VDRGfxColorRGB          destClrKeyLower;        //!< Lower limit for destination color keying
	VDRGfxColorRGB          destClrKeyUpper;        //!< Upper limit for destination color keying
	VDRGfxScanLayout        bitmapScanLayout;       //!< Bitmap layout: fields stored separately, or interleaved into a frame
	VDispVideoScanMode      bitmapScanMode;         //!< Bitmap scan mode: interlaced or progressive
	VideoBitmapContent      bitmapContent;          //!< Bitmap field content: both fields/frame, top or bottom
	VDRGfxRect              srcRect;                //!< Position and size of source rectangle
	VDRGfxRect              destRect;               //!< Position and size of rectangle on destination
	int16                   zOrder;                 //!< Position of the bitmap in z-order
	STFHiPrec32BitDuration  fieldDuration;          //!< Used in case of interlaced frame to calculate the start time of the second field
	bool                    bitmapBusy;             //!< If this flag is set, the bitmap is currently being rendered and not yet ready to be displayed or processed further	
	STFHiPrec32BitDuration  renderDuration;         //!< Debug facility: If the corresponding bit is set in the flags, the time it took to render this bitmap is stored here				   
	uint32                  uniqueID;               //!< This ID is assidned and used internally by video mixer and blitter. Don't use this field otherwise, it may be overwritten
	PictureDisplayExtension panScanOffsets;         //!< Contains optional pan scan offsets associated with this frame (depending on 'validNumber')
	ClosedCaptioning        closedCaptioning;       //!< Contains optional closed captioning data associated with this frame (depending on 'validBytes')
//@}
		
	};



/*!
	\brief Structure VDispBitmap

  Holds all the information nedded for displaying bitmap, including bitmap pixels.
*/

struct VDispBitmap
	{
	VDRGfxBitmapPixels gfxPix; //!< Contains "pointers" to pixel data
	VDispBitmapDesc desc;      //!< Bitmap description
	};


//Temporary class - will be removed and replaced by appropriate STF class
class Uint32RegisterAccess
	{
	public:
		Uint32RegisterAccess(void* regBaseAddr)
			{
			this->regBaseAddr = (uint8*)regBaseAddr;
			}

		void WriteReg(uint32 regOffs, uint32 value)
			{
#ifndef _WIN32
			*((uint32*)(regBaseAddr+regOffs)) = value;
#endif
			}

		uint32 ReadReg(uint32 regOffs)
			{
#ifndef _WIN32
			return *((uint32*)(regBaseAddr+regOffs));
#else
			return 0;
#endif
			}

	protected:
		uint8* regBaseAddr;
	};

#endif
