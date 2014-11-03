#include <pebble.h>
#include "win-plex.h"
#include "libs/pebble-assist.h"
#include "generated/keys.h"
#include "skipstone.h"
#include "players.h"
#include "plex.h"
#include "windows/win-plexstatus.h"

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;
static MenuLayer *menu_layer = NULL;

void win_plex_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_plex_push(void) {
	window_stack_push(window, true);
}

void win_plex_deinit(void) {
	window_destroy_safe(window);
}

void win_plex_reload_data_and_mark_dirty(void) {
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return plex_clients_count() ? plex_clients_count() : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (plex_get_error()) {
		return graphics_text_layout_get_content_size(plex_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 4, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft).h + 16;
	}
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, players_get_current()->title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	graphics_context_set_text_color(ctx, GColorBlack);
	if (plex_get_error()) {
		graphics_draw_text(ctx, plex_get_error(), fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(4, 4, 136, 128), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	} else {
		PlexClient *client = plex_client_get(cell_index->row);
		menu_cell_basic_draw(ctx, cell_layer, client->title, client->subtitle, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (!plex_clients_count()) return;
	plex_client_set_current(cell_index->row);
	win_plexstatus_push();
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	plex_request(KEY_REQUEST_CLIENTS);
}

static void window_load(Window *window) {
	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
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

	if (!plex_clients_count()) plex_request(KEY_REQUEST_CLIENTS);
}

static void window_unload(Window *window) {
	menu_layer_destroy_safe(menu_layer);
}
