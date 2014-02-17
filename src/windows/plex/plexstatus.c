#include <pebble.h>
#include "plexstatus.h"
#include "../../libs/pebble-assist.h"
#include "../../common.h"

static void display_action_bar_icons();
static void send_request(char *request);
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_play_pause;
static GBitmap *action_icon_forward;
static GBitmap *action_icon_rewind;
static GBitmap *action_icon_up;
static GBitmap *action_icon_down;
static GBitmap *action_icon_select;

static BitmapLayer *logo_layer;
static GBitmap *logo;

static TextLayer *title_layer;

static Player player;
static Player client;

static bool controlling_keypad;

void plexstatus_init(Player p, Player c) {
	player = p;
	client = c;

	controlling_keypad = persist_exists(KEY_PLEX_CONTROLLING_KEYPAD) ? persist_read_bool(KEY_PLEX_CONTROLLING_KEYPAD) : true;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);

	send_request("refresh");
}

void plexstatus_destroy(void) {
	window_destroy_safe(window);
}

void plexstatus_in_received_handler(DictionaryIterator *iter) {

}

void plexstatus_out_sent_handler(DictionaryIterator *sent) {

}

void plexstatus_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void display_action_bar_icons() {
	action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, controlling_keypad ? action_icon_up : action_icon_forward);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, controlling_keypad ? action_icon_down : action_icon_rewind);
	action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, controlling_keypad ? action_icon_select : action_icon_play_pause);
}

static void send_request(char *request) {
	Tuplet request_tuple = TupletCString(KEY_REQUEST, request);
	Tuplet index_tuple = TupletInteger(KEY_INDEX, player.index);
	Tuplet client_tuple = TupletInteger(KEY_CLIENT, client.index);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &request_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_tuplet(iter, &client_tuple);
	dict_write_end(iter);

	app_message_outbox_send();
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	plexstatus_destroy();
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_keypad ? "up" : "forward_short");
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_keypad ? "down" : "backword_short");
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_keypad ? "select" : "play_pause");
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_keypad ? "right" : "forward_long");
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request(controlling_keypad ? "left" : "backword_long");
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("back");
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_keypad = !controlling_keypad;
	display_action_bar_icons();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 1000, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 1000, down_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
	window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, false, select_double_click_handler);
}

static void window_load(Window *window) {
	action_icon_play_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY_PAUSE);
	action_icon_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FORWARD);
	action_icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);
	action_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
	action_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
	action_icon_select = gbitmap_create_with_resource(RESOURCE_ID_ICON_SELECT);

	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	display_action_bar_icons();

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	title_layer = text_layer_create((GRect) { .origin = { 4, 0 }, .size = { bounds.size.w - 28, 28 } });
	text_layer_set_text(title_layer, client.title);
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	logo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLEX_LOGO);
	logo_layer = bitmap_layer_create((GRect) { .origin = { 24, 44 }, .size = { 80, 80 } });
	bitmap_layer_set_bitmap(logo_layer, logo);

	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
}

static void window_unload(Window *window) {
	gbitmap_destroy(action_icon_play_pause);
	gbitmap_destroy(action_icon_forward);
	gbitmap_destroy(action_icon_rewind);
	gbitmap_destroy(action_icon_up);
	gbitmap_destroy(action_icon_down);
	gbitmap_destroy(action_icon_select);
	gbitmap_destroy(logo);
	bitmap_layer_destroy(logo_layer);
	action_bar_layer_destroy(action_bar);
	text_layer_destroy(title_layer);
}
