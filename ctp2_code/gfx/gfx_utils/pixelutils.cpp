#include "c3.h"

#include "pixelutils.h"
#ifdef WIN32
#include <windows.h>
#else
#include "windows.h"
#endif

Pixel16		gRGBTable[RGB_VALUES];

extern sint32 g_is565Format;

void pixelutils_Initialize(void)
{
	pixelutils_ComputeRGBTable();
}

Pixel16 *RGB32ToRGB16(char *buf, uint16 width, uint16 height)
{
	uint16_t	*outBuf;
	uint16_t  *destPixel = (uint16_t *)malloc(width * height * 2);
	uint32_t	*srcPixel = (uint32_t *)buf;

	outBuf = destPixel;

	if (outBuf == NULL) return NULL;

	for(int i=0; i<width*height; i++) {
		uint32_t pix = *srcPixel;
		uint16_t r, g, b;
		uint8_t a;

		r = (uint16_t) ((pix & 0x000000FF) >> 0);
		g = (uint16_t) ((pix & 0x0000FF00) >> 8);
		b = (uint16_t) ((pix & 0x00FF0000) >> 16);
		a = (uint8_t) ((pix & 0xFF000000) >> 24);

		if (g_is565Format)
			*destPixel = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3));		
		else
			*destPixel = (Pixel16) (((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3));
		
		destPixel++;
		srcPixel++;
	}

	return (Pixel16 *)outBuf;
}

void RGB32Components(Pixel32 pixel, Pixel16 *r, Pixel16 *g, Pixel16 *b, Pixel16 *a)
{
	*r = (Pixel16) ((pixel & 0x000000FF) >> 0);
	*g = (Pixel16) ((pixel & 0x0000FF00) >> 8);
	*b = (Pixel16) ((pixel & 0x00FF0000) >> 16);
	*a = (Pixel16) ((pixel & 0xFF000000) >> 24);
}

void RGB32Info(Pixel32 pixel, Pixel16 *outPixel, uint8_t *alpha)
{
	uint16_t r, g, b, a;

	RGB32Components(pixel, &r, &g, &b, &a);

	*alpha = (uint8_t) a;

	if (g_is565Format)
		*outPixel = (Pixel16) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3));
	else
		*outPixel = (Pixel16) (((r & 0xF8) << 7) | ((g & 0xF8) << 2) | ((b & 0xF8) >> 3));
}

void pixelutils_ComputeRGBTable(void)
{
	for (int r = 0; r < 32; r++)
    {
		for (int g = 0; g < 32; g++)
        {
			for (int b = 0; b < 32; b++)
            {
                Pixel16 rgb555 = (r << 10) | (g << 5) | b;
				gRGBTable[rgb555] = rgb555;
            }
        }
    }
}

Pixel16 pixelutils_RGB(int r,int g,int b)
{
	if (g_is565Format)
	{
		Pixel16 temp = gRGBTable[(r<<10) | (g<<5) | b];
		uint16_t rg = (temp & 0x7FE0) << 1;
		uint16_t b = (temp & 0x001F);
		return (rg | b);
	}
	else
		return gRGBTable[(r<<10) | (g<<5) | b];
}

Pixel32 ComponentsToRGB32(Pixel16 r, Pixel16 g, Pixel16 b, Pixel16 a) 
{
	Pixel32		pix32;

	pix32 = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);

	return pix32;
}

Pixel16 pixelutils_Desaturate(Pixel16 pixel)
{
	extern sint32		g_is565Format;

	Pixel16		tempPix;
	sint32		ave;

	if (g_is565Format) 
	{
		ave = (((pixel & 0xF800)>>11) + ((pixel & 0x07E0) >> 6) + (pixel & 0x001F)+128)>>2;

		tempPix = (Pixel16)(((ave & 0x1F) << 11) | ((ave & 0x3F) << 6) | (ave & 0x1F));
	} else {
		ave = (((pixel & 0x7C00) >> 10) + ((pixel & 0x03E0) >> 5) + (pixel & 0x001F)+128)>>2;

		tempPix = (Pixel16)(((ave & 0x1F) << 10) | ((ave & 0x1F) << 5) | (ave & 0x1f));
	}

	return tempPix;
}

#if 0

Pixel16 pixelutils_Additive_565(Pixel16 pixel1, Pixel16 pixel2)
{
	Pixel16				r, g, b, sum = (uint16_t)(pixel2 & 0x1F) ;

	r = (uint16_t)((pixel1 & 0xF800) >> 11) ;
	g = (uint16_t)((pixel1 & 0x07E0) >> 5) ;
	b = (uint16_t)((pixel1 & 0x001F)) ;

	r += sum;
	if (r > 0x001F) r = 0x001F;
	g += (sum << 1);
	if (g > 0x003F) g = 0x003F;
	b += sum;
	if (b > 0x001F) b = 0x001F;

	return ((r<<11) | (g<<5) | b);
}

Pixel16 pixelutils_Lightening_565(Pixel16 pixel)
{
	uint16_t r, g, b;

	r = (pixel & 0xF800) >> 10;
	if (r > 0x001F)
   		r = 0x001F;
	
	g = (pixel & 0x07E0) >> 4;
	if (g > 0x003F)
   		g = 0x003F;
	
	b = (pixel & 0x001F) << 1;
	if (b > 0x001F)
   		b = 0x001F;

   return ((r<<11) | (g<<5) | b);
}

Pixel16 pixelutils_PercentDarken_565(Pixel16 pixel, sint32 percent)
{
	sint32 r32, g32, b32;
	sint16 r, g, b;
	sint32 newPercent = k_MAX_PERCENT - percent;

	r32 = (pixel & 0xF800) >> 11;
	g32 = (pixel & 0x07E0) >> 5;
	b32 = (pixel & 0x001F);

	r32 = (r32 * newPercent) >> k_PERCENT_SHIFT;
	r = (sint16)r32;
	g32 = (g32 * newPercent) >> k_PERCENT_SHIFT;
	g = (sint16)g32;
	b32 = (b32 * newPercent) >> k_PERCENT_SHIFT;
	b = (sint16)b32;

	return ((r<<11) | (g<<5) | b);
}

Pixel16 pixelutils_PercentLighten_565(Pixel16 pixel, sint32 percent)
{
	sint32 r32, g32, b32;
	sint16 r, g, b;
	sint32 newPercent = k_MAX_PERCENT + percent;

	r32 = (pixel & 0xF800) >> 11;
	g32 = (pixel & 0x07E0) >> 5;
	b32 = (pixel & 0x001F);

	r32 = (r32 * newPercent) >> k_PERCENT_SHIFT;
	if (r32 > 0x001F)
		r32 = 0x001F;
	r = (sint16)r32;
	g32 = (g32 * newPercent) >> k_PERCENT_SHIFT;
	if (g32 > 0x003F)
		g32 = 0x003F;
	g = (sint16)g32;
	b32 = (b32 * newPercent) >> k_PERCENT_SHIFT;
	if (b32 > 0x001F)
		b32 = 0x001F;
	b = (sint16)b32;

	return ((r<<11) | (g<<5) | b);
}

Pixel16 pixelutils_Desaturate_565(Pixel16 pixel)
{
	sint32 ave = (((pixel & 0xF800) >> 11) + ((pixel & 0x07E0) >> 6) + (pixel & 0x001F)) / 3;

	return ((Pixel16)(((ave & 0x1F) << 11) | ((ave & 0x3F) << 6) | (ave & 0x1F)));
}
#endif

Pixel32 pixelutils_Blend32_565(Pixel32 pixel1, Pixel32 pixel2, uint16_t blend);
Pixel32 pixelutils_Additive32_565(Pixel32 pixel1, Pixel32 pixel2);
Pixel32 pixelutils_BlendFast32_565(sint32 pixel1, sint32 pixel2, sint32 blend);
Pixel32 pixelutils_Shadow32_565(Pixel32 pixel);
Pixel32 pixelutils_Lightening32_565(Pixel16 pixel);
Pixel32 pixelutils_PercentDarken32_565(Pixel32 pixel, sint32 percent);
Pixel32 pixelutils_PercentLighten32_565(Pixel32 pixel, sint32 percent);

Pixel32 pixelutils_Blend32_555(Pixel32 pixel1, Pixel32 pixel2, uint16_t blend);
Pixel32 pixelutils_Additive32_555(Pixel32 pixel1, Pixel32 pixel2);
Pixel32 pixelutils_BlendFast32_555(Pixel32 pixel1,Pixel32 pixel2, sint32 blend);
Pixel32 pixelutils_Shadow32_555(Pixel32 pixel);
Pixel32 pixelutils_Lightening32_555(Pixel32 pixel);
Pixel32 pixelutils_PercentDarken32_555(Pixel32 pixel, sint32 percent);
Pixel32 pixelutils_PercentLighten32_555(Pixel32 pixel, sint32 percent);
