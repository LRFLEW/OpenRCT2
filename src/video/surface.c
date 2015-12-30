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

#include "surface.h"
#include "../addresses.h"
#include "../common.h"
#include "../config.h"
#include "video.h"

static SDL_Surface *_surface = NULL;
static SDL_Surface *_RGBASurface = NULL;
static SDL_Palette *_palette = NULL;

void surface_refresh() {
	int width = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
	int height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
	
	surface_close();
	
	_surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
	_RGBASurface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_SetSurfaceBlendMode(_RGBASurface, SDL_BLENDMODE_NONE);
	_palette = SDL_AllocPalette(256);
	
	if (!_surface || !_palette || !_RGBASurface) {
		log_fatal("%p || %p || %p == NULL %s", _surface, _palette, _RGBASurface, SDL_GetError());
		exit(-1);
	}
	
	if (SDL_SetSurfacePalette(_surface, _palette)) {
		log_fatal("SDL_SetSurfacePalette failed %s", SDL_GetError());
		exit(-1);
	}
	
	video_refresh_screenbuffer(width, height, _surface->pitch);
}

void surface_draw() {
	// Lock the surface before setting its pixels
	if (SDL_MUSTLOCK(_surface)) {
		if (SDL_LockSurface(_surface) < 0) {
			log_error("locking failed %s", SDL_GetError());
			return;
		}
	}
	
	// Copy pixels from the virtual screen buffer to the surface
	memcpy(_surface->pixels, gScreenBuffer, _surface->pitch * _surface->h);
	
	// Unlock the surface
	if (SDL_MUSTLOCK(_surface))
		SDL_UnlockSurface(_surface);
	
	// Copy the surface to the window
	if (gConfigGeneral.window_scale == 1 || gConfigGeneral.window_scale <= 0)
	{
		if (SDL_BlitSurface(_surface, NULL, SDL_GetWindowSurface(gWindow), NULL)) {
			log_fatal("SDL_BlitSurface %s", SDL_GetError());
			exit(1);
		}
	} else {
		// first blit to rgba surface to change the pixel format
		if (SDL_BlitSurface(_surface, NULL, _RGBASurface, NULL)) {
			log_fatal("SDL_BlitSurface %s", SDL_GetError());
			exit(1);
		}
		// then scale to window size. Without changing to RGBA first, SDL complains
		// about blit configurations being incompatible.
		if (SDL_BlitScaled(_RGBASurface, NULL, SDL_GetWindowSurface(gWindow), NULL)) {
			log_fatal("SDL_BlitScaled %s", SDL_GetError());
			exit(1);
		}
	}
	if (SDL_UpdateWindowSurface(gWindow)) {
		log_fatal("SDL_UpdateWindowSurface %s", SDL_GetError());
		exit(1);
	}
}

void surface_close() {
	if (_surface != NULL) {
		SDL_FreeSurface(_surface);
		_surface = NULL;
	}
	if (_RGBASurface != NULL) {
		SDL_FreeSurface(_RGBASurface);
		_RGBASurface = NULL;
	}
	if (_palette != NULL) {
		SDL_FreePalette(_palette);
		_palette = NULL;
	}
}

void surface_update_palette() {
	if (_palette != NULL && SDL_SetPaletteColors(_palette, gPalette, 0, 256)) {
		log_fatal("SDL_SetPaletteColors failed %s", SDL_GetError());
		exit(1);
	}
}
