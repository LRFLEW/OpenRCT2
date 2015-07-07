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

#include "custom_music.h"
#include "ride.h"

void custom_music_reset_all() {
	int i;
	for (int i = 0; i < MAX_CUSTOM_MUSIC; ++i) {
		g_custom_music[i].path[0] = '\0';
	}
}

uint8 custom_music_add(char *path) {
	uint8 i;
	for (i = 0; i < MAX_CUSTOM_MUSIC; ++i) {
		if (g_custom_music[i].path[0] == '\0') {
			strncpy(g_custom_music[i].path, path, MAX_PATH);
			return i;
		}
	}
	return 255;
}

void custom_music_remove(uint8 song_id) {
	g_custom_music[song_id].path[0] = '\0';
}

void custom_music_set_ride(uint8 ride_id, uint8 song_id) {
	rct_ride *ride = GET_RIDE(ride_id);
	if (ride->music != MUSIC_STYLE_OPENRCT_CUSTOM || ride->music_tune_id != song_id) {
		ride->music = MUSIC_STYLE_OPENRCT_CUSTOM;
		ride->music_tune_id = song_id;
		ride->music_position = 0;
	}
}
