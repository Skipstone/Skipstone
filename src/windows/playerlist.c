#include <pebble.h>
#include "playerlist.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "plex.h"
#include "vlc.h"
#include "xbmc.h"

#define MAX_PLAYERS 20

static Player players[MAX_PLAYERS];

static int num_players;

static void refresh_list();
static void request_data();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void playerlist_init(void) {
	window = window_create();

	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback,
		.select_click = menu_select_callback,
		.select_long_click = menu_select_long_callback,
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);

	window_stack_push(window, true);

	refresh_list();
}

void playerlist_destroy(void) {
//	plex_destroy();
	vlc_destroy();
//	xbmc_destroy();
	layer_remove_from_parent(menu_layer_get_layer(menu_layer));
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void playerlist_in_received_handler(DictionaryIterator *iter) {
	Tuple *player_tuple = dict_find(iter, KEY_PLAYER);
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *subtitle_tuple = dict_find(iter, KEY_STATUS);

	if (player_tuple && index_tuple && title_tuple && subtitle_tuple) {
		Player player;
		player.index = index_tuple->value->int16;
		strncpy(player.title, title_tuple->value->cstring, sizeof(player.title));
		strncpy(player.subtitle, subtitle_tuple->value->cstring, sizeof(player.subtitle));
		player.player = player_tuple->value->int16;
		players[player.index] = player;
		num_players++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "received player [%d] %s - %s", player.index, player.title, player.subtitle);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void refresh_list() {
	memset(players, 0x0, sizeof(players));
	num_players = 0;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
//	request_data();
	// add test data until JS is updated
	players[num_players++] = (Player) { .index = 0, .title = "MacBook Pro", .subtitle = "VLC - 10.0.1.4", .player = MediaPlayerVLC };
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

static void request_data() {
	Tuplet request_tuple = TupletInteger(KEY_REQUEST, 1);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &request_tuple);
	dict_write_end(iter);

	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_players) ? num_players : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, "Media Players");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_players == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, players[cell_index->row].title, players[cell_index->row].subtitle, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_players == 0) {
		return;
	}

	switch (players[cell_index->row].player) {
		case MediaPlayerPLEX:
//			plex_init(cell_index->row);
			break;
		case MediaPlayerVLC:
			vlc_init(players[cell_index->row]);
			break;
		case MediaPlayerXBMC:
//			xbmc_init(cell_index->row);
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
