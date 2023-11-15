
//  Copyright (c) Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#pragma once
#ifndef _UFONT_PRIMITIVES_H_
#define _UFONT_PRIMITIVES_H_



// 1bpp primitives. draws to the selected target surface (fontSetRenderSurface())
void fontDrawLine (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set);		// 1:set pixel, 0:clear/remove/unset pixel
void fontDrawRectangle (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set);
void fontDrawRectangleFilled (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set);
void fontDrawCircle (_ufont_t *font, const int xc, const int yc, const int radius, const uint32_t set);
void fontDrawCircleFilled (_ufont_t *font, int xc, int yc, const int radius, const uint32_t set);
void fontDrawEllipse (_ufont_t *font, const int x, const int y, const int r1, const int r2, const uint32_t set);
void fontDrawArc (_ufont_t *font, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint32_t set);
void fontInvertRectangle (_ufont_t *font, int x1, int y1, int x2, int y2);
void fontDrawTriangle (_ufont_t *font, int x1, int y1, int x2, int y2, int x3, int y3, const uint32_t set);
void fontDrawTriangleFilled (_ufont_t *font, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const uint32_t set);
void fontDrawLineDotted (_ufont_t *font, int x1, int y1, int x2, int y2, const uint32_t set);


// 16bit primitives. draw to distination/display surface (fontS/GetDisplayBuffer())
void surfaceDrawLine (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour);
void surfaceDrawRectangle (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour);
void surfaceDrawRectangleFilled (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour);
void surfaceDrawCircle (_ufont_surface_t *surface, const int xc, const int yc, const int radius, const uint16_t colour);
void surfaceDrawCircleFilled (_ufont_surface_t *surface, int xc, int yc, const int radius, const uint16_t colour);
void surfaceDrawEllipse (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, const uint16_t colour);
void surfaceDrawArc (_ufont_surface_t *surface, const int x, const int y, const int r1, const int r2, float a1, float a2, const uint16_t colour);
void surfaceDrawTriangle (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, int x3, int y3, const uint16_t colour);
void surfaceDrawTriangleFilled (_ufont_surface_t *surface, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const uint16_t colour);
void surfaceDrawLineDotted (_ufont_surface_t *surface, int x1, int y1, int x2, int y2, const uint16_t colour);

// width bound checking
void surfaceDrawImage (_ufont_surface_t *surface, _ufont_surface_t *image, const int isCentered, const int cx, const int cy);

// faster than surfaceDrawImage but without the per pixel bound checks
void surfaceBlitImage (_ufont_surface_t *surface, _ufont_surface_t *image, const int dx, const int dy);


// set without bound checking
static inline void setPixel1_nb (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	*(surface->pixels+((y*pitch)+(x>>3))) |= (1<<(x&7));
}

static inline void unsetPixel1_nb (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	*(surface->pixels+((y*pitch)+(x>>3))) &= ~(1<<(x&7));
}

// set with bound checking
static inline void setPixel1_bc (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		setPixel1_nb(surface, pitch, x, y);
}

static inline void unsetPixel1_bc (_ufont_surface_t *surface, const int pitch, const int x, const int y)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		*(surface->pixels+((y*pitch)+(x>>3))) &= ~(1<<(x&7));
}

static inline void unsetPixel1 (uint8_t *pixels, const int x)
{
	*(pixels+(x>>3)) &= ~(1<<(x&7));
}

static inline void setPixel1 (uint8_t *pixels, const int x)
{
	*(pixels+(x>>3)) |= (1<<(x&7));
}

static inline int getPixel1 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(pixels+((y * pitch)+(x>>3))) >> (x&7)&0x01;
}

static inline uint8_t getPixels8 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(pixels+((y * pitch)+(x>>3)));
}

static inline uint32_t getPixels32 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *(uint32_t*)(pixels+((y * pitch)+(x>>3)));
}

static inline void setPixels8 (uint8_t *pixels, const int pitch, const int x, const int y, uint8_t data)
{
	*(pixels+((y * pitch)+(x>>3))) = data;
}

static inline void setPixel16_bc (_ufont_surface_t *surface, const int x, const int y, const uint16_t colour)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		*((uint16_t*)(surface->pixels+((surface->width<<1)*y)+(x<<1))) = colour;
}

static inline void setPixel16_nb (_ufont_surface_t *surface, const int x, const int y, const uint16_t colour)
{
	*((uint16_t*)(surface->pixels+((surface->width<<1)*y)+(x<<1))) = colour;
}

static inline uint16_t getPixel16 (uint8_t *pixels, const int pitch, const int x, const int y)
{
	return *((uint16_t*)(pixels+(pitch*y)+(x<<1)));
}

static inline uint16_t getPixel16_bc (_ufont_surface_t *surface, const int x, const int y)
{
	if (x >= 0 && x < surface->width && y >= 0 && y < surface->height)
		return *((uint16_t*)(surface->pixels+((surface->width<<1)*y)+(x<<1)));
	else
		return 0;
}

static inline void surfaceDrawCopy (_ufont_surface_t *surface, const int x1, const int y1, const int x2, const int y2, const int des_x, const int des_y)
{
	int dy = des_y;
		
	for (int y = y1; y <= y2; y++, dy++){
		int dx = des_x;
		for (int x = x1; x <= x2; x++){
			uint16_t col = getPixel16_bc(surface, x, y);
			setPixel16_bc(surface, dx++, dy, col);
		}
	}
}

#endif
