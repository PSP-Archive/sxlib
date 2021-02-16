/*
 * sxlib.h
 * SXLibrary Version 0.1.5 by XArt Software & Designs http://www.xart.co.uk
 *
 * This work is not under any license, you are free to use and modify any part of this work you see fit.
 * No LICENSE for a free working together world.
 *
 */

#include <sxlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <pspwlan.h>
#include <psprtc.h>


#include <pspkernel.h>
#include <png.h>

#include <pspdisplay.h>

#include <pspctrl.h>

#include <pspgu.h>
#include <pspgum.h>


#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

#define SX_DRAW_BUFFER		0
#define SX_DISPLAY_BUFFER	0x88000
#define SX_DEPTH_BUFFER		0x110000

static unsigned int __attribute__((aligned(16))) list[262144];

typedef struct {	float			u,v;
					unsigned int	color;
					float			x, y, z;		} vertex;	// vertex to render

typedef struct {	float			u, v;
					float			x, y, z;		} vertex2d;

typedef struct {	unsigned int	color;
					float			x, y, z;		} lineVertex;



//extern unsigned char noimage_start[];

//extern unsigned char vscroll_start[];
//extern unsigned char progress_start[];

//SXImgImage *gVScroll = NULL;
//SXImgImage *gProgress = NULL;


uint32_t gSXDrawFrame = 0x4000000;


#ifndef _PSP
#pragma mark DRM Digital Rights Managment
#endif

uint8_t gDrmKey[] = {"ABCDEFGHIJKLMNOPQRSTUVWX"};
uint8_t gDrmPublicKey[24];

struct {
	uint32_t	appSig;			/* Binary */
	uint32_t	created;
	uint32_t	expire;
	char		devSig[12];		/* ASCII */
} SXDrmDRM;

uint32_t sxDrmGetBestBeforeDate(uint32_t yearInc, uint32_t monthInc, uint32_t dayInc)
{
	pspTime time;
	uint32_t BestBefore;
	uint16_t days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	sceRtcGetCurrentClock(&time, 0);
	
	time.day += dayInc;
	if (time.day > days[time.month - 1]) {
		time.day -=  days[time.month - 1];
		time.month ++;
	}
	
	time.month += monthInc;
	if (time.month > 12) {
		time.year += (time.month / 12);
		time.month = (time.month % 12);
	}
	
	time.year += yearInc;
	
	
	BestBefore = ( (uint32_t)time.year * 100 + (uint32_t)time.month ) * 100 + (uint32_t)time.day;
	
	return BestBefore;
}

void sxDrmSetEncryptionKey(const uint8_t *key) {
	memcpy(gDrmKey, key, sizeof(uint8_t) * 24);
}

void sxDrmEncrypt(const unsigned char *c ,void *data, uint32_t length)
{
	uint32_t i;
	unsigned char *p;
	
	p = (unsigned char *)data;
	for (i=0; i<length; i++) {
		*p ^= *(c + i);
		p++;
	}
}

#define sxDrmDecrypt	sxDrmEncrypt

void sxDrmGeneratePrivateKey(uint8_t *privateKey) {
	uint32_t i;
	uint8_t *p;
	
	/* Generate the public key that will be saved in the DRM file */
	for (i=0; i<24; i++) {
		gDrmPublicKey[i] = rand() % 255;
	}
	
	/* Using the generated public key, create the private key that is used to encrypt the data. */
	p = (uint8_t *)privateKey;
	for (i=0; i<24; i++) {
		*p = gDrmKey[i];
		*p ^= gDrmPublicKey[i];
		p++;
	}
}

void sxDrmRetrievePrivateKey(uint8_t *privateKey) {
	uint32_t i;
	uint8_t *p;
	
	/* Using the generated public key, create the private key that is used to encrypt the data. */
	p = (uint8_t *)privateKey;
	for (i=0; i<24; i++) {
		*p = gDrmKey[i];
		*p ^= gDrmPublicKey[i];
		p++;
	}
}

int sxDrmLoadDRM(void)
{
	FILE *fp;
	fp = fopen("data.drm", "rb");
	if (fp == NULL) return -1;
	fread(gDrmPublicKey, sizeof(uint8_t) * 24, 1, fp);
	fread(&SXDrmDRM, sizeof(SXDrmDRM), 1, fp);
	fclose(fp);
	
	uint8_t privateKey[24];
	sxDrmRetrievePrivateKey(privateKey);
	
	sxDrmDecrypt(privateKey, &SXDrmDRM, sizeof(SXDrmDRM));
	return 0;
}

int sxDrmSaveDRM(void)
{
	FILE *fp;
	fp = fopen("data.drm", "wb");
	if (fp == NULL) return -1;
	
	uint8_t privateKey[24];
	sxDrmGeneratePrivateKey(privateKey);
	
	sxDrmEncrypt(privateKey, &SXDrmDRM, sizeof(SXDrmDRM));
	
	fwrite(gDrmPublicKey, sizeof(uint8_t) * 24, 1, fp);
	fwrite(&SXDrmDRM, sizeof(SXDrmDRM), 1, fp);
	fclose(fp);
	return 0;
}


void sxDrmCreateDRMDataforApplication(uint32_t appSig, int signToDevice)
{
	SXDrmDRM.appSig = appSig;
	SXDrmDRM.created = sxDrmGetBestBeforeDate(0,0,0);
	SXDrmDRM.expire = sxDrmGetBestBeforeDate(0,0,1); /* DRM Day Activation Time */
	
	if (signToDevice) {
		uint8_t addr[8];
		memset(addr, 0, 7);
		if (sceWlanGetEtherAddr(addr) == 0) {
			char buf[16];
			sprintf(buf, "%02X%02X%02X%02X%02X%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
			memcpy(SXDrmDRM.devSig, buf, 12);
		} else memset(SXDrmDRM.devSig, 'F', 12);
	} else {
		memset(SXDrmDRM.devSig, 'F', 12);
	}
	
	sxDrmSaveDRM();
}

int sxDrmIsApplicationSignedToDevice(uint32_t appSig)
{
	uint8_t addr[8];
	int i;
	
	/* load DRM data */
	if(sxDrmLoadDRM() != 0) return 0;
	
	/* Check application sig of DRM data */
	if(SXDrmDRM.appSig != appSig) return 0;
	
	
	/* Check DRM data for valid created date */
	if (SXDrmDRM.created > sxDrmGetBestBeforeDate(0,0,0)) return 0;
	
	/* Check devive sig of DRM data */
	if(strncmp(SXDrmDRM.devSig, "XXXXXXXXXXXX", 12) == 0) return -1;
	memset(addr, 0, 7);
	if(sceWlanGetEtherAddr(addr) != 0) return 0;
	char buf[16];
	sprintf(buf, "%02X%02X%02X%02X%02X%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	for(i=0; i<12; i++) {
		if(SXDrmDRM.devSig[i] != buf[i]) return 0;
	}
	
	return 1;
}

int sxDrmSignApplicationToDevice(uint32_t appSig)
{
	uint8_t addr[8];
	
	/* load DRM data */
	if(sxDrmLoadDRM() != 0) return 0;
	
	/* Check application sig of DRM data */
	if(SXDrmDRM.appSig != appSig) return 0;
	
	if(strncmp(SXDrmDRM.devSig, "XXXXXXXXXXXX", 12) != 0) {
		/* Check DRM data for valid created and expire date */
		if (SXDrmDRM.created > sxDrmGetBestBeforeDate(0,0,0)) return 0;
		if (sxDrmGetBestBeforeDate(0,0,0) >= SXDrmDRM.expire) return 0;
	}
	
	memset(addr, 0, 7);
	if (sceWlanGetEtherAddr(addr) != 0) return 0;
	char buf[16];
	sprintf(buf, "%02X%02X%02X%02X%02X%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	memcpy(SXDrmDRM.devSig, buf, 12);
	
	/* save DRM data */
	if(sxDrmSaveDRM() != 0) return 0;
	
	return 1;
}

int sxDrmSignApplicationForAnyDevice(uint32_t appSig)
{	
	/* load DRM data */
	if(sxDrmLoadDRM() != 0) return 0;
	
	/* Check application sig of DRM data */
	if(SXDrmDRM.appSig != appSig) return 0;
	
	/* Check DRM data for valid created and expire UTC values */
	if (SXDrmDRM.created > sxDrmGetBestBeforeDate(0,0,0)) return 0;
	if (sxDrmGetBestBeforeDate(0,0,0) >= SXDrmDRM.expire) return 0;
	
	memcpy(SXDrmDRM.devSig, "XXXXXXXXXXXX", 12);
	
	/* save DRM data */
	if(sxDrmSaveDRM() != 0) return 0;
	
	return 1;
}


#ifndef _PSP
#pragma mark System
#endif

void sxInit(void) {
	
	
	// Init GU    
    sceGuInit();
	sceGuStart(GU_DIRECT, list);
	
	sceGuDrawBuffer(GU_PSM_8888, (void*)SX_DRAW_BUFFER, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)SX_DISPLAY_BUFFER, BUF_WIDTH);
	sceGuDepthBuffer((void*)SX_DEPTH_BUFFER, BUF_WIDTH);
	
	sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuDepthRange(65535, 0);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0,0);
	
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	// Init Ctrl
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	
	// Init GUI
	
	/*
	gVScroll = sxImgCreateImage(16,256);
	if (gVScroll != NULL) sxImgSetImage(vscroll_start, gVScroll);
	
	gProgress = sxImgCreateImage(128,32);
	if (gProgress != NULL) sxImgSetImage(progress_start, gProgress);
	 */
}

void sxExitApplication(void) {
	
	sceKernelExitGame();
}

void sxStartDrawing(void) {
	sceGuStart(GU_DIRECT, list);
	
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective( 75.0f, 16.0f/9.0f, 0.5f, 1000.0f);
	
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
	
	sceGuClearColor(0xff000000);
	sceGuClearDepth(0);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
}

void sxEndDrawing(void) {
	// End drawing
	sceGuFinish();
	sceGuSync(0,0);
	
	// Swap buffers (waiting for vsync)
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	
	gSXDrawFrame ^= SX_DISPLAY_BUFFER;
}

#ifndef _PSP
#pragma mark Platform
#endif

/*
 For checking if little-endian format is used by host.
 
 Return Value
 If the little-endian format is used then 1 will be returned else 0 will be returned for big-endian.
 */
int sxLittleEndian(void)
{
	int checkEndian = 1;  
	if( 1 == *(char *)&checkEndian ) return 1;
	return 0;
}

/*
 For checking if big-endian format is used by host.
 
 Return Value
 If the big-endian format is used then 1 will be returned else 0 will be returned for little-endian.
 */
int sxBigEndian(void)
{
	int checkEndian = 1;  
	if( 1 != *(char *)&checkEndian ) return 1;
	return 0;
}

/*
 Converts a 16-bit integer from big-endian format to the host native byte order.
 
 Parameters
 arg
 The integer whose bytes should be swapped.
 
 Return Value
 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
 */
int16_t sxSwapInt16BigToHost(int16_t arg)
{
	short int i=0;
	int checkEndian = 1;  
	if( 1 == *(char *)&checkEndian )
	{
		// Intel (little endian)
		i=arg;
		i=((i&0xFF00)>>8)|((i&0x00FF)<<8);
	}
	else
	{
		// PPC (big endian)
		i=arg;
	}
	return i;
}

/*
 Converts a 16-bit integer from little-endian format to the host native byte order.
 
 Parameters
 arg
 The integer whose bytes should be swapped.
 
 Return Value
 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
 */
int16_t sxSwapInt16LittleToHost(int16_t arg)
{
	short int i=0;
	int checkEndian = 1;  
	if( 1 == *(char *)&checkEndian )
	{
		// Intel (little endian)
		i=arg;
	}
	else
	{
		// PPC (big endian)
		i=arg;
		i=((i&0xFF00)>>8)|((i&0x00FF)<<8);
	}
	return i;
}

/*
 Converts a 32-bit integer from big-endian format to the host native byte order.
 
 Parameters
 arg
 The integer whose bytes should be swapped.
 
 Return Value
 The integer with its bytes swapped. If the host is big-endian, this function returns arg unchanged.
 */
int32_t sxSwapInt32BigToHost(int32_t arg)
{
	int i=0;
	int checkEndian = 1;  
	if( 1 == *(char *)&checkEndian )
	{
		// Intel (little endian)
		i=arg;
		i=((i&0xFF000000)>>24)|((i&0x00FF0000)>>8)|((i&0x0000FF00)<<8)|((i&0x000000FF)<<24);
	}
	else
	{
		// PPC (big endian)
		i=arg;
	}
	return i;
}

/*
 Converts a 32-bit integer from little-endian format to the host native byte order.
 
 Parameters
 arg
 The integer whose bytes should be swapped.
 
 Return Value
 The integer with its bytes swapped. If the host is little-endian, this function returns arg unchanged.
 */
int32_t sxSwapInt32LittleToHost(int32_t arg)
{
	int i=0;
	int checkEndian = 1;  
	if( 1 == *(char *)&checkEndian )
	{
		// Intel (little endian)
		i=arg;
	}
	else
	{
		// PPC (big endian)
		i=arg;
		i=((i&0xFF000000)>>24)|((i&0x00FF0000)>>8)|((i&0x0000FF00)<<8)|((i&0x000000FF)<<24);
	}
	return i;
}

#ifndef _PSP
#pragma mark Controller
#endif

float gCtrlMouseX = SCR_WIDTH / 2, gCtrlMouseY = SCR_HEIGHT / 2;
int gCtrlMouseEnabled = 0;

uint8_t gCtrlAnalogToDPadSensitivity = 0;

void sxCtrlEnableMouseEmulation(void) {
}

void sxCtrlDisableMouseEmulation(void) {
}

void sxCtrlAnalogEmulatedDPad(uint8_t sensitivity) {
	gCtrlAnalogToDPadSensitivity = sensitivity;
}

void sxCtrlReadControls(SXCtrlController *ctrl) {
	SceCtrlData ctl;
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(1);
	sceCtrlPeekBufferPositive( &ctl, 1 );
	
	ctrl->analogX = (signed)ctl.Lx-128;
	ctrl->analogY = (signed)ctl.Ly-128;
	
	if (gCtrlAnalogToDPadSensitivity && !gCtrlMouseEnabled) {
		ctl.Buttons |= (ctrl->analogY >= gCtrlAnalogToDPadSensitivity) ? SX_CTRL_DOWN	: 0;
		ctl.Buttons |= (ctrl->analogY <= gCtrlAnalogToDPadSensitivity) ? SX_CTRL_UP		: 0;
		ctl.Buttons |= (ctrl->analogX >= gCtrlAnalogToDPadSensitivity) ? SX_CTRL_RIGHT	: 0;
		ctl.Buttons |= (ctrl->analogX <= gCtrlAnalogToDPadSensitivity) ? SX_CTRL_LEFT	: 0;
	}
	
	if (gCtrlMouseEnabled) {
		ctrl->mouse.x = gCtrlMouseX;
		ctrl->mouse.y = gCtrlMouseY;
	}
	
	if (ctrl->autoRepeatInterval > 0) {		
		if (ctrl->lastHeld != ctl.Buttons)
			ctrl->autoRepeatCounter=0;
		else			{
			ctrl->autoRepeatCounter++;
			if (ctrl->autoRepeatCounter >= ctrl->autoRepeatInit) {
				if ((ctrl->autoRepeatCounter - ctrl->autoRepeatInit) % ctrl->autoRepeatInterval == 0)
					ctrl->lastHeld &= ~ctrl->autoRepeatMask;
			}
		}
	}
	
	ctrl->pressed = ~ctrl->lastHeld & ctl.Buttons;
	ctrl->released = ctrl->lastHeld & ~ctl.Buttons;
	
	ctrl->held = ctl.Buttons;
	ctrl->lastHeld = ctl.Buttons;
	
	return;
}

#ifndef _PSP
#pragma mark Textures
#endif

int sxTexPowerOfTwo(int i);
int sxTexMultipleOfEight(int i);

int sxTexPowerOfTwo(int i)
{
	if (i <=   2) return   2;
	if (i <=   4) return   4;
	if (i <=   8) return   8;
	if (i <=  16) return  16;
	if (i <=  32) return  32;
	if (i <=  64) return  64;
	if (i <= 128) return 128;
	if (i <= 256) return 256;
	
	return 512;
}


int sxTexMultipleOfEight(int i)
{
	int multiple = i / 8;
	if (i % 8) multiple++;
	return multiple * 8;
}

void sxTexSwizzle(SXImgImage *image)
{
	uint8_t* out;
	uint8_t* in;
	unsigned width;
	unsigned height;
	
	out = malloc(image->textureWidth * image->textureHeight * (image->pixelFormat == GU_PSM_8888 ? 4 : 2));
	if (! out) return;
	
	in = (uint8_t*)image->data;
	width = image->textureWidth * (image->pixelFormat == GU_PSM_8888 ? 4 : 2);
	height = image->textureHeight;
	
	
	unsigned int blockx, blocky;
	unsigned int j;
	
	unsigned int width_blocks = (width / 16);
	unsigned int height_blocks = (height / 8);
	
	unsigned int src_pitch = (width-16)/4;
	unsigned int src_row = width * 8;
	
	const uint8_t* ysrc = in;
	uint32_t* dst = (uint32_t*) out;
	
	for (blocky = 0; blocky < height_blocks; ++blocky)
	{
		const uint8_t* xsrc = ysrc;
		for (blockx = 0; blockx < width_blocks; ++blockx)
		{
			const uint32_t* src = (uint32_t*) xsrc;
			for (j = 0; j < 8; ++j)
			{
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src += src_pitch;
			}
			xsrc += 16;
		}
		ysrc += src_row;
	}
}


#ifndef _PSP
#pragma mark Images
#endif
SXImgImage* sxImgLoadPNGImageFromOffset(const char* filename, int offset);

SXImgImage* sxImgLoadImageFromOffset(const char *filename, uint32_t offset) {
	char sig[4];
	FILE *fp;
	
	fp = fopen(filename, "rb");
	if (fp == NULL) return 0;
	
	fseek(fp, offset, SEEK_SET);
	fread(sig, sizeof(char), sizeof(char) * 4, fp);
	fclose(fp);
	
	if (strncmp(sig, "\x89PNG", 4) == 0) return sxImgLoadPNGImageFromOffset(filename, offset);
	return 0;
}

/*
 * OCPicture.loadPNG Method
 *
 * Loads the specifyed imagetue image as a PNG format.
 * RGB or RGBA
 *
 * OCPicture.loadPNG filename, offset
 */

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}


SXImgImage* sxImgLoadPNGImageFromOffset(const char *filename, int offset)
{
	int number_passes, pass;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	uint32_t* line;
	FILE *fp;
	SXImgImage *image=NULL;
	
	if ((fp = fopen(filename, "rb")) == NULL) {
		return NULL;
	}
	
	fseek(fp,offset,SEEK_SET);
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fclose(fp);
		return NULL;
	}
	
	// Optinal error function currently not used in this class
	//png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	
	if(interlace_type) {
		fclose(fp);
		return NULL;
	}
	
	image=(SXImgImage*)malloc(sizeof(SXImgImage));
	if (!image) {
		fclose(fp);
		return NULL;
	}
	
	// New (the check) - Convert 16-bits per colour component to 8-bits per colour component.
	if (bit_depth == 16) png_set_strip_16(png_ptr);
	
	// Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	// byte into separate bytes (useful for paletted and grayscale images).
	png_set_packing(png_ptr);
	
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	
	// New - Convert grayscale to RGB triplets
	if ((color_type == PNG_COLOR_TYPE_GRAY) || (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)) png_set_gray_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = sxTexPowerOfTwo(width);
	image->textureHeight = sxTexMultipleOfEight(height);
	image->opacity = 1.0f;
	image->scaleWidth = 1.0f;
	image->scaleHeight = 1.0f;
	image->x = 0;
	image->y = 0;
	image->angle = 0;
	
	image->data = (SXColor *) memalign(16, image->textureWidth * image->textureHeight * sizeof(SXColor));
	if (!image->data) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	
	line = (uint32_t*) malloc(width * sizeof(SXColor));
	if (!line) {
		free(image->data);
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	
	// New - Turn on interlace handling.
	number_passes = png_set_interlace_handling(png_ptr);
	
	// New - Call to gamma correct and add the background to the palette
	// and update info structure.
	png_read_update_info(png_ptr, info_ptr);
	
	// New - Read the image, one line at a line
	for (pass = 0; pass < number_passes; pass++) {
		for (y = 0; y < (int)height; y++)
		{
			png_read_row(png_ptr, (uint8_t*) line, png_bytep_NULL);
			for (x = 0; x < (int)width; x++)
			{
				*(image->data + (x + y * image->textureWidth)) = *(line+x);
			}
		}
	}
	free(line);
	
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	return image;
}

SXImgImage* sxImgCreateImage(int width, int height)
{
	SXImgImage* image = (SXImgImage*) malloc(sizeof(SXImgImage));
	if (!image) return NULL;
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth =  sxTexPowerOfTwo(width);
	image->textureHeight = sxTexMultipleOfEight(height);
	image->opacity = 1.0f;
	image->scaleWidth = 1.0f;
	image->scaleHeight = 1.0f;
	image->x = 0;
	image->y = 0;
	image->angle = 0;
	image->data = (SXColor*) memalign(16, image->textureWidth * image->textureHeight * sizeof(SXColor));
	if (!image->data) {
		free(image);
		return NULL;
	}
	memset(image->data, 0, image->textureWidth * image->textureHeight * sizeof(SXColor));
	return image;
}


void sxImgFreeImage(SXImgImage* image)
{
	if (!image) return;
	if (!image->data) {
		free(image->data);
		return;
	}
	
	free(image->data);
	free(image);
}

void sxImgClearImage(SXColor color, SXImgImage* image)
{
	int i;
	int size = image->textureWidth * image->textureHeight;
	
	SXColor* data = image->data;
	
	if (!image) return;
	if (!image->data) return;
	
	for (i = 0; i < size; i++, data++) *data = color;
}

void sxImgSetImage(void *data, SXImgImage* image) {
	if (!image) return;
	if (!image->data) return;
	memcpy(image->data, data, image->textureWidth * image->textureHeight * sizeof(SXColor));
}


#ifndef _PSP
#pragma mark Graphics
#endif
void sxRenderQuick2DImageOn3D(int left, int top, const SXImgImage* image) {
	if (!image) return;
	if (!image->data) return;
	
	sceGuCopyImage(GU_PSM_8888,0,0,image->imageWidth,image->imageHeight,image->textureWidth,(void *)image->data,left, top,512,(void*)gSXDrawFrame);
}

void sxRender2DImageOn3D(float left, float top, const SXImgImage* image) {
	sxRender2DImageSegmentOn3D(left, top, image, 0, 0, image->imageWidth, image->imageHeight);
}

void sxRender2DImageRotOn3D(float left, float top, const SXImgImage* image, int x, int y, int width, int height) {
	if (!image) return;
	if (!image->data) return;
	
	sceGuTexImage(0, image->textureWidth, image->textureHeight, image->textureWidth, (void*) image->data);
	float u = 1.0f / ((float)image->textureWidth);
	float v = 1.0f / ((float)image->textureHeight);
	sceGuTexScale(u, v);
	
	int j = 0;
	while (j < width) {
		vertex2d* vertices = (vertex2d*) sceGuGetMemory(2 * sizeof(vertex2d));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		//float xrot, yrot;
		//xrot = sinf(rad);
		//yrot = cosf(rad);
		vertices[0].u = (x + j);
		vertices[0].v = (y);
		vertices[0].x = (left + j);
		vertices[0].y = (top);
		vertices[0].z = 0;
		vertices[1].u = (x + j + sliceWidth);
		vertices[1].v = (y + height);
		vertices[1].x = (left + j + sliceWidth);
		vertices[1].y = (top + height);
		vertices[1].z = 0;
		//sceGumRotateX(rad);
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
}

void sxRender2DImageSegmentOn3D(float left, float top, const SXImgImage* image, int x, int y, int width, int height) {
	if (!image) return;
	if (!image->data) return;
	
	vertex2d* DisplayVertices = (vertex2d*) sceGuGetMemory(4 * sizeof(vertex2d));
	
	// we do not need to test for depth
	sceGuDisable(GU_DEPTH_TEST); 
	// setting the texture
	if (image->opacity != 1.0f) {
		sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
		sceGuAmbientColor(0x00ffffff | ((uint32_t)(255.0f * image->opacity) << 24));
	} else {
		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	}
	sceGuTexMode(GU_PSM_8888, 0 ,0 ,0);
	sceGuTexImage(0, image->textureWidth, image->textureHeight, image->textureWidth, (void*) (image->data + y * image->textureWidth + x));
	
	float sWidth = (float)width * image->scaleWidth;
	float sHeight = (float)height * image->scaleHeight;
	
	// setting the 4 vertices			
	DisplayVertices[0].u = 0.0f;
	DisplayVertices[0].v = 0.0f;
	DisplayVertices[0].x = left;
	DisplayVertices[0].y = top;
	DisplayVertices[0].z = 0.0f;
	
	DisplayVertices[1].u = width;
	DisplayVertices[1].v = 0.0f;
	DisplayVertices[1].x = left + sWidth;
	DisplayVertices[1].y = top;
	DisplayVertices[1].z = 0.0f;
	
	DisplayVertices[2].u = 0.0f;
	DisplayVertices[2].v = height;
	DisplayVertices[2].x = left;
	DisplayVertices[2].y = top + sHeight;
	DisplayVertices[2].z = 0.0f;
	
	DisplayVertices[3].u = width;
	DisplayVertices[3].v = height;
	DisplayVertices[3].x = left + sWidth;
	DisplayVertices[3].y = top + sHeight;
	DisplayVertices[3].z = 0.0f;
	
	// draw the trianglestrip with transform 2D
	sceGuDrawArray(GU_TRIANGLE_STRIP, GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 4, 0, DisplayVertices);
	
	// enable the depthtesting again.
	sceGuEnable(GU_DEPTH_TEST); 
}

void sxRender2DLineOn3D(const float x1, const float y1, const float x2, const float y2, const SXColor color) {
	
	sceGuDisable(GU_TEXTURE_2D);
	sceGuDisable(GU_DEPTH_TEST);
	
	lineVertex* Line = (lineVertex*)sceGuGetMemory(2 * sizeof(lineVertex));
	
	Line[0].color = color;
	Line[0].x = x1;
	Line[0].y = y1;
	Line[0].z = 0.0f;
	
	Line[1].color = color;
	Line[1].x = x2;
	Line[1].y = y2;
	Line[1].z = 0.0f;
	
	
	sceGuDrawArray(GU_LINES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, Line);
	
	sceGuEnable(GU_DEPTH_TEST);
	sceGuEnable(GU_TEXTURE_2D);
}

void sxRender3DLine(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2, const SXColor color) {
	
	sceGuDisable(GU_TEXTURE_2D);
	
	lineVertex* Line = (lineVertex*)sceGuGetMemory(2 * sizeof(lineVertex));
	
	Line[0].color = color;
	Line[0].x = x1;
	Line[0].y = y1;
	Line[0].z = z1;
	
	Line[1].color = color;
	Line[1].x = x2;
	Line[1].y = y2;
	Line[1].z = z1;
	
	sceGumMatrixMode(GU_MODEL); 
	sceGumLoadIdentity();
	sceGuDrawArray(GU_LINES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 2, 0, Line);
	sceGuEnable(GU_TEXTURE_2D);
}

void sxRender2DGradientOn3D(float left, float top, int width, int height, const SXColor topLeftColor, const SXColor topRightColor, const SXColor bottomLeftColor, const SXColor bottomRightColor) {
	lineVertex* DisplayVertices = (lineVertex*) sceGuGetMemory(4 * sizeof(lineVertex));
	
	// we do not need to test for depth
	sceGuDisable(GU_DEPTH_TEST);
	
	// disable texture
	sceGuDisable(GU_TEXTURE_2D);
	
	
	// setting the 4 vertices			
	DisplayVertices[0].color = topLeftColor;
	DisplayVertices[0].x = left;
	DisplayVertices[0].y = top;
	DisplayVertices[0].z = 0.0f;
	
	DisplayVertices[1].color = topRightColor;
	DisplayVertices[1].x = left + width;
	DisplayVertices[1].y = top;
	DisplayVertices[1].z = 0.0f;
	
	DisplayVertices[2].color = bottomLeftColor;
	DisplayVertices[2].x = left;
	DisplayVertices[2].y = top + height;
	DisplayVertices[2].z = 0.0f;

	DisplayVertices[3].color = bottomRightColor;
	DisplayVertices[3].x = left + width;
	DisplayVertices[3].y = top + height;
	DisplayVertices[3].z = 0.0f;
	
	// draw the trianglestrip with transform 2D
	sceGuDrawArray(GU_TRIANGLE_STRIP, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 4, 0, DisplayVertices);
	
	// enable the depthtesting again.
	sceGuEnable(GU_DEPTH_TEST);
	
	sceGuEnable(GU_TEXTURE_2D);
}

/*
#ifndef _PSP
#pragma mark GUI
#endif


void sxGuiScrollBar(int range, int value, int resolution) {
	// Draw vertical scroll bar.
	if (range >= resolution) {
		sxRender2DImageSegmentOn3D(464,8,gVScroll, 0, 0, 8, 256);
		
		int thumbHeight = (int)( 256.0f * ((float)resolution / (float)range) );
		int thumbOffset = (int)( (256.0f - (float)thumbHeight) * ((float)value / (float)(range - 1)) );
		
		sxRender2DImageSegmentOn3D(464,8 + thumbOffset, gVScroll, 8, 0, 8, thumbHeight - 4);
		sxRender2DImageSegmentOn3D(464,8 + thumbOffset + thumbHeight - 4, gVScroll, 8, 256 - 4, 8, 4);
	}	
}


void sxGuiProgressBar(float perc) {
	int i = (128.0f * perc);
	sxRender2DImageSegmentOn3D(176,128,gProgress, 0, 0, 128, 16);
	sxRender2DImageSegmentOn3D(176,128,gProgress, 0, 16, i, 16);
}
*/

#ifndef _PSP
#pragma mark File IO
#endif

int sxIoFileSize(const char *file)
{
	FILE *fp;
	if ((fp = fopen(file, "rb")) == NULL)
		return 0;
	else {
		fseek(fp,0,SEEK_END);
		int size = ftell(fp);
		fclose(fp);
		return size;
	}
}

/*
 *
 * Returns 1 if directory exist else returns 0
 * 
 */

int sxIoIsDir(const char *path)
{
	struct stat sbuf;
	stat(path, &sbuf);
	
	return (sbuf.st_mode & S_IFDIR) ? 1 : 0;
}

int sxIoDeleteDir(const char *directory)
{
	DIR *dir_ptr;
	struct dirent *entry;
	char str[256];
	
	if ((dir_ptr = opendir(directory)) == NULL) return 0;
	while ((entry = readdir(dir_ptr)) != NULL) {
		if (strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) continue;
		sprintf(str,"%s/%s",directory,entry->d_name);
		if (sxIoIsDir(str)) {
			sxIoDeleteDir(str);
			rmdir(str);
		} else remove(str);
		
	}
	closedir(dir_ptr);
	rmdir(directory);
	return 1;
}


