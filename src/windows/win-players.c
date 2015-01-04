#include <pebble.h>
#include "win-players.h"
#include "libs/pebble-assist.h"
#include "skipstone.h"
#include "players.h"
#include "win-plex.h"
#include "win-vlc.h"
#include "win-xbmc.h"
#include "win-wdtv.h"

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static Window* window = NULL;
static MenuLayer* menu_layer = NULL;

void win_players_init(void) {
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

void win_players_deinit(void) {
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void win_players_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

void win_players_push_player(void) {
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = players_get_current_index(), .section = 0 }, MenuRowAlignTop, false);
	switch (players_get_current()->mediaplayer) {
		case MediaPlayerPLEX:
			win_plex_push();
			break;
		case MediaPlayerVLC:
			win_vlc_push();
			break;
		case MediaPlayerXBMC:
			win_xbmc_push();
			break;
		case MediaPlayerWDTV:
			win_wdtv_push();
			break;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return players_count() ? players_count() : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (players_get_error()) {
		return graphics_text_layout_get_content_size(players_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + 12;
	}
	return 30;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, "Media Players");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	if (players_get_error()) {
		graphics_draw_text(ctx, players_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		Player *player = players_get(cell_index->row);
		
		GSize mediaSize = graphics_text_layout_get_content_size(player_to_str(player->mediaplayer), fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(4, 2, 136, 22), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
		graphics_draw_text(ctx, player_to_str(player->mediaplayer), fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(144 - mediaSize.w - 4, 2, mediaSize.w, 22), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
		
		graphics_draw_text(ctx, player->title, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 2, 134 - mediaSize.w, 22), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (!players_count()) return;
	players_set_current(cell_index->row);
	win_players_push_player();
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	players_request();
}
