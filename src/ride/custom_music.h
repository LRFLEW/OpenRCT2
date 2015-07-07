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

#ifndef _CUSTOM_MUSIC_H_
#define _CUSTOM_MUSIC_H_

#include "../common.h"

#define MAX_CUSTOM_MUSIC 255

typedef struct {
	char path[255];
} custom_music;

custom_music g_custom_music[MAX_CUSTOM_MUSIC];

void custom_music_reset_all();
uint8 custom_music_add(char *path);
void custom_music_remove(uint8 song_id);
void custom_music_set_ride(uint8 ride_id, uint8 song_id);

#endif