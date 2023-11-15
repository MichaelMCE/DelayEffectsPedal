#pragma once
#ifndef _PNGCOMMON_H_
#define _PNGCOMMON_H_


#include "../libpng/png.h"
#include "../common.h"


typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}__attribute__ ((packed))TRGB;		// 888


typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}__attribute__ ((packed))TRGBA;		// 8888



static inline void setPixel8 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//uint16_t *pixels = (uint16_t*)buffer;	
	
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint8_t*)(buffer+(y*pitch) + x) = colour;
}


// non packed 12bit
static inline void setPixel12 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint16_t*)(buffer+(y*pitch) + (x<<1)) = colour; //&0x0FFF
}

static inline void setPixel16 (uint8_t *buffer, const int pitch, const int x, const int y, uint16_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;
	
	*(uint16_t*)(buffer+(y*pitch) + (x<<1)) = colour;
}

static inline void setPixel24 (uint8_t *buffer, const int pitch, const int x, const int y, uint32_t colour)
{
	//if (x >= WIDTH || x < 0) return;
	//if (y >= HEIGHT || y < 0) return;

	TRGB *addr = (TRGB*)(buffer+((y*pitch)+(x*3)));
	//*addr = *_addr;
		
	TRGB *_addr = (TRGB*)&colour;
	*addr = *_addr;
}

static inline intptr_t *getPixelAddress32 (uint8_t *frame, const int pitch, const int x, const int y)
{
	return (intptr_t*)(frame+((y*pitch)+(x<<2)));
}

static inline void readPng32To16_565 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int32_t ox, int32_t oy, int passCount, const uint32_t pitch)
{
	//const int pitch = width<<1;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<8;	// 5
			int g = (line[i++]&0xFC)<<3;	// 6
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

static inline void readPng32To16_555 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
	
	while(passCount--){	// interlacing requires multiple passes
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF8)<<7;	// 5
			int g = (line[i++]&0xF8)<<2;	// 5
			int b = (line[i++]&0xF8)>>3;	// 5
			setPixel16(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
	}
}

static inline void readPng32To24 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width*3;
	
	while(passCount--){
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++, i += 4){
			setPixel24(frame, pitch, x+ox, y+oy, line[i]<<16|line[i+1]<<8|line[i+2]);
		}
	}
	}
}


static inline void readPng32To32A_cpy (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<2;
	
	while(passCount--)
	for (int y = oy; y < (const int)(oy+height); y++){
		png_read_row(*png_ptr, line, NULL);
		TRGBA *p = (TRGBA*)getPixelAddress32(frame, pitch, ox, y);
		if (!p) continue;

		int i = 0;
		for (int x = 0; x < width; x++){
			p[x].b = line[i++];
			p[x].g = line[i++];
			p[x].r = line[i++];
			p[x].a = line[i++];
		}
	}
}

inline void readPng32To8 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width;
		
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xE0);
			int g = (line[i++]&0xE0)>>3;
			int b = (line[i++]&0xC0)>>6;
			setPixel8(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}

inline void readPng32To12 (uint8_t *frame, png_structp *png_ptr, int width, int height, uint8_t *line, int ox, int oy, int passCount)
{
	const int pitch = width<<1;
		
	while(passCount--)
	for (int y = 0; y < height; y++){
		png_read_row(*png_ptr, line, NULL);
		int i = 0;
		for (int x = 0; x < width; x++){
			int r = (line[i++]&0xF0)<<4;
			int g = (line[i++]&0xF0);
			int b = (line[i++]&0xF0)>>4;
			setPixel12(frame, pitch, x+ox, y+oy, r|g|b);
			i++;
		}
	}
}


#endif

