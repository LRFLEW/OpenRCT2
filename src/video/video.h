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

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include "../common.h"
#include <SDL.h>

void video_refresh();
void video_draw();
void video_init();
void video_close();

extern SDL_Color gPalette[256];
void video_update_palette(const uint8* colours, int start_index, int num_colours);
void video_refresh_screenbuffer(int width, int height, int pitch);

extern int gHardwareDisplay;

extern void *gScreenBuffer;
extern int gScreenBufferSize;
extern int gScreenBufferWidth;
extern int gScreenBufferHeight;
extern int gScreenBufferPitch;

#endif /* _VIDEO_H_ */
