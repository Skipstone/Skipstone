#include <pebble.h>
#include "playerlist.h"
#include "../libs/pebble-assist.h"
#include "../common.h"
#include "plex/plex.h"
#include "vlc/vlc.h"
#include "xbmc/xbmc.h"
#include "wdtv/wdtv.h"

#define MAX_PLAYERS 20

static Player players[MAX_PLAYERS];

static int num_players;

const char* player_to_str(MediaPlayer player);
static void clear_cells();
static void refresh_list();
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
}

void playerlist_destroy(void) {
	plex_destroy();
	vlc_destroy();
	xbmc_destroy();
	wdtv_destroy();
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
		if(index_tuple->value->int16 == 0) clear_cells();
		Player player;
		player.index = index_tuple->value->int16;
		strncpy(player.title, title_tuple->value->cstring, sizeof(player.title) - 1);
		strncpy(player.subtitle, subtitle_tuple->value->cstring, sizeof(player.subtitle) - 1);
		player.player = player_tuple->value->int16;
		players[player.index] = player;
		num_players++;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

void playerlist_in_dropped_handler(AppMessageResult reason) {

}

void playerlist_out_sent_handler(DictionaryIterator *sent) {

}

void playerlist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

const char* player_to_str(MediaPlayer player) {
	switch (player) {
		case MediaPlayerPLEX:
			return "Plex";
		case MediaPlayerVLC:
			return "VLC";
		case MediaPlayerXBMC:
			return "XBMC";
		case MediaPlayerWDTV:
			return "WDTV";
		case MediaPlayerNONE:
		default:
			return "";
	}
}

static void clear_cells() {
	memset(players, 0x0, sizeof(players));
	num_players = 0;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

static void refresh_list() {
	clear_cells();
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
	return 48;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, "Media Players");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_players == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, players[cell_index->row].title, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, -4 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, players[cell_index->row].subtitle, fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 24 }, .size = { 100, 20 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
		graphics_draw_text(ctx, player_to_str(players[cell_index->row].player), fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 24 }, .size = { PEBBLE_WIDTH - 8, 20 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_players == 0) {
		return;
	}

	switch (players[cell_index->row].player) {
		case MediaPlayerPLEX:
			plex_init(players[cell_index->row]);
			break;
		case MediaPlayerVLC:
			vlc_init(players[cell_index->row]);
			break;
		case MediaPlayerXBMC:
			xbmc_init(players[cell_index->row]);
			break;
		case MediaPlayerWDTV:
			wdtv_init(players[cell_index->row]);
			break;
		case MediaPlayerNONE:
			break;
	}
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
