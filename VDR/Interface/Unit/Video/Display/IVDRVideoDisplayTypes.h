///
///	@brief	VDR Video Display public types and definitions
///

#ifndef VDRVIDEODISPLAYTYPES_H
#define VDRVIDEODISPLAYTYPES_H

#include "STF/Interface/Types/STFBasicTypes.h"
#include "VDR/Interface/Memory/IVDRMemoryPoolAllocator.h"

//
//! Video Display related public types and definitions (used in Video Display public interfaces and VDR driver implementation)
//

//! VDRGfxColor type
typedef uint32			VDRGfxColor;

//! VDRGfxColorRGB type
typedef uint32			VDRGfxColorRGB;

//! VDRGfxColorYCbCr type
typedef uint32			VDRGfxColorYCbCr;

#define VDR_RGB2LONG(y, Cb, Cr) ((uint32)( (((uint8)(r))<<16) | (((uint8)(g))<<8) | ((uint8)(b))) )
#define VDR_ARGB2LONG(a, r, g, b) ((uint32)( (((uint8)(a))<<24) | (((uint8)(r))<<16) | (((uint8)g)<<8) | (uint8)(b)))
#define VDR_GET_A(gfxCol) (((gfxCol)>>24)&0xff)
#define VDR_GET_R(gfxCol) (((gfxCol)>>16)&0xff)
#define VDR_GET_G(gfxCol) (((gfxCol)>>8)&0xff)
#define VDR_GET_B(gfxCol) (((gfxCol))&0xff)

#define VDR_YCbCr2LONG(y, Cb, Cr) ((uint32)( (((uint8)(Cr))<<16) | (((uint8)(y))<<8) | ((uint8)(Cb))) )
#define VDR_AYCbCr2LONG(a, y, Cb, Cr) ((uint32)( (((uint8)(a))<<24) | (((uint8)(Cr))<<16) | (((uint8)(y))<<8) | ((uint8)(Cb))) )
#define VDR_GET_Y(gfxCol) (((gfxCol)>>8) & 0xff)
#define VDR_GET_Cb(gfxCol) ((gfxCol) & 0xff)
#define VDR_GET_Cr(gfxCol) (((gfxCol)>>16) & 0xff)

/*!
	@brief Structure VDRGfxPoint, defines position in 2D coordinate system ("a dot").
*/

struct VDRGfxPoint
	{
	int32 x; //!< x coordinate
	int32 y; //!< y coordinate
	};


/*!
	@brief Structure VDRGfxRect, defines rectangle area.

	Fields 'left', 'top', 'right' and 'bottom' are not included in rectangle
	area (rectangle width is right - left, and height is bottom - top).
*/

struct VDRGfxRect
	{
	int32 left; //!< left edge
	int32 top; //!< top edge
	int32 right; //!< right edge
	int32 bottom; //!< bottom edge

	int32 Width(void) {return right - left;} //!< get rectangle width
	int32 Height(void) {return bottom - top;} //!< get rectangle height

	//! Upscale rectangle by a given factor
	void UpscaleRectangle(int32 scaleFactor)
		{
		left = left*scaleFactor;
		top = top*scaleFactor;
		right = right*scaleFactor;
		bottom = bottom*scaleFactor;
		}

	//! Downscale rectangle by a given factor
	void DownscaleRectangle(int32 scaleFactor)
		{
		left = left/scaleFactor;
		top = top/scaleFactor;
		right = right/scaleFactor;
		bottom = bottom/scaleFactor;
		}
	};


/*!
	@brief Enum VDRGfxColorFormat
*/

enum VDRGfxColorFormat
	{
	// RGB types
	VDR_RGB565 = 0, //!< 16 bpp
	VDR_RGB888, //!< 24 bpp
	VDR_ARGB8565, //!< 24 bpp (8 alpha, 16 color)
	VDR_ARGB8888, //!< 32 bpp (8 alpha, 24 color)
	VDR_ARGB1555, //!< 16 bpp (1 alpha, 16 color)
	VDR_ARGB4444, //!< 16 bpp (4 apha, 12 color)

	// CLUT (Color LookUp Table) types (RGB palette)   
	VDR_RGB_CLUT1, //!< 1 bpp
	VDR_RGB_CLUT2, //!< 2 bpp
	VDR_RGB_CLUT4, //!< 4 bpp
	VDR_RGB_CLUT8, //!< 8 bpp
	VDR_RGB_ACLUT44, //!< 8 bpp (4 alpha, 4 CLUT index)
	VDR_RGB_ACLUT88, //!< 16 bpp (8 alpha, 8 CLUT index)

	// CLUT (Color LookUp Table) types (YCbCr palette)   
	VDR_YCbCr_CLUT1, //!< 1 bpp
	VDR_YCbCr_CLUT2, //!< 2 bpp
	VDR_YCbCr_CLUT4, //!< 4 bpp
	VDR_YCbCr_CLUT8, //!< 8 bpp
	VDR_YCbCr_ACLUT44, //!< 8 bpp (4 alpha, 4 CLUT index)
	VDR_YCbCr_ACLUT88, //!< 16 bpp (8 alpha, 8 CLUT index)

	// YCbCr types
	VDR_YCbCr888, //!< 24 bpp: CrYCb
	VDR_YCbCr420, //!< 12 bpp: multi-plane format - Y, Cb and Cr are stored in separate planes, Cb and Cr subsampled horiz. and vert. by factor of 2
	VDR_YCbCr422R, //!< 16 bpp: Y2CrY1Cb (2 pixels) - raster format
	VDR_AYCbCr8888, //!< 32 bpp (8 alpha, 24 color): ACrYCb
	VDR_YCbCr420MB2, //!< 12 bpp, regular 4:2:0 macroblock format with 2 planes: 1 for Y and 1 for Cb and Cr
	VDR_YCbCr420STMB, //!< 12 bpp: ST proprietory 4:2:0 macroblock format
	VDR_YCbCr422STMB //!< 16 bpp: ST proprietory 422 macroblock format
	};


/*!
	@brief Enum VDRGfxScanLayout, values for bitmap scan format
*/

enum VDRGfxScanLayout
	{
	VDR_SL_FIELDS = 0,    //!< Stored as two fields one behind another (first all the lines of top field, then all the lines of bottom field)
	VDR_SL_FRAME,        //!< Stored as frame. If interlaced: top and bottom field lines are stored in consecutive order - weaved to a frame
	VDR_SL_SINGLE_FIELD	//!< Stored as single field
	};


/*!
	@brief Enum VDRGfxBitmapFlags, values for bitmap flags
*/

enum VDRGfxBitmapFlags
	{
	VDR_BF_ALPHA_128_LEVELS = 0x00000001 //!< Valid only for 8-bit alpha color formats and CLUT formats. If set, alpha value ranges from 0 to 127. If not set, alpha ranges from 0 to 255.
	};


/*!
	@brief Structure VDRGfxBitmapDesc, contains bitmap description
*/

struct VDRGfxBitmapDesc
	{
	uint16 width; //!< Bitmap width in pixels
	uint16 height; //!< Bitmap height in lines
	uint16 pitch;	  //!< Number of bytes from current pixel to the same pixel one line below. For multiplanar formats, this is pitch of first plane.
	uint16 pitch23;  //!< Number of bytes from current pixel to the same pixel one line below, for second and third plane. Valid only in multiplanar formats.
	VDRGfxColorFormat colorFormat; //<! Bitmap color format
	uint32 flags;	//!< Different bitmap flags, @see VDRGfxBitmapFlags. Default value is 0.
	
	//! Get number of bits per pixel related to color format
	STFResult GetBitsPerPixel(uint32* bitsPerPixel)
		{
		switch (colorFormat)
			{
			case VDR_AYCbCr8888: //!< 32 bpp (8 alpha, 24 color): ACrYCb
			case VDR_ARGB8888: //!< 32 bpp (8 alpha, 24 color)
				 *bitsPerPixel = 32; break;

			case VDR_ARGB8565: //!< 24 bpp (8 alpha, 16 color)
			case VDR_YCbCr888: //!< 24 bpp: CrYCb
			case VDR_RGB888: //!< 24 bpp
				 *bitsPerPixel = 24; break;

			case VDR_RGB565: //!< 16 bpp
			case VDR_ARGB1555: //!< 16 bpp (1 alpha, 16 color)
			case VDR_ARGB4444: //!< 16 bpp (4 apha, 12 color)
			case VDR_RGB_ACLUT88:
			case VDR_YCbCr_ACLUT88: //!< 16 bpp (8 alpha, 8 CLUT index)
			case VDR_YCbCr422R: //!< 16 bpp: Y2CrY1Cb (2 pixels) - raster format
			case VDR_YCbCr422STMB: //!< 16 bpp: ST proprietory 422 macroblock format
				 *bitsPerPixel = 16; break;

			// CLUT (Color LookUp Table) types    
			case VDR_RGB_CLUT8: //!< 8 bpp
			case VDR_YCbCr_CLUT8: //!< 8 bpp
			case VDR_RGB_ACLUT44: //!< 8 bpp (4 alpha, 4 CLUT index)
			case VDR_YCbCr_ACLUT44: //!< 8 bpp (4 alpha, 4 CLUT index)
				 *bitsPerPixel = 8; break;

			case VDR_RGB_CLUT1: //!< 1 bpp
			case VDR_YCbCr_CLUT1: //!< 1 bpp
				 *bitsPerPixel = 1; break;

			case VDR_RGB_CLUT2: //!< 2 bpp
			case VDR_YCbCr_CLUT2: //!< 2 bpp
				 *bitsPerPixel = 2; break;

			case VDR_RGB_CLUT4: //!< 4 bpp
			case VDR_YCbCr_CLUT4: //!< 4 bpp
				 *bitsPerPixel = 4; break;

			// YCbCr types
			case VDR_YCbCr420MB2: //!< 12 bpp, regular 4:2:0 macroblock format with 2 planes: 1 for Y and 1 for Cb and Cr
			case VDR_YCbCr420: //!< 12 bpp: multi-plane format - Y, Cb and Cr are stored in separate planes, Cb and Cr subsampled horiz. and vert. by factor of 2
			case VDR_YCbCr420STMB: //!< 12 bpp: ST proprietory 4:2:0 macroblock format
				 *bitsPerPixel = 12; break;

			default:
				STFRES_RAISE(STFRES_OBJECT_INVALID); // unsupported formats
			}
		
		STFRES_RAISE_OK;
		}
		
	//! @brief Get number of separate bitmap buffers required for storing bitmap pixels.
	//!
	//! In case of non-CLUT formats that's actually the number of bitmap planes. In
	//! case of CLUT format, this is number of required buffers includding buffer for
	//! CLUT (Color Lookup Table) itself, so it's 2.
	STFResult GetNumberOfPixelBuffers(uint32* numBmpPixelBuffers)
		{
		switch (colorFormat)
			{
			// Raster formats
			case VDR_AYCbCr8888: //!< 32 bpp (8 alpha, 24 color): ACrYCb
			case VDR_ARGB8888: //!< 32 bpp (8 alpha, 24 color)
			case VDR_ARGB8565: //!< 24 bpp (8 alpha, 16 color)
			case VDR_YCbCr888: //!< 24 bpp: CrYCb
			case VDR_RGB888: //!< 24 bpp
			case VDR_RGB565: //!< 16 bpp
			case VDR_ARGB1555: //!< 16 bpp (1 alpha, 16 color)
			case VDR_ARGB4444: //!< 16 bpp (4 apha, 12 color)
			case VDR_YCbCr422R: //!< 16 bpp: Y2CrY1Cb (2 pixels) - raster format
				 *numBmpPixelBuffers = 1; break;

			// CLUT (Color LookUp Table) formats    
			case VDR_RGB_CLUT1: //!< 1 bpp
			case VDR_YCbCr_CLUT1: //!< 1 bpp
			case VDR_RGB_CLUT2: //!< 2 bpp
			case VDR_YCbCr_CLUT2: //!< 2 bpp
			case VDR_RGB_CLUT4: //!< 4 bpp
			case VDR_YCbCr_CLUT4: //!< 4 bpp
			case VDR_RGB_CLUT8: //!< 8 bpp
			case VDR_YCbCr_CLUT8: //!< 8 bpp
			case VDR_RGB_ACLUT44: //!< 8 bpp (4 alpha, 4 CLUT index)
			case VDR_YCbCr_ACLUT44: //!< 8 bpp (4 alpha, 4 CLUT index)
			case VDR_RGB_ACLUT88: //!< 16 bpp (8 alpha, 8 CLUT index)
			case VDR_YCbCr_ACLUT88: //!< 16 bpp (8 alpha, 8 CLUT index)

			// YCbCr multiplanar formats
			case VDR_YCbCr420MB2: //!< 12 bpp, regular 4:2:0 macroblock format with 2 planes: 1 for Y and 1 for Cb and Cr
			case VDR_YCbCr422STMB: //!< 16 bpp: ST proprietory 422 macroblock format
			case VDR_YCbCr420STMB: //!< 12 bpp: ST proprietory 4:2:0 macroblock format
				*numBmpPixelBuffers = 2; break;

			case VDR_YCbCr420: //!< 12 bpp: multi-plane format - Y, Cb and Cr are stored in separate planes, Cb and Cr subsampled horiz. and vert. by factor of 2
				*numBmpPixelBuffers = 3; break;

			default:
				STFRES_RAISE(STFRES_OBJECT_INVALID); // unsupported formats
			}
		
		STFRES_RAISE_OK;
		}

	bool IsClutType()
		{
		switch (colorFormat)
			{
			case VDR_YCbCr420STMB:
			case VDR_YCbCr422STMB:
			// RGB types       
			case VDR_RGB565:
			case VDR_RGB888:
  			case VDR_ARGB8565:
			case VDR_ARGB8888:
			case VDR_ARGB1555:
			case VDR_ARGB4444:
			// YCbCr types
			case VDR_YCbCr888:
			case VDR_YCbCr420:
			case VDR_YCbCr422R:
			case VDR_AYCbCr8888:
			default:
				return false;

			// CLUT (Color LookUp Table) types    
			case VDR_RGB_CLUT1:
			case VDR_RGB_CLUT2:
			case VDR_RGB_CLUT4:
			case VDR_RGB_CLUT8:
			case VDR_RGB_ACLUT44:
			case VDR_RGB_ACLUT88:
			// CLUT (Color LookUp Table) types    
			case VDR_YCbCr_CLUT1:
			case VDR_YCbCr_CLUT2:
			case VDR_YCbCr_CLUT4:
			case VDR_YCbCr_CLUT8:
			case VDR_YCbCr_ACLUT44:
			case VDR_YCbCr_ACLUT88:
				return true;

			}
		}
	};


/*!
	@brief Structure VDRGfxBitmapPixels, contains "pointers" to memory where bitmap pixels are stored.

  Pixel memory is accessed via VDRDataRange structures. VDRDataRange is essentially	a "smart pointer" that offers some more
  flexibility in accessing memory data, and provides user/kernel mode memory re-mapping.
*/

struct VDRGfxBitmapPixels
	{
	VDRDataRange pixels; //!< "Pointer" to pixel data, or, for multiplanar formats, pointer to the pixel data of first plane.
	VDRDataRange pixels2; //!< "Pointer" to the pixel data of second plane. Valid only in multiplanar formats.
	VDRDataRange pixels3; //!< "Pointer" to the pixel data of third plane. Valid only in multiplanar formats.
	VDRDataRange clut; //!< "Pointer" to Color Lookup Table (also known as palette). Valid only in CLUT (paletized) color formats.
	};


/*!
	@brief Structure VDRGfxFont, describes font parameters
*/

struct VDRGfxFont
	{
	uint16 numSegments; //!< Number of font segments
	uint16 height; //!< Font height
	// uint32 segTable[n];	
	};



#endif // #ifndef VDRVIDEODISPLAYTYPES_H

