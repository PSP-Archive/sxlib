/*
 * sxlib.h
 * SXLibrary Version 0.1.5 by XArt Software & Designs http://www.xart.co.uk
 *
 * This work is not under any license, you are free to use and modify any part of this work you see fit.
 * No LICENSE for a free working together world.
 *
 */

#ifndef SXLIB_H
#define SXLIB_H

#include <stdint.h>

typedef uint32_t SXColor;


typedef struct {	SXColor *		data;
					unsigned int	pixelFormat;				// uses GU_PSM_* constants in pspgu.h
					unsigned int	textureWidth;				// the real width of data, 2^n with n>=0
					unsigned int	textureHeight;				// the real height of data, 2^n with n>=0
					unsigned int	imageWidth;					// the image width
					unsigned int	imageHeight;	
					float			scaleWidth;
					float			scaleHeight;
					short			depth;						// 24 or 32
					float			opacity;
					int				x;							// image x positions
					int				y;							// image y positions
					unsigned int	angle;						// angle (rotation) in degrees
} SXImgImage;

typedef struct	{	uint32_t		held;						// Controls currently down (held)
					uint32_t		pressed;					// Controls pressed (only reported once when the user pressed it)
					uint32_t		released;					// Controls released (only reported once when the user releases it)
					uint32_t		lastHeld;					// Allows you to trick with the held member without messing up the auto-repeat feature
	
					short			autoRepeatInit;				// Time for the initialization of the autorepeat feature
					short			autoRepeatInterval;			// Interval before the autorepeat feature is switched on (the time the user must let the key down)
					int				autoRepeatMask;				// Controls affected by the autorepeat feature
					short			autoRepeatCounter;			// Counter (internal)

					signed char		analogX;					// Horizontal position of the analog stick (-128: left, +127: right)
					signed char		analogY;					// Vertical position of the analog stick (-128: top, +127: bottom)
	
					struct {
						float		x;
						float		y;
					} mouse;
} SXCtrlController;


#define SX_CTRL_SELECT		0x00000001
#define SX_CTRL_START		0x00000008
#define SX_CTRL_UP			0x00000010
#define SX_CTRL_RIGHT		0x00000020
#define SX_CTRL_DOWN		0x00000040
#define SX_CTRL_LEFT		0x00000080
#define SX_CTRL_L			0x00000100
#define SX_CTRL_R			0x00000200
#define SX_CTRL_TRIANGLE	0x00001000
#define SX_CTRL_CIRCLE		0x00002000
#define SX_CTRL_CROSS		0x00004000
#define SX_CTRL_SQUARE		0x00008000
#define SX_CTRL_HOME		0x00010000
#define SX_CTRL_HOLD		0x00020000
#define SX_CTRL_NOTE		0x00800000

#ifndef _PSP
#pragma mark IntraFont Extended Defines
#endif

#define INTRAFONT_LATIN(f)		ltn[f]

#define INTRAFONT_SERIF			0x01
#define INTRAFONT_ITALIC		0x02
#define INTRAFONT_BOLD			0x04
#define INTRAFONT_SMALL			0x08

#define INTRAFONT_JAPAN			jpn0
#define INTRAFONT_KOREAN		kr0
#define INTRAFONT_SYMBOLS		arib


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _PSP
#pragma mark DRM Digital Rights Managment
#endif
	
	/*
	 * set a encryption key for encrypting DRM data with.
	 *
	 * @param key			: encryption key to be used (24 bytes)
	 */
	void sxDrmSetEncryptionKey(const uint8_t *key);
	
	/*
	 * creates a valid DRM data file for application.
	 *
	 * @param appSig		: application signiture (32bit ID)
	 * @param signToDevice	: sign to device if signToDevice == 1 else leave unsigned to device.
	 * @param expireSeconds	: number of seconds before DRM file becomes invalid.
	 */
	void sxDrmCreateDRMDataforApplication(uint32_t appSig, int signToDevice);
	
	/*
	 * finds out if the application has been signed for the device.
	 *
	 * @param appSig	: application signiture (32bit ID)
	 * @return			: returns 1 if application is signed to the device else returns 0.
	 */
	int sxDrmIsApplicationSignedToDevice(uint32_t appSig);
	
	/*
	 * signs the application for the device.
	 *
	 * @param appSig	: application signiture (32bit ID)
	 * @return			: returns 1 if application succefuly signed to the device else returns 0.
	 */
	int sxDrmSignApplicationToDevice(uint32_t appSig);
	
	/*
	 * signs the application for any device.
	 *
	 * @param appSig	: application signiture (32bit ID)
	 * @return			: returns 1 if application succefuly signed to the device else returns 0.
	 */
	int sxDrmSignApplicationForAnyDevice(uint32_t appSig);
	
	
	static int running;
	
#ifndef _PSP
#pragma mark System
#endif
	
	void sxInit(void);
	void sxExitApplication(void);
	
	void sxStartDrawing(void);
	void sxEndDrawing(void);
	
#ifndef _PSP
#pragma mark Platform
#endif
	
	/*
	 For checking if little-endian format is used by host.
	 
	 Return Value
	 If the little-endian format is used then 1 will be returned else 0 will be returned for big-endian.
	 */
	int sxLittleEndian(void);
	
	/*
	 For checking if big-endian format is used by host.
	 
	 Return Value
	 If the big-endian format is used then 1 will be returned else 0 will be returned for little-endian.
	 */
	int sxBigEndian(void);
	
	/*
	 For checking what endian format is used by platform.
	 
	 Return Value
	 If the little-endian format is used then 1 will be returned else 0 will be returned for big-endian.
	 */
	int sxLittleEndian(void);
	
	/*
	 Converts a 16-bit integer from big-endian format to the host native byte order.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
	 */
	int16_t sxSwapInt16BigToHost(int16_t arg);
	
	/*
	 Converts a 16-bit integer from little-endian format to the host native byte order.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
	 */
	int16_t sxSwapInt16LittleToHost(int16_t arg);
	
	/*
	 Alias to swapInt16BigToHost
	 Converts a 16-bit integer from the host native byte order to big-endian format.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
	 */
	#define sxSwapInt16HostToBig sxSwapInt16BigToHost
	
	/*
	 Alias to swapInt16LittleToHost
	 Converts a 16-bit integer from the host native byte order to little-endian format.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
	 */
	#define sxSwapInt16HostToLittle sxSwapInt16LittleToHost
	
	
	/*
	 Converts a 32-bit integer from big-endian format to the host native byte order.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
	 */
	int32_t sxSwapInt32BigToHost(int32_t arg);
	
	/*
	 Converts a 32-bit integer from little-endian format to the host native byte order.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
	 */
	int32_t sxSwapInt32LittleToHost(int32_t arg);
	
	/*
	 Alias to swapInt32BigToHost
	 Converts a 32-bit integer from the host native byte order to big-endian format.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
	 */
	#define sxSwapInt32HostToBig sxSwapInt32BigToHost
	
	/*
	 Converts a 32-bit integer from the host native byte order to little-endian format.
	 
	 Parameters
	 arg
	 The integer whose bytes should be swapped.
	 
	 Return Value
	 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
	 */
	#define sxSwapInt32HostToLittle sxSwapInt32LittleToHost


#ifndef _PSP
#pragma mark Controller
#endif
	void sxCtrlEnableMouseEmulation(void);
	void sxCtrlDisableMouseEmulation(void);
	void sxCtrlReadControls(SXCtrlController *ctrl);
	#define sxCtrlFlushControls sxCtrlReadControls
	
#ifndef _PSP
#pragma mark Textures
#endif
	
	void sxTexSwizzle(SXImgImage *image);

#ifndef _PSP
#pragma mark Images
#endif
	
	/*
	 * Load a PNG image.
	 *
	 * @pre filename != NULL
	 * @param filename - filename of the PNG image to load
	 * @return pointer to a new allocated Image struct, or NULL on failure
	 */
	#define sxImgLoadImage(f) sxImgLoadImageFromOffset(f,0)
	
	/*
	 * Load a PNG image from file at the given offset in the file, e.g EBOOT.PBP has an embeded PNG at a given offset in the file.
	 *
	 * @pre filename != NULL
	 * @param filename - filename of the PNG image to load
	 * @return pointer to a new allocated Image struct, or NULL on failure
	 */
	SXImgImage* sxImgLoadImageFromOffset(const char *filename, uint32_t offset);
	
	
	/*
	 * Create an empty image.
	 *
	 * @pre width > 0 && height > 0 && width <= 512 && height <= 512
	 * @param width - width of the new image
	 * @param height - height of the new image
	 * @return pointer to a new allocated Image struct, all pixels initialized to color 0, or NULL on failure
	 */
	SXImgImage* sxImgCreateImage(int width, int height);
	
	/*
	 * Frees an allocated image.
	 *
	 * @pre image != null
	 * @param image a pointer to an image struct
	 */
	void sxImgFreeImage(SXImgImage* image);
	
	/*
	 * Initialize all pixels of an image with a color.
	 *
	 * @pre image != NULL
	 * @param color - new color for the pixels
	 * @param image - image to clear
	 */
	void sxImgClearImage(SXColor color, SXImgImage* image);
	
	/*
	 * Initialize all pixels of an image with pre-defined colors.
	 *
	 * @pre image != NULL
	 * @param data - new colors for the pixels
	 * @param image - image to set
	 */
	void sxImgSetImage(void *data, SXImgImage* image);
	
	
#ifndef _PSP
#pragma mark Graphics
#endif
	void sxRenderQuick2DImageOn3D(int left, int top, const SXImgImage* image);
	void sxRender2DImageOn3D(float left, float top, const SXImgImage* image);
	void sxRender2DImage(float left, float top, float zIndex, const SXImgImage* image);
	void sxRender2DImageSegmentOn3D(float left, float top, const SXImgImage* image, int x, int y, int width, int height);
	
	void sxRender2DLineOn3D(const float x1, const float y1, const float x2, const float y2, const SXColor color);
	void sxRender3DLine(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const SXColor color);
	
	void sxRender2DGradientOn3D(float left, float top, int width, int height, const SXColor topLeftColor, const SXColor topRightColor, const SXColor bottomLeftColor, const SXColor bottomRightColor);
	
/*
#ifndef _PSP
#pragma mark GUI
#endif
	void sxGuiScrollBar(int range, int value, int resolution);
	void sxGuiProgressBar(float perc);
*/
	
#ifndef _PSP
#pragma mark File IO
#endif
	
	int sxIoFileSize(const char *file);
	int sxIoIsDir(const char *path);
	int sxIoDeleteDir(const char *directory);
	
#ifdef __cplusplus
}
#endif

#endif
