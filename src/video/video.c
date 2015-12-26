/*****************************************************************************
 * Copyright (c) 2015 Lewis Fox
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "video.h"
#include "../config.h"
#include "../drawing/drawing.h"
#include "../game.h"
#include "../openrct2.h"

#include "surface.h"
#include "renderer.h"

enum {
	MODE_REFRESH,
	MODE_DRAW,
	MODE_CLOSE,
	MODE_UPDATE_PALETTE,
	MODE_FUNCTION_COUNT
};

static void (*_displayModes[2][MODE_FUNCTION_COUNT])() = {
	{
		surface_refresh,
		surface_draw,
		surface_close,
		surface_update_palette,
	},
	{
		renderer_refresh,
		renderer_draw,
		renderer_close,
		renderer_update_palette,
	},
};

SDL_Color gPalette[256];

int gHardwareDisplay;

void *gScreenBuffer;
int gScreenBufferSize;
int gScreenBufferWidth;
int gScreenBufferHeight;
int gScreenBufferPitch;

void video_refresh_screenbuffer(int width, int height, int pitch)
{
	int newScreenBufferSize = pitch * height;
	char *newScreenBuffer = (char*)malloc(newScreenBufferSize);
	if (gScreenBuffer == NULL) {
		memset(newScreenBuffer, 0, newScreenBufferSize);
	} else {
		if (gScreenBufferPitch == pitch) {
			memcpy(newScreenBuffer, gScreenBuffer, min(gScreenBufferSize, newScreenBufferSize));
		} else {
			char *src = gScreenBuffer;
			char *dst = newScreenBuffer;
			
			int minWidth = min(gScreenBufferWidth, width);
			int minHeight = min(gScreenBufferHeight, height);
			for (int y = 0; y < minHeight; y++) {
				memcpy(dst, src, minWidth);
				if (pitch - minWidth > 0)
					memset(dst + minWidth, 0, pitch - minWidth);
				
				src += gScreenBufferPitch;
				dst += pitch;
			}
		}
		//if (newScreenBufferSize - gScreenBufferSize > 0)
		//	memset((uint8*)newScreenBuffer + gScreenBufferSize, 0, newScreenBufferSize - gScreenBufferSize);
		free(gScreenBuffer);
	}
	
	gScreenBuffer = newScreenBuffer;
	gScreenBufferSize = newScreenBufferSize;
	gScreenBufferWidth = width;
	gScreenBufferHeight = height;
	gScreenBufferPitch = pitch;
	
	rct_drawpixelinfo *screenDPI;
	screenDPI = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);
	screenDPI->bits = gScreenBuffer;
	screenDPI->x = 0;
	screenDPI->y = 0;
	screenDPI->width = width;
	screenDPI->height = height;
	screenDPI->pitch = gScreenBufferPitch - width;
	
	RCT2_GLOBAL(0x009ABDF0, uint8) = 6;
	RCT2_GLOBAL(0x009ABDF1, uint8) = 3;
	RCT2_GLOBAL(0x009ABDF2, uint8) = 1;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_WIDTH, uint16) = 64;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_HEIGHT, uint16) = 8;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_COLUMNS, uint32) = (width >> 6) + 1;
	RCT2_GLOBAL(RCT2_ADDRESS_DIRTY_BLOCK_ROWS, uint32) = (height >> 3) + 1;
}

void video_refresh()
{
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, gConfigGeneral.minimize_fullscreen_focus_loss ? "1" : "0");
	_displayModes[gHardwareDisplay][MODE_REFRESH]();
}

void video_draw()
{
	_displayModes[gHardwareDisplay][MODE_DRAW]();
}

static uint8 soft_light(uint8 a, uint8 b)
{
	float fa = a / 255.0f;
	float fb = b / 255.0f;
	float fr;
	if (fb < 0.5f) {
		fr = (2 * fa * fb) + ((fa * fa) * (1 - (2 * fb)));
	} else {
		fr = (2 * fa * (1 - fb)) + (sqrtf(fa) * ((2 * fb) - 1));
	}
	return (uint8)(clamp(0.0f, fr, 1.0f) * 255.0f);
}

static uint8 lerp(uint8 a, uint8 b, float t)
{
	if (t <= 0) return a;
	if (t >= 1) return b;
	
	int range = b - a;
	int amount = (int)(range * t);
	return (uint8)(a + amount);
}

void video_update_palette(const uint8* colours, int start_index, int num_colours)
{
	SDL_Surface *surface;
	int i;
	colours += start_index * 4;
	
	for (i = start_index; i < num_colours + start_index; i++) {
		gPalette[i].r = colours[2];
		gPalette[i].g = colours[1];
		gPalette[i].b = colours[0];
		gPalette[i].a = 0;
		
		float night = gDayNightCycle;
		if (night >= 0 && RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 0) {
			gPalette[i].r = lerp(gPalette[i].r, soft_light(gPalette[i].r, 8), night);
			gPalette[i].g = lerp(gPalette[i].g, soft_light(gPalette[i].g, 8), night);
			gPalette[i].b = lerp(gPalette[i].b, soft_light(gPalette[i].b, 128), night);
		}
		
		colours += 4;
	}
	
	if (!gOpenRCT2Headless) {
		_displayModes[gHardwareDisplay][MODE_UPDATE_PALETTE]();
	}
}

void video_init() {
	// Set the highest palette entry to white.
	// This fixes a bug with the TT:rainbow road due to the
	// image not using the correct white palette entry.
	gPalette[255].a = 0;
	gPalette[255].r = 255;
	gPalette[255].g = 255;
	gPalette[255].b = 255;
}

void video_close() {
	_displayModes[gHardwareDisplay][MODE_CLOSE]();
}
