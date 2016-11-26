#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "../config.h"
#include "../drawing/drawing.h"
#include "../game.h"
#include "../input.h"
#include "../interface/themes.h"
#include "../interface/title_sequences.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../peep/peep.h"
#include "../peep/staff.h"
#include "../scenario.h"
#include "../ScenarioSources.h"
#include "../sprites.h"
#include "../title/TitleScreen.h"
#include "../title/TitleSequence.h"
#include "../title/TitleSequenceManager.h"
#include "../util/util.h"
#include "../world/sprite.h"
#include "dropdown.h"
#include "error.h"

enum {
	WINDOW_TITLE_EDITOR_TAB_PRESETS,
	WINDOW_TITLE_EDITOR_TAB_SAVES,
	WINDOW_TITLE_EDITOR_TAB_SCRIPT,
	WINDOW_TITLE_EDITOR_TAB_COUNT
} WINDOW_TITLE_EDITOR_TAB;

static void window_title_editor_close(rct_window *w);
static void window_title_editor_mouseup(rct_window *w, int widgetIndex);
static void window_title_editor_resize(rct_window *w);
static void window_title_editor_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_title_editor_dropdown(rct_window *w, int widgetIndex, int dropdownIndex);
static void window_title_editor_update(rct_window *w);
static void window_title_editor_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height);
static void window_title_editor_scrollmousedown(rct_window *w, int scrollIndex, int x, int y);
static void window_title_editor_scrollmouseover(rct_window *w, int scrollIndex, int x, int y);
static void window_title_editor_textinput(rct_window *w, int widgetIndex, char *text);
static void window_title_editor_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId);
static void window_title_editor_invalidate(rct_window *w);
static void window_title_editor_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_title_editor_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex);
static void window_title_editor_scrollpaint_saves(rct_window *w, rct_drawpixelinfo *dpi);
static void window_title_editor_scrollpaint_commands(rct_window *w, rct_drawpixelinfo *dpi);
static void window_title_editor_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w);
static void window_title_editor_load_sequence();

static rct_window_event_list window_title_editor_events = {
	window_title_editor_close,
	window_title_editor_mouseup,
	window_title_editor_resize,
	window_title_editor_mousedown,
	window_title_editor_dropdown,
	NULL,
	window_title_editor_update,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_title_editor_scrollgetsize,
	window_title_editor_scrollmousedown,
	NULL,
	window_title_editor_scrollmouseover,
	window_title_editor_textinput,
	NULL,
	NULL,
	window_title_editor_tooltip,
	NULL,
	NULL,
	window_title_editor_invalidate,
	window_title_editor_paint,
	window_title_editor_scrollpaint,
};

enum WINDOW_TITLE_EDITOR_WIDGET_IDX {
	WIDX_TITLE_EDITOR_BACKGROUND,
	WIDX_TITLE_EDITOR_TITLE,
	WIDX_TITLE_EDITOR_CLOSE,
	WIDX_TITLE_EDITOR_TAB_CONTENT_PANEL,
	WIDX_TITLE_EDITOR_PRESETS_TAB,
	WIDX_TITLE_EDITOR_SAVES_TAB,
	WIDX_TITLE_EDITOR_SCRIPT_TAB,
	WIDX_TITLE_EDITOR_LIST,

	// Presets Tab
	WIDX_TITLE_EDITOR_PRESETS,
	WIDX_TITLE_EDITOR_PRESETS_DROPDOWN,
	WIDX_TITLE_EDITOR_NEW_BUTTON,
	WIDX_TITLE_EDITOR_DUPLICATE_BUTTON,
	WIDX_TITLE_EDITOR_DELETE_BUTTON,
	WIDX_TITLE_EDITOR_RENAME_BUTTON,

	// Saves Tab
	WIDX_TITLE_EDITOR_ADD,
	WIDX_TITLE_EDITOR_REMOVE,
	WIDX_TITLE_EDITOR_RENAME,
	WIDX_TITLE_EDITOR_LOAD,

	// Script Tab
	WIDX_TITLE_EDITOR_INSERT,
	WIDX_TITLE_EDITOR_EDIT,
	WIDX_TITLE_EDITOR_DELETE,
	//WIDX_TITLE_EDITOR_RELOAD,
	WIDX_TITLE_EDITOR_SKIP_TO,

	WIDX_TITLE_EDITOR_MOVE_UP,
	WIDX_TITLE_EDITOR_MOVE_DOWN,

	WIDX_TITLE_EDITOR_REPLAY,
	WIDX_TITLE_EDITOR_STOP,
	WIDX_TITLE_EDITOR_PLAY,
	WIDX_TITLE_EDITOR_SKIP,
};

// Increase BW if certain launguages do not fit
// BW should be a multiple of 4
#define WW 320
#define WH 270
#define BX 8
#define BW 72
#define BY 52
#define BH 63
#define BS 18
#define ROW_HEIGHT 11
#define SCROLL_WIDTH 350
#define WH2 127

static rct_widget window_title_editor_widgets[] = {
	{ WWT_FRAME,			0,	0,		WW-1,	0,		WH2-1,	0xFFFFFFFF,				STR_NONE },								// panel / background
	{ WWT_CAPTION,			0,	1,		WW-2,	1,		14,		STR_TITLE_EDITOR_TITLE,	STR_WINDOW_TITLE_TIP },					// title bar
	{ WWT_CLOSEBOX,			0,	WW-13,	WW-3,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP },					// close button
	{ WWT_RESIZE,			1,	0,		WW-1,	43,		WH2-1,	0xFFFFFFFF,				STR_NONE },								// tab content panel
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x20000000 | SPR_TAB,	STR_THEMES_TAB_SETTINGS_TIP },	// presets tab
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x20000000 | SPR_TAB,	STR_TITLE_EDITOR_SAVES_TAB_TIP },		// saves tab
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x20000000 | SPR_TAB,	STR_TITLE_EDITOR_SCRIPT_TAB_TIP },		// script tab
	{ WWT_SCROLL,			1,	BX+BW+9,WW-4,	48,		WH-4,	SCROLL_BOTH,			STR_NONE },								// command/save list

	// Presets Tab
	{ WWT_DROPDOWN,			1,	125,	299,	60,		71,		STR_NONE,							STR_NONE },						// Preset title sequences
	{ WWT_DROPDOWN_BUTTON,	1,	288,	298,	61,		70,		STR_DROPDOWN_GLYPH,					STR_NONE },
	{ WWT_DROPDOWN_BUTTON,	1,	10,		100,	82,		93,		STR_TITLE_EDITOR_ACTION_CREATE,		STR_TITLE_EDITOR_ACTION_CREATE_SEQUENCE_TIP },						// Create button
	{ WWT_DROPDOWN_BUTTON,	1,	10,		100,	82+20,	93+20,	STR_TITLE_EDITOR_ACTION_DUPLICATE,	STR_TITLE_EDITOR_ACTION_DUPLICATE_SEQUENCE_TIP },						// Duplicate button
	{ WWT_DROPDOWN_BUTTON,	1,	110,	200,	82,		93,		STR_TRACK_MANAGE_DELETE,			STR_TITLE_EDITOR_ACTION_DELETE_SEQUENCE_TIP },						// Delete button
	{ WWT_DROPDOWN_BUTTON,	1,	210,	300,	82,		93,		STR_TRACK_MANAGE_RENAME,			STR_TITLE_EDITOR_ACTION_RENAME_SEQUENCE_TIP },						// Rename button

	// Saves Tab
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY,			BH,			STR_TITLE_EDITOR_ACTION_ADD,		STR_TITLE_EDITOR_ACTION_ADD_TIP }, // Add
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*1),	BH+(BS*1),	STR_TITLE_EDITOR_ACTION_REMOVE,		STR_TITLE_EDITOR_ACTION_REMOVE_TIP }, // Remove
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*2),	BH+(BS*2),	STR_TRACK_MANAGE_RENAME,			STR_TITLE_EDITOR_ACTION_RENAME_TIP }, // Rename
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*3),	BH+(BS*3),	STR_TITLE_EDITOR_ACTION_LOAD,		STR_TITLE_EDITOR_ACTION_LOAD_TIP }, // Load

	// Script Tab
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY,			BH,			STR_TITLE_EDITOR_ACTION_INSERT,		STR_TITLE_EDITOR_ACTION_INSERT_TIP }, // Insert
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*1),	BH+(BS*1),	STR_TITLE_EDITOR_ACTION_EDIT,		STR_TITLE_EDITOR_ACTION_EDIT_TIP }, // Edit
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*2),	BH+(BS*2),	STR_TRACK_MANAGE_DELETE,			STR_TITLE_EDITOR_ACTION_DELETE_TIP }, // Delete
	//{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*3),	BH+(BS*3),	STR_TITLE_EDITOR_ACTION_RELOAD,		STR_TITLE_EDITOR_ACTION_RELOAD_TIP }, // Reload
	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW-1,BY+(BS*3),	BH+(BS*3),	STR_TITLE_EDITOR_ACTION_SKIP_TO,	STR_TITLE_EDITOR_ACTION_SKIP_TO_TIP }, // Skip to

	{ WWT_DROPDOWN_BUTTON,	1,	BX,		BX+BW/2-1,BY+(BS*5),BH+(BS*5),	STR_DOWN,	STR_TITLE_EDITOR_ACTION_MOVE_DOWN_TIP }, // Move down
	{ WWT_DROPDOWN_BUTTON,	1,	BX+BW/2,BX+BW-1,BY+(BS*5),	BH+(BS*5),	STR_UP,	STR_TITLE_EDITOR_ACTION_MOVE_UP_TIP }, // Move up

	{ WWT_IMGBTN,			1,	BX,		BX+BW/4-1,	WH-32,	WH-16,		SPR_G2_TITLE_RESTART,	STR_TITLE_EDITOR_ACTION_REPLAY_TIP }, // Replay
	{ WWT_IMGBTN,			1,	BX+BW/4,BX+BW/2-1,	WH-32,	WH-16,		SPR_G2_TITLE_STOP,		STR_TITLE_EDITOR_ACTION_STOP_TIP }, // Stop
	{ WWT_IMGBTN,			1,	BX+BW/2,BX+BW*3/4-1,WH-32,	WH-16,		SPR_G2_TITLE_PLAY,		STR_TITLE_EDITOR_ACTION_PLAY_TIP }, // Play
	{ WWT_IMGBTN,			1,	BX+BW*3/4,BX+BW,	WH-32,	WH-16,		SPR_G2_TITLE_SKIP,		STR_TITLE_EDITOR_ACTION_SKIP_TIP }, // Skip

	{ WIDGETS_END },
};

static sint16 _window_title_editor_highlighted_index;
static TitleSequence * _loadedTitleSequence;

int gTitleScriptCommand;
int gTitleScriptSkipLoad;
int gTitleScriptSkipTo;
int gTitleScriptSave;

static int window_title_editor_tab_animation_loops[] = {
	64,
	1,
	28
};
static int window_title_editor_tab_animation_divisor[] = {
	4,
	1,
	4
};
static int window_title_editor_tab_sprites[] = {
	SPR_TAB_RIDE_0,
	SPR_FLOPPY,
	SPR_TAB_STATS_0
};

void window_title_editor_open(int tab)
{
	rct_window* window;

	// Check if window is already open
	window = window_bring_to_front_by_class(WC_TITLE_EDITOR);
	if (window != NULL)
		return;

	window = window_create_auto_pos(WW, WH2, &window_title_editor_events, WC_TITLE_EDITOR, WF_10 | WF_RESIZABLE);
	window->widgets = window_title_editor_widgets;
	window->enabled_widgets =
		(1 << WIDX_TITLE_EDITOR_CLOSE) |
		(1 << WIDX_TITLE_EDITOR_PRESETS_TAB) |
		(1 << WIDX_TITLE_EDITOR_SAVES_TAB) |
		(1 << WIDX_TITLE_EDITOR_SCRIPT_TAB) |

		(1 << WIDX_TITLE_EDITOR_PRESETS) |
		(1 << WIDX_TITLE_EDITOR_PRESETS_DROPDOWN) |
		(1 << WIDX_TITLE_EDITOR_NEW_BUTTON) |
		(1 << WIDX_TITLE_EDITOR_DUPLICATE_BUTTON) |
		(1 << WIDX_TITLE_EDITOR_DELETE_BUTTON) |
		(1 << WIDX_TITLE_EDITOR_RENAME_BUTTON) |

		(1 << WIDX_TITLE_EDITOR_ADD) |
		(1 << WIDX_TITLE_EDITOR_REMOVE) |
		(1 << WIDX_TITLE_EDITOR_RENAME) |
		(1 << WIDX_TITLE_EDITOR_LOAD) |

		(1 << WIDX_TITLE_EDITOR_INSERT) |
		(1 << WIDX_TITLE_EDITOR_EDIT) |
		(1 << WIDX_TITLE_EDITOR_DELETE) |
		//(1 << WIDX_TITLE_EDITOR_RELOAD) |
		(1 << WIDX_TITLE_EDITOR_SKIP_TO) |
		(1 << WIDX_TITLE_EDITOR_MOVE_DOWN) |
		(1 << WIDX_TITLE_EDITOR_MOVE_UP) |

		(1 << WIDX_TITLE_EDITOR_PLAY) |
		(1 << WIDX_TITLE_EDITOR_STOP) |
		(1 << WIDX_TITLE_EDITOR_REPLAY) |
		(1 << WIDX_TITLE_EDITOR_SKIP);

	window_init_scroll_widgets(window);
	window->list_information_type = 0;

	window->selected_tab = tab;
	window->selected_list_item = -1;
	_window_title_editor_highlighted_index = -1;
	window->scrolls[0].v_top = 0;
	window->scrolls[0].h_left = 0;

	window->min_width = WW;
	window->min_height = WH;
	window->max_width = 500;
	window->max_height = 450;

	window_title_editor_load_sequence();
}

static void window_title_editor_close(rct_window *w)
{
	// Close the related windows
	window_close_by_class(WC_TITLE_COMMAND_EDITOR);
	if (gLoadSaveTitleSequenceSave) {
		window_close_by_class(WC_LOADSAVE);
	}
}

static void window_title_editor_mouseup(rct_window *w, int widgetIndex)
{
	bool readOnly = (gCurrentTitleSequence < TITLE_SEQUENCE_DEFAULT_PRESETS);
	bool playing = (gCurrentTitleSequence == gCurrentPreviewTitleSequence) && ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	bool inTitle = ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	bool commandEditorOpen = (window_find_by_class(WC_TITLE_COMMAND_EDITOR) != NULL);
	switch (widgetIndex) {
	case WIDX_TITLE_EDITOR_CLOSE:
		window_close(w);
		break;
	case WIDX_TITLE_EDITOR_NEW_BUTTON:
		if (!commandEditorOpen) {
			// TODO: This should probably be 'NEW'
			window_text_input_open(w, widgetIndex, STR_TITLE_EDITOR_ACTION_DUPLICATE, STR_TITLE_EDITOR_ENTER_NAME_FOR_SEQUENCE, STR_NONE, 0, 64);
		}
		break;
	case WIDX_TITLE_EDITOR_DUPLICATE_BUTTON:
		if (!commandEditorOpen) {
			window_text_input_open(w, widgetIndex, STR_TITLE_EDITOR_ACTION_DUPLICATE, STR_TITLE_EDITOR_ENTER_NAME_FOR_SEQUENCE, STR_STRING, (uintptr_t)_loadedTitleSequence->Name, 64);
		}
		break;
	case WIDX_TITLE_EDITOR_DELETE_BUTTON:
		if (!readOnly && !commandEditorOpen) {
			title_sequence_delete_preset(gCurrentTitleSequence);
		}
		break;
	case WIDX_TITLE_EDITOR_RENAME_BUTTON:
		if (!readOnly && !commandEditorOpen) {
			window_text_input_open(w, widgetIndex, STR_TRACK_MANAGE_RENAME, STR_TITLE_EDITOR_ENTER_NAME_FOR_SEQUENCE, STR_STRING, (uintptr_t)_loadedTitleSequence->Name, 64);
		}
		break;
	case WIDX_TITLE_EDITOR_ADD:
		if (!readOnly && !playing && !commandEditorOpen) {
			window_loadsave_open(LOADSAVETYPE_LOAD | LOADSAVETYPE_GAME, NULL);
			gLoadSaveTitleSequenceSave = true;
		}
		break;
	case WIDX_TITLE_EDITOR_REMOVE:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1) {
				title_sequence_remove_save(gCurrentTitleSequence, w->selected_list_item);
				if (w->selected_list_item > 0) {
					w->selected_list_item--;
				} else if (w->selected_list_item > (sint16)_loadedTitleSequence->NumSaves) {
					w->selected_list_item = (sint16)(_loadedTitleSequence->NumSaves - 1);
				}
			}
		}
		break;
	case WIDX_TITLE_EDITOR_RENAME:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1) {
				window_text_input_open(w, widgetIndex, STR_FILEBROWSER_RENAME_SAVE_TITLE, STR_TITLE_EDITOR_ENTER_NAME_FOR_SAVE, STR_STRING, (uintptr_t)_loadedTitleSequence->Saves[w->selected_list_item], 52 - 1);
			}
		}
		break;
	case WIDX_TITLE_EDITOR_LOAD:
		if (w->selected_list_item != -1) {
			utf8 path[MAX_PATH];
			if (str_is_null_or_empty(_loadedTitleSequence->Path)) {
				safe_strcpy(path, _loadedTitleSequence->Path, sizeof(path));
			} else {
				// TODO: This should probably use a constant
				platform_get_user_directory(path, "title sequences", sizeof(path));
				safe_strcat_path(path, _loadedTitleSequence->Name, sizeof(path));
			}

			safe_strcat_path(path, _loadedTitleSequence->Saves[w->selected_list_item], sizeof(path));
			game_load_save(path);
			window_title_editor_open(1);
		}
		break;
	case WIDX_TITLE_EDITOR_INSERT:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1) {
				window_title_command_editor_open(w->selected_list_item + 1, true);
			} else {
				window_title_command_editor_open((int)_loadedTitleSequence->NumCommands, true);
			}
		}
		break;
	case WIDX_TITLE_EDITOR_EDIT:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1 && w->selected_list_item < (sint16)_loadedTitleSequence->NumCommands) {
				window_title_command_editor_open(w->selected_list_item, false);
			}
		}
		break;
	case WIDX_TITLE_EDITOR_DELETE:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1 && w->selected_list_item < (sint16)_loadedTitleSequence->NumCommands) {
				title_sequence_delete_command(gCurrentTitleSequence, w->selected_list_item);
				if (w->selected_list_item > 0) {
					w->selected_list_item--;
				} else if (w->selected_list_item >= (sint16)_loadedTitleSequence->NumCommands) {
					w->selected_list_item = (sint16)(_loadedTitleSequence->NumCommands - 1);
				}
			}
		}
		break;
	/*case WIDX_TITLE_EDITOR_RELOAD:
		if (!playing && !commandEditorOpen) {
			//title_sequence_open
		}
		break;*/
	case WIDX_TITLE_EDITOR_SKIP_TO:
		if (playing && w->selected_list_item != -1 && w->selected_list_item < (sint16)_loadedTitleSequence->NumCommands) {
			if (gTitleScriptCommand > w->selected_list_item) {
				gTitleScriptCommand = 0;
			}
			if (gTitleScriptCommand != w->selected_list_item) {
				for (int i = gTitleScriptCommand + 1; i < (int)_loadedTitleSequence->NumCommands; i++) {
					if (_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_LOAD ||
						_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_LOADMM) {
						gTitleScriptSkipLoad = i;
					}
					if (i == w->selected_list_item) {
						gTitleScriptSkipTo = i;
						break;
					}
				}
			}
		}
		break;
	case WIDX_TITLE_EDITOR_MOVE_DOWN:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1 && w->selected_list_item < (sint16)_loadedTitleSequence->NumCommands - 1) {
				title_sequence_move_down_command(gCurrentTitleSequence, w->selected_list_item);
				w->selected_list_item++;
			}
		}
		break;
	case WIDX_TITLE_EDITOR_MOVE_UP:
		if (!readOnly && !playing && !commandEditorOpen) {
			if (w->selected_list_item != -1 && w->selected_list_item > 0) {
				title_sequence_move_up_command(gCurrentTitleSequence, w->selected_list_item);
				w->selected_list_item--;
			}
		}
		break;
	case WIDX_TITLE_EDITOR_REPLAY:
		break;
	case WIDX_TITLE_EDITOR_STOP:
		if (playing) {
			gCurrentPreviewTitleSequence = 0;
		}
		break;
	case WIDX_TITLE_EDITOR_PLAY:
		if (gCurrentTitleSequence != gCurrentPreviewTitleSequence && inTitle) {
			gCurrentPreviewTitleSequence = gCurrentTitleSequence;
		}
		break;
	case WIDX_TITLE_EDITOR_SKIP:
		if (playing) {
			int i;
			for (i = gTitleScriptCommand; i < (int)_loadedTitleSequence->NumCommands; i++) {
				if (_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_LOAD ||
					_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_LOADMM
				) {
					gTitleScriptSkipLoad = i;
					break;
				} else if (_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_WAIT ||
					_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_END) {
					gTitleScriptSkipTo = i;
					break;
				} else if (_loadedTitleSequence->Commands[i].Type == TITLE_SCRIPT_RESTART) {
					gTitleScriptSkipLoad = -1;
					gTitleScriptSkipTo = -1;
					break;
				}
			}
			if (i == _loadedTitleSequence->NumCommands && i - 1 > gTitleScriptCommand) {
				gTitleScriptSkipTo = i - 1;
				gTitleScriptSkipLoad = -1;
			}
		}
		break;
	}
	if (readOnly == 2) {
		window_error_open(STR_ERROR_CANT_CHANGE_TITLE_SEQUENCE, STR_NONE);
	} else if (commandEditorOpen == 2) {
		window_error_open(STR_TITLE_EDITOR_ERR_CANT_CHANGE_WHILE_EDITOR_IS_OPEN, STR_NONE);
	} else if (playing == 2) {
		window_error_open(STR_TITLE_EDITOR_ERR_CANT_EDIT_WHILE_PLAYING, STR_TITLE_EDITOR_PRESS_STOP_TO_CONTINUE_EDITING);
	}
}

static void window_title_editor_resize(rct_window *w)
{
	if (w->selected_tab == WINDOW_TITLE_EDITOR_TAB_PRESETS) {
		window_set_resize(w, WW, WH, WH2, WH2);
	} else {
		window_set_resize(w, WW, WH, 500, 580);
	}
}

static void window_title_editor_mousedown(int widgetIndex, rct_window* w, rct_widget* widget)
{
	switch (widgetIndex) {
	case WIDX_TITLE_EDITOR_PRESETS_TAB:
	case WIDX_TITLE_EDITOR_SAVES_TAB:
	case WIDX_TITLE_EDITOR_SCRIPT_TAB:
	{
		int newSelectedTab = widgetIndex - WIDX_TITLE_EDITOR_PRESETS_TAB;
		if (w->selected_tab != newSelectedTab) {
			w->selected_tab = newSelectedTab;
			w->selected_list_item = -1;
			_window_title_editor_highlighted_index = -1;
			w->scrolls[0].v_top = 0;
			w->frame_no = 0;
			window_event_resize_call(w);
			window_invalidate(w);
		}
		break;
	}
	case WIDX_TITLE_EDITOR_PRESETS_DROPDOWN:
		if (window_find_by_class(WC_TITLE_COMMAND_EDITOR) != NULL) {
			window_error_open(STR_TITLE_EDITOR_ERR_CANT_CHANGE_WHILE_EDITOR_IS_OPEN, STR_NONE);
		} else {
			int numItems = (int)title_sequence_manager_get_count();
			for (int i = 0; i < numItems; i++) {
				gDropdownItemsFormat[i] = STR_OPTIONS_DROPDOWN_ITEM;
				gDropdownItemsArgs[i] = (uintptr_t)title_sequence_manager_get_name(i);
			}

			widget--;
			window_dropdown_show_text_custom_width(
				w->x + widget->left,
				w->y + widget->top,
				widget->bottom - widget->top + 1,
				w->colours[1],
				DROPDOWN_FLAG_STAY_OPEN,
				numItems,
				widget->right - widget->left - 3);
			dropdown_set_checked(gCurrentTitleSequence, true);
		}
		break;
	}
}

static void window_title_editor_dropdown(rct_window *w, int widgetIndex, int dropdownIndex)
{
	if (dropdownIndex == -1)
		return;

	switch (widgetIndex) {
	case WIDX_TITLE_EDITOR_PRESETS_DROPDOWN:
		gCurrentTitleSequence = dropdownIndex;
		window_title_editor_load_sequence();
		window_invalidate(w);
		break;
	}
}

static void window_title_editor_update(rct_window *w)
{
	w->frame_no++;
	if (w->frame_no >= window_title_editor_tab_animation_loops[w->selected_tab]) {
		w->frame_no = 0;
	}

	if (!widget_is_highlighted(w, WIDX_TITLE_EDITOR_LIST)) {
		_window_title_editor_highlighted_index = -1;
		widget_invalidate(w, WIDX_TITLE_EDITOR_LIST);
	}

	widget_invalidate(w, WIDX_TITLE_EDITOR_PRESETS_TAB + w->selected_tab);
}

static void window_title_editor_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height)
{
	size_t lineCount = 1;
	if (w->selected_tab == WINDOW_TITLE_EDITOR_TAB_SAVES) {
		lineCount = _loadedTitleSequence->NumSaves;
	} else if (w->selected_tab == WINDOW_TITLE_EDITOR_TAB_SCRIPT) {
		lineCount = _loadedTitleSequence->NumCommands;
	}

	*height = (int)(lineCount * ROW_HEIGHT);
	int i = *height - window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].bottom + window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].top + 21;
	if (i < 0) {
		i = 0;
	}
	if (i < w->scrolls[0].v_top) {
		w->scrolls[0].v_top = i;
		window_invalidate(w);
	}

	*width = SCROLL_WIDTH;
}

static void window_title_editor_scrollmousedown(rct_window *w, int scrollIndex, int x, int y)
{
	int index = y / ROW_HEIGHT;
	w->selected_list_item = -1;
	switch (w->selected_tab) {
	case WINDOW_TITLE_EDITOR_TAB_SAVES:
		if (index < (int)_loadedTitleSequence->NumSaves) {
			w->selected_list_item = index;
			widget_invalidate(w, WIDX_TITLE_EDITOR_LIST);
		}
		break;
	case WINDOW_TITLE_EDITOR_TAB_SCRIPT:
		if (index < (int)_loadedTitleSequence->NumCommands) {
			w->selected_list_item = index;
			widget_invalidate(w, WIDX_TITLE_EDITOR_LIST);
		}
		break;
	}
}

static void window_title_editor_scrollmouseover(rct_window *w, int scrollIndex, int x, int y)
{
	int index = y / ROW_HEIGHT;
	switch (w->selected_tab) {
	case WINDOW_TITLE_EDITOR_TAB_SAVES:
		if (index < (int)_loadedTitleSequence->NumSaves)
			_window_title_editor_highlighted_index = (sint16)index;
		break;
	case WINDOW_TITLE_EDITOR_TAB_SCRIPT:
		if (index < (int)_loadedTitleSequence->NumCommands)
			_window_title_editor_highlighted_index = (sint16)index;
		break;
	}
	widget_invalidate(w, WIDX_TITLE_EDITOR_LIST);
}

static void window_title_editor_textinput(rct_window *w, int widgetIndex, char *text)
{
	if (str_is_null_or_empty(text)) {
		return;
	}

	switch (widgetIndex) {
	case WIDX_TITLE_EDITOR_NEW_BUTTON:
	case WIDX_TITLE_EDITOR_DUPLICATE_BUTTON:
	case WIDX_TITLE_EDITOR_RENAME_BUTTON:
		if (filename_valid_characters(text)) {
			if (!title_sequence_name_exists(text)) {
				if (widgetIndex == WIDX_TITLE_EDITOR_NEW_BUTTON) {
					title_sequence_create_preset(text);
				} else if (widgetIndex == WIDX_TITLE_EDITOR_DUPLICATE_BUTTON) {
					title_sequence_duplicate_preset(gCurrentTitleSequence, text);
				} else {
					title_sequence_rename_preset(gCurrentTitleSequence, text);
				}
				config_save_default();
				window_invalidate(w);
			} else {
				window_error_open(STR_ERROR_EXISTING_NAME, STR_NONE);
			}
		} else {
			window_error_open(STR_ERROR_INVALID_CHARACTERS, STR_NONE);
		}
		break;
	case WIDX_TITLE_EDITOR_RENAME:
		if (filename_valid_characters(text)) {
			if (!title_sequence_save_exists(gCurrentTitleSequence, text)) {
				title_sequence_rename_save(gCurrentTitleSequence, w->selected_list_item, text);
				TileSequenceSave(_loadedTitleSequence);
				window_invalidate(w);
			} else {
				window_error_open(STR_ERROR_EXISTING_NAME, STR_NONE);
			}
		} else {
			window_error_open(STR_ERROR_INVALID_CHARACTERS, STR_NONE);
		}
		break;
	}
}

static void window_title_editor_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId)
{
	set_format_arg(0, rct_string_id, STR_LIST);
}

static void window_title_editor_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	int pressed_widgets = w->pressed_widgets & 0xFFFFFF8F;
	uint8 widgetIndex = w->selected_tab + 4;

	w->pressed_widgets = pressed_widgets | (1 << widgetIndex);

	window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].type = WWT_EMPTY;

	window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS_DROPDOWN].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_NEW_BUTTON].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_DUPLICATE_BUTTON].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_DELETE_BUTTON].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_RENAME_BUTTON].type = WWT_EMPTY;

	window_title_editor_widgets[WIDX_TITLE_EDITOR_ADD].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_REMOVE].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_RENAME].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_LOAD].type = WWT_EMPTY;

	window_title_editor_widgets[WIDX_TITLE_EDITOR_INSERT].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_EDIT].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_DELETE].type = WWT_EMPTY;
	//window_title_editor_widgets[WIDX_TITLE_EDITOR_RELOAD].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP_TO].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_MOVE_UP].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_MOVE_DOWN].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_PLAY].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_STOP].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_REPLAY].type = WWT_EMPTY;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP].type = WWT_EMPTY;

	switch (w->selected_tab) {
	case WINDOW_TITLE_EDITOR_TAB_PRESETS:
		window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].type = WWT_DROPDOWN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS_DROPDOWN].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_NEW_BUTTON].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_DUPLICATE_BUTTON].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_DELETE_BUTTON].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_RENAME_BUTTON].type = WWT_DROPDOWN_BUTTON;
		break;
	case WINDOW_TITLE_EDITOR_TAB_SAVES:
		window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].type = WWT_SCROLL;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_ADD].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_REMOVE].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_RENAME].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_LOAD].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_PLAY].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_STOP].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_REPLAY].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP].type = WWT_IMGBTN;
		break;
	case WINDOW_TITLE_EDITOR_TAB_SCRIPT:
		window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].type = WWT_SCROLL;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_INSERT].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_EDIT].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_DELETE].type = WWT_DROPDOWN_BUTTON;
		//window_title_editor_widgets[WIDX_TITLE_EDITOR_RELOAD].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP_TO].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_MOVE_UP].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_MOVE_DOWN].type = WWT_DROPDOWN_BUTTON;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_PLAY].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_STOP].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_REPLAY].type = WWT_IMGBTN;
		window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP].type = WWT_IMGBTN;
		break;
	}

	window_title_editor_widgets[WIDX_TITLE_EDITOR_BACKGROUND].right = w->width - 1;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_BACKGROUND].bottom = w->height - 1;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_TAB_CONTENT_PANEL].right = w->width - 1;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_TAB_CONTENT_PANEL].bottom = w->height - 1;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_TITLE].right = w->width - 2;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_CLOSE].left = w->width - 2 - 0x0B;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_CLOSE].right = w->width - 2 - 0x0B + 0x0A;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].right = w->width - 4;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_LIST].bottom = w->height - 16;

	window_title_editor_widgets[WIDX_TITLE_EDITOR_REPLAY].top = w->height - 32;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_REPLAY].bottom = w->height - 16;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_STOP].top = w->height - 32;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_STOP].bottom = w->height - 16;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_PLAY].top = w->height - 32;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_PLAY].bottom = w->height - 16;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP].top = w->height - 32;
	window_title_editor_widgets[WIDX_TITLE_EDITOR_SKIP].bottom = w->height - 16;

	int playing = (gCurrentTitleSequence == gCurrentPreviewTitleSequence) && ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	int inTitle = ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	if (!inTitle) {
		w->disabled_widgets |= (1 << WIDX_TITLE_EDITOR_PLAY);
	} else {
		w->disabled_widgets &= ~(1 << WIDX_TITLE_EDITOR_PLAY);
	}
	if (!playing) {
		w->disabled_widgets |= (1 << WIDX_TITLE_EDITOR_REPLAY) | (1 << WIDX_TITLE_EDITOR_STOP) | (1 << WIDX_TITLE_EDITOR_SKIP) | (1 << WIDX_TITLE_EDITOR_SKIP_TO);
	} else {
		w->disabled_widgets &= ~((1 << WIDX_TITLE_EDITOR_REPLAY) | (1 << WIDX_TITLE_EDITOR_STOP) | (1 << WIDX_TITLE_EDITOR_SKIP) | (1 << WIDX_TITLE_EDITOR_SKIP_TO));
	}
}

static void window_title_editor_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	// Widgets
	window_draw_widgets(w, dpi);
	window_title_editor_draw_tab_images(dpi, w);

	// Draw strings
	switch (w->selected_tab) {
	case WINDOW_TITLE_EDITOR_TAB_PRESETS:
		set_format_arg(0, uintptr_t, _loadedTitleSequence->Name);
		gfx_draw_string_left(dpi, STR_TITLE_SEQUENCE, NULL, w->colours[1], w->x + 10, w->y + window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].top + 1);
		gfx_draw_string_left_clipped(
			dpi,
			STR_STRING,
			gCommonFormatArgs,
			w->colours[1],
			w->x + window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].left + 1,
			w->y + window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].top,
			w->x + window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS_DROPDOWN].left - window_title_editor_widgets[WIDX_TITLE_EDITOR_PRESETS].left - 4);
		break;
	case WINDOW_TITLE_EDITOR_TAB_SAVES:
		break;
	case WINDOW_TITLE_EDITOR_TAB_SCRIPT:
		break;
	}
}

static void window_title_editor_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex)
{
	gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ColourMapA[w->colours[1]].mid_light);
	switch (w->selected_tab) {
	case WINDOW_TITLE_EDITOR_TAB_SAVES:
		window_title_editor_scrollpaint_saves(w, dpi);
		break;
	case WINDOW_TITLE_EDITOR_TAB_SCRIPT:
		window_title_editor_scrollpaint_commands(w, dpi);
		break;
	}
}

static void window_title_editor_scrollpaint_saves(rct_window *w, rct_drawpixelinfo *dpi)
{
	int x = 0;
	int y = 0;
	bool inTitle = ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	for (int i = 0; i < (int)_loadedTitleSequence->NumSaves; i++, y += ROW_HEIGHT) {
		bool selected = false;
		bool hover = false;
		if (i == w->selected_list_item) {
			selected = true;
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].dark);
		} else if (i == _window_title_editor_highlighted_index || (i == gTitleScriptSave && inTitle && gCurrentTitleSequence == gCurrentPreviewTitleSequence)) {
			hover = true;
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].mid_dark);
		} else if (i & 1) {
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].lighter | 0x1000000);
		}

		char buffer[256];
		set_format_arg(0, uintptr_t, _loadedTitleSequence->Saves[i]);
		if (selected || hover) {
			format_string(buffer, 256, STR_STRING, gCommonFormatArgs);
		} else {
			format_string(buffer + 1, 255, STR_STRING, gCommonFormatArgs);
			buffer[0] = (utf8)FORMAT_BLACK;
		}
		set_format_arg(0, uintptr_t, &buffer);
		gfx_draw_string_left(dpi, STR_STRING, gCommonFormatArgs, w->colours[1], x + 5, y);
	}
}

static void window_title_editor_scrollpaint_commands(rct_window *w, rct_drawpixelinfo *dpi)
{
	int x = 0;
	int y = 0;
	bool inTitle = ((gScreenFlags & SCREEN_FLAGS_TITLE_DEMO) == SCREEN_FLAGS_TITLE_DEMO);
	for (int i = 0; i < (int)_loadedTitleSequence->NumCommands; i++, y += ROW_HEIGHT) {
		TitleCommand * command = &_loadedTitleSequence->Commands[i];
		bool selected = false;
		bool hover = false;
		bool error = false;
		if (i == w->selected_list_item) {
			selected = true;
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].dark);
		} else if (i == (int)_window_title_editor_highlighted_index || (i == gTitleScriptCommand && inTitle && gCurrentTitleSequence == gCurrentPreviewTitleSequence)) {
			hover = true;
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].mid_dark);
		} else if (i & 1) {
			gfx_fill_rect(dpi, x, y, x + SCROLL_WIDTH + 100, y + ROW_HEIGHT - 1, ColourMapA[w->colours[1]].lighter | 0x1000000);
		}

		rct_string_id commandName = STR_NONE;
		switch (command->Type) {
		case TITLE_SCRIPT_LOAD:
			commandName = STR_TITLE_EDITOR_COMMAND_LOAD_FILE;
			if (command->SaveIndex == 0xFF) {
				commandName = STR_TITLE_EDITOR_COMMAND_LOAD_NO_SAVE;
				error = true;
			}
			set_format_arg(0, uintptr_t, _loadedTitleSequence->Saves[command->SaveIndex]);
			break;
		case TITLE_SCRIPT_LOADMM:
			commandName = STR_TITLE_EDITOR_COMMAND_LOAD_SFMM;
			break;
		case TITLE_SCRIPT_LOCATION:
			commandName = STR_TITLE_EDITOR_COMMAND_LOCATION;
			set_format_arg(0, uint16, command->X);
			set_format_arg(2, uint16, command->Y);
			break;
		case TITLE_SCRIPT_ROTATE:
			commandName = STR_TITLE_EDITOR_COMMAND_ROTATE;
			set_format_arg(0, uint16, command->Rotations);
			break;
		case TITLE_SCRIPT_ZOOM:
			commandName = STR_TITLE_EDITOR_COMMAND_ZOOM;
			set_format_arg(0, uint16, command->Zoom);
			break;
		case TITLE_SCRIPT_SPEED:
			commandName = STR_TITLE_EDITOR_COMMAND_SPEED;
			set_format_arg(0, rct_string_id, SpeedNames[command->Speed - 1]);
			break;
		case TITLE_SCRIPT_WAIT:
			commandName = STR_TITLE_EDITOR_COMMAND_WAIT;
			set_format_arg(0, uint16, command->Seconds);
			break;
		case TITLE_SCRIPT_RESTART:
			commandName = STR_TITLE_EDITOR_RESTART;
			// TODO: Why the format arg?
			set_format_arg(0, uint16, command->Zoom);
			break;
		case TITLE_SCRIPT_END:
			commandName = STR_TITLE_EDITOR_END;
			break;
		case TITLE_SCRIPT_LOADRCT1:
			commandName = STR_TITLE_EDITOR_COMMAND_LOAD_FILE;
			const char * name = "";
			source_desc desc;
			if (scenario_get_source_desc_by_id(command->SaveIndex, &desc)) {
				name = desc.title;
			}
			set_format_arg(0, uintptr_t, name);
			break;
		default:
			log_warning("Unknown command %d", command->Type);
		}

		char buffer[256];
		if ((selected || hover) && !error) {
			format_string(buffer, 256, commandName, gCommonFormatArgs);
		} else {
			format_string(buffer + 1, 255, commandName, gCommonFormatArgs);
			buffer[0] = (utf8)(error ? ((selected || hover) ? FORMAT_LIGHTPINK : FORMAT_RED) : FORMAT_BLACK);
		}
		set_format_arg(0, uintptr_t, &buffer);
		gfx_draw_string_left(dpi, STR_STRING, gCommonFormatArgs, w->colours[1], x + 5, y);
	}
}

static void window_title_editor_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w)
{
	for (int i = 0; i < WINDOW_TITLE_EDITOR_TAB_COUNT; i++) {
		int x = 0;
		int y = 0;
		int spriteId = window_title_editor_tab_sprites[i];
		if (w->selected_tab == i) {
			spriteId += w->frame_no / window_title_editor_tab_animation_divisor[w->selected_tab];
		}
		if (i == 1) {
			x = 4;
			y = 1;
		}
		gfx_draw_sprite(dpi, spriteId, w->x + w->widgets[WIDX_TITLE_EDITOR_PRESETS_TAB + i].left + x, w->y + w->widgets[WIDX_TITLE_EDITOR_PRESETS_TAB + i].top + y, 0);
	}
}

static void window_title_editor_load_sequence()
{
	const char * path = title_sequence_manager_get_path(gCurrentTitleSequence);
	_loadedTitleSequence = LoadTitleSequence(path);
}
