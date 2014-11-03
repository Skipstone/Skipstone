#include <pebble.h>
#include "win-wdtv.h"
#include "libs/pebble-assist.h"
#include "generated/keys.h"
#include "progress_bar.h"
#include "skipstone.h"
#include "players.h"
#include "wdtv.h"

static void update_display();
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;

static ActionBarLayer *action_bar = NULL;

static InverterLayer *inverter_layer = NULL;

static BitmapLayer *logo_bitmap_layer = NULL;
static GBitmap *logo_bitmap = NULL;

static uint8_t controlling_type = 0;

void win_wdtv_init(void) {
	logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WDTV);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_wdtv_push(void) {
	window_stack_push(window, true);
}

void win_wdtv_deinit(void) {
	persist_write_bool(KEY_PERSIST_WDTV_CONTROLLING_TYPE, controlling_type);
	gbitmap_destroy_safe(logo_bitmap);
	window_destroy_safe(window);
}

void win_wdtv_reload_data_and_mark_dirty(void) {
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void update_display() {
	inverter_layer_destroy_safe(inverter_layer);
	switch (controlling_type) {
		case 0:
			inverter_layer = inverter_layer_create(GRect(8, 10, 108, 36));
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play_pause);
			break;
		case 1:
			inverter_layer = inverter_layer_create(GRect(8, 58, 108, 36));
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);
			break;
		case 2:
			inverter_layer = inverter_layer_create(GRect(8, 105, 108, 36));
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_back);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_settings);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_home);
			break;
	}
	inverter_layer_add_to_window(inverter_layer, window);
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_REWIND);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_UP);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_BACK);
			break;
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_FORWARD);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_DOWN);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_SETUP);
			break;
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_PLAY_PAUSE);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_OK);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_HOME);
			break;
	}
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_PREV);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_LEFT);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_MUTE);
			break;
	}
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_NEXT);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_RIGHT);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_POWER);
			break;
	}
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case 0:
			wdtv_request(KEY_REQUEST_STOP);
			break;
		case 1:
			wdtv_request(KEY_REQUEST_BACK);
			break;
		case 2:
			wdtv_request(KEY_REQUEST_OPTION);
			break;
	}
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_type = (controlling_type + 1) % 3;
	update_display();
	skipstone_short_vibe();
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
	controlling_type = persist_read_int(KEY_PERSIST_WDTV_CONTROLLING_TYPE);

	action_bar_layer_create_in_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

	logo_bitmap_layer = bitmap_layer_create(GRect(0, 0, 122, 152));
	bitmap_layer_set_bitmap(logo_bitmap_layer, logo_bitmap);
	bitmap_layer_add_to_window(logo_bitmap_layer, window);

	update_display();
}

static void window_unload(Window *window) {
	action_bar_layer_destroy_safe(action_bar);
	inverter_layer_destroy_safe(inverter_layer);
	bitmap_layer_destroy_safe(logo_bitmap_layer);
}
