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

#include "renderer.h"
#include "../addresses.h"
#include "../common.h"
#include "../config.h"
#include "../game.h"
#include "video.h"

static SDL_Renderer *_renderer = NULL;
static SDL_Texture *_bufferTexture = NULL;
static SDL_PixelFormat *_bufferTextureFormat = NULL;
static uint32 _paletteHWMapped[256];

static uint32 _pixelBeforeOverlay;
static uint32 _pixelAfterOverlay;

static void read_center_pixel(int width, int height, uint32 *pixel) {
	SDL_Rect centerPixelRegion = {width / 2, height / 2, 1, 1};
	SDL_RenderReadPixels(_renderer, &centerPixelRegion, SDL_PIXELFORMAT_RGBA8888, pixel, sizeof(uint32));
}

// Should be called before SDL_RenderPresent to capture frame buffer before Steam overlay is drawn.
static void overlay_pre_render_check(int width, int height) {
	read_center_pixel(width, height, &_pixelBeforeOverlay);
}

// Should be called after SDL_RenderPresent, when Steam overlay has had the chance to be drawn.
static void overlay_post_render_check(int width, int height) {
	static bool overlayActive = false;
	static bool pausedBeforeOverlay = false;
	
	read_center_pixel(width, height, &_pixelAfterOverlay);
	
	// Detect an active Steam overlay by checking if the center pixel is changed by the gray fade.
	// Will not be triggered by applications rendering to corners, like FRAPS, MSI Afterburner and Friends popups.
	bool newOverlayActive = _pixelBeforeOverlay != _pixelAfterOverlay;
	
	// Toggle game pause state consistently with base pause state
	if (!overlayActive && newOverlayActive) {
		pausedBeforeOverlay = RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint32) & 1;
		
		if (!pausedBeforeOverlay) pause_toggle();
	} else if (overlayActive && !newOverlayActive && !pausedBeforeOverlay) {
		pause_toggle();
	}
	
	overlayActive = newOverlayActive;
}

void renderer_refresh() {
	int width = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
	int height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
	
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	
	if (_renderer == NULL)
		_renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (_renderer == NULL) {
		log_warning("SDL_CreateRenderer failed: %s", SDL_GetError());
		log_warning("Falling back to software rendering...");
		gHardwareDisplay = false;
		video_refresh(); // try again without hardware rendering
		return;
	}
	
	if (_bufferTexture != NULL)
		SDL_DestroyTexture(_bufferTexture);
	
	if (_bufferTextureFormat != NULL)
		SDL_FreeFormat(_bufferTextureFormat);
	
	SDL_RendererInfo rendererinfo;
	SDL_GetRendererInfo(_renderer, &rendererinfo);
	Uint32 pixelformat = SDL_PIXELFORMAT_UNKNOWN;
	for(unsigned int i = 0; i < rendererinfo.num_texture_formats; i++){
		Uint32 format = rendererinfo.texture_formats[i];
		if(!SDL_ISPIXELFORMAT_FOURCC(format) && !SDL_ISPIXELFORMAT_INDEXED(format) && (pixelformat == SDL_PIXELFORMAT_UNKNOWN || SDL_BYTESPERPIXEL(format) < SDL_BYTESPERPIXEL(pixelformat))){
			pixelformat = format;
		}
	}
	
	_bufferTexture = SDL_CreateTexture(_renderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, width, height);
	Uint32 format;
	SDL_QueryTexture(_bufferTexture, &format, 0, 0, 0);
	_bufferTextureFormat = SDL_AllocFormat(format);
	video_refresh_screenbuffer(width, height, width);
	// Load the current palette into the HWmapped version.
	for (int i = 0; i < 256; ++i) {
		_paletteHWMapped[i] = SDL_MapRGB(_bufferTextureFormat, gPalette[i].r, gPalette[i].g, gPalette[i].b);
	}
}

void renderer_draw() {
	int width = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
	int height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
	
	void *pixels;
	int pitch;
	if (SDL_LockTexture(_bufferTexture, NULL, &pixels, &pitch) == 0) {
		uint8 *src = (uint8*)gScreenBuffer;
		int padding = pitch - (width * 4);
		if (pitch == width * 4) {
			uint32 *dst = pixels;
			for (int i = width * height; i > 0; i--) { *dst++ = *(uint32 *)(&_paletteHWMapped[*src++]); }
		}
		else if (pitch == (width * 2) + padding) {
			uint16 *dst = pixels;
			for (int y = height; y > 0; y--) {
				for (int x = width; x > 0; x--) { *dst++ = *(uint16 *)(&_paletteHWMapped[*src++]); }
				dst = (uint16*)(((uint8 *)dst) + padding);
			}
		}
		else if (pitch == width + padding) {
			uint8 *dst = pixels;
			for (int y = height; y > 0; y--) {
				for (int x = width; x > 0; x--) { *dst++ = *(uint8 *)(&_paletteHWMapped[*src++]); }
				dst += padding;
			}
		}
		SDL_UnlockTexture(_bufferTexture);
	}
	
	SDL_RenderCopy(_renderer, _bufferTexture, NULL, NULL);
	
	if (gSteamOverlayActive && gConfigGeneral.steam_overlay_pause) {
		overlay_pre_render_check(width, height);
	}
	
	SDL_RenderPresent(_renderer);
	
	if (gSteamOverlayActive && gConfigGeneral.steam_overlay_pause) {
		overlay_post_render_check(width, height);
	}
}

void renderer_close() {
	SDL_FreeFormat(_bufferTextureFormat);
	_bufferTextureFormat = NULL;
	SDL_DestroyTexture(_bufferTexture);
	_bufferTexture = NULL;
	SDL_DestroyRenderer(_renderer);
	_renderer = NULL;
}

void renderer_update_palette() {
	if (_bufferTextureFormat != NULL) {
		for (int i=0; i < 256; ++i) {
			_paletteHWMapped[i] = SDL_MapRGB(_bufferTextureFormat, gPalette[i].r, gPalette[i].g, gPalette[i].b);
		}
	}
}
