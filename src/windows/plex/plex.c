#include <pebble.h>
#include "plex.h"
#include "../../libs/pebble-assist.h"
#include "../../common.h"
#include "plexstatus.h"

#define MAX_CLIENTS 20

static Player clients[MAX_CLIENTS];
static Player player;

static int num_clients = 0;
static int no_clients = false;

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

void plex_init(Player p) {
	player = p;

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

void plex_destroy(void) {
	plexstatus_destroy();
	layer_remove_from_parent(menu_layer_get_layer(menu_layer));
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

void plex_in_received_handler(DictionaryIterator *iter) {
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *subtitle_tuple = dict_find(iter, KEY_STATUS);

	if (index_tuple && title_tuple && subtitle_tuple) {
		if (index_tuple->value->int16 == 0) clear_cells();
		Player client;
		client.index = index_tuple->value->int16;
		strncpy(client.title, title_tuple->value->cstring, sizeof(client.title) - 1);
		strncpy(client.subtitle, subtitle_tuple->value->cstring, sizeof(client.subtitle) - 1);
		clients[client.index] = client;
	}
	else if (index_tuple) {
		num_clients = index_tuple->value->int16;
		no_clients = num_clients == 0;
		menu_layer_reload_data_and_mark_dirty(menu_layer);
	}
}

void plex_in_dropped_handler(AppMessageResult reason) {

}

void plex_out_sent_handler(DictionaryIterator *sent) {

}

void plex_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void clear_cells() {
	memset(clients, 0x0, sizeof(clients));
	num_clients = 0;
	no_clients = false;
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

static void refresh_list() {
	clear_cells();
	Tuplet request_tuple = TupletCString(KEY_REQUEST, "refresh");
	Tuplet index_tuple = TupletInteger(KEY_INDEX, player.index);
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	if (iter == NULL) return;
	dict_write_tuplet(iter, &request_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_end(iter);
	app_message_outbox_send();
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return num_clients ? num_clients : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_BASIC_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, player.title);
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (no_clients) {
		menu_cell_basic_draw(ctx, cell_layer, "No clients found.", NULL, NULL);
	} else if (num_clients == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		menu_cell_basic_draw(ctx, cell_layer, clients[cell_index->row].title, clients[cell_index->row].subtitle, NULL);
	}
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_clients == 0) {
		return;
	}
	plexstatus_init(player, clients[cell_index->row]);
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	refresh_list();
}
