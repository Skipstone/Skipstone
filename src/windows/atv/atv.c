#include <pebble.h>
#include "atv.h"
#include "../../libs/pebble-assist.h"
#include "../../common.h"

#define NUM_TYPES 2

static void send_request(char *request);
static void update_display();
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_double_click_handler(ClickRecognizerRef recognizer, void *context);
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
static GBitmap *action_icon_back;
static GBitmap *action_icon_home;
static GBitmap *action_icon_settings;

static TextLayer *title_layer;

static BitmapLayer *logo_layer;
static GBitmap *logo;

static InverterLayer *inverter_layer;

static Player player;

static int controlling_type;

void atv_init(Player p) {
	player = p;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);
}

void atv_destroy(void) {
	persist_write_int(KEY_ATV_CONTROLLING_TYPE, controlling_type);
	window_destroy_safe(window);
}

void atv_in_received_handler(DictionaryIterator *iter) {

}

void atv_out_sent_handler(DictionaryIterator *sent) {

}

void atv_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void send_request(char *request) {
	Tuplet request_tuple = TupletCString(KEY_REQUEST, request);
	Tuplet index_tuple = TupletInteger(KEY_INDEX, player.index);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &request_tuple);
	dict_write_tuplet(iter, &index_tuple);
	dict_write_end(iter);

	app_message_outbox_send();
}

static void update_display() {
	inverter_layer_destroy(inverter_layer);
	switch (controlling_type) {
		case 0:
			inverter_layer = inverter_layer_create((GRect) { .origin = { 8, 40 }, .size = { PEBBLE_WIDTH - 36, 36 } });
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play_pause);
			break;
		case 1:
			inverter_layer = inverter_layer_create((GRect) { .origin = { 8, 88 }, .size = { PEBBLE_WIDTH - 36, 36 } });
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);
			break;
	}
	layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(inverter_layer));
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	atv_destroy();
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("rewind");
			break;
		case 1:
			send_request("arrow_up");
			break;
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("forward");
			break;
		case 1:
			send_request("arrow_down");
			break;
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("play_pause");
			break;
		case 1:
			send_request("select");
			break;
	}
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("skip_backward");
			break;
		case 1:
			send_request("arrow_left");
			break;
	}
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("skip_forward");
			break;
		case 1:
			send_request("arrow_right");
			break;
	}
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			send_request("stop");
			break;
		case 1:
			send_request("menu");
			break;
	}
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_type = (controlling_type + 1) % NUM_TYPES;
	update_display();
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
	controlling_type = persist_exists(KEY_ATV_CONTROLLING_TYPE) ? persist_read_int(KEY_ATV_CONTROLLING_TYPE) : 0;

	action_icon_play_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY_PAUSE);
	action_icon_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FORWARD);
	action_icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);
	action_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
	action_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
	action_icon_select = gbitmap_create_with_resource(RESOURCE_ID_ICON_SELECT);
	action_icon_back = gbitmap_create_with_resource(RESOURCE_ID_ICON_BACK);
	action_icon_home = gbitmap_create_with_resource(RESOURCE_ID_ICON_HOME);
	action_icon_settings = gbitmap_create_with_resource(RESOURCE_ID_ICON_SETTINGS);

	action_bar = action_bar_layer_create();
	action_bar_layer_add_to_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	logo = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WDTV);
	logo_layer = bitmap_layer_create((GRect) { .origin = { 0, 30 }, .size = { 122, 100 } });
	bitmap_layer_set_bitmap(logo_layer, logo);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(logo_layer));

	update_display();

	title_layer = text_layer_create((GRect) { .origin = { 8, 0 }, .size = { 120, 52 } });
	text_layer_set_text(title_layer, player.title);
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_layer));
}

static void window_unload(Window *window) {
	gbitmap_destroy(action_icon_play_pause);
	gbitmap_destroy(action_icon_forward);
	gbitmap_destroy(action_icon_rewind);
	gbitmap_destroy(action_icon_up);
	gbitmap_destroy(action_icon_down);
	gbitmap_destroy(action_icon_select);
	gbitmap_destroy(action_icon_back);
	gbitmap_destroy(action_icon_home);
	gbitmap_destroy(action_icon_settings);
	gbitmap_destroy(logo);
	bitmap_layer_destroy(logo_layer);
	action_bar_layer_destroy(action_bar);
	inverter_layer_destroy(inverter_layer);
	text_layer_destroy(title_layer);
}
