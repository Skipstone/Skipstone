#include <pebble.h>
#include "xbmc.h"
#include "../../libs/pebble-assist.h"
#include "../../common.h"
#include "../../progress_bar.h"

static void send_request(char *request);
static void display_action_bar_icons();
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

static AppTimer *update_timer;
static ActionBarLayer *action_bar;

static GBitmap *action_icon_volume_up;
static GBitmap *action_icon_volume_down;
static GBitmap *action_icon_play;
static GBitmap *action_icon_pause;
static GBitmap *action_icon_forward;
static GBitmap *action_icon_rewind;
static GBitmap *action_icon_up;
static GBitmap *action_icon_down;
static GBitmap *action_icon_select;

static TextLayer *title_layer;
static TextLayer *status_text_layer;
static TextLayer *status_layer;
static TextLayer *volume_text_layer;
static TextLayer *volume_layer;

static ProgressBarLayer *volume_bar;
static ProgressBarLayer *seek_bar;

static Player player;

typedef enum {
	CONTROLLING_TYPE_KEYPAD,
	CONTROLLING_TYPE_SEEK,
	CONTROLLING_TYPE_VOLUME
} CONTROLLING_TYPE;

static CONTROLLING_TYPE controlling_type;
static bool playing;

void xbmc_init(Player p) {
	player = p;

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_stack_push(window, true);

	send_request("refresh");
}

void xbmc_destroy(void) {
	persist_write_int(KEY_XBMC_CONTROLLING_TYPE, controlling_type);
	window_destroy_safe(window);
}

void xbmc_in_received_handler(DictionaryIterator *iter) {
	Tuple *title_tuple = dict_find(iter, KEY_TITLE);
	Tuple *volume_tuple = dict_find(iter, KEY_VOLUME);
	Tuple *status_tuple = dict_find(iter, KEY_STATUS);
	Tuple *seek_tuple = dict_find(iter, KEY_SEEK);

	if (title_tuple) {
		text_layer_set_text(title_layer, title_tuple->value->cstring);
	}
	if (volume_tuple) {
		static char vol[5];
		snprintf(vol, sizeof(vol), "%d%%", volume_tuple->value->int16);
		text_layer_set_text(volume_layer, vol);
		progress_bar_layer_set_value(volume_bar, volume_tuple->value->int16);
	}
	if (status_tuple) {
		text_layer_set_text(status_layer, status_tuple->value->cstring);
		playing = (strcmp(status_tuple->value->cstring, "Playing") == 0);
		if (controlling_type != CONTROLLING_TYPE_KEYPAD) {
			display_action_bar_icons();
		}
	}
	if (seek_tuple) {
		progress_bar_layer_set_value(seek_bar, seek_tuple->value->int16);
	}
}

void xbmc_out_sent_handler(DictionaryIterator *sent) {

}

void xbmc_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {

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

static void display_action_bar_icons() {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);
			break;
		case CONTROLLING_TYPE_SEEK:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (playing) ? action_icon_pause : action_icon_play);
			break;
		case CONTROLLING_TYPE_VOLUME:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_volume_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_volume_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (playing) ? action_icon_pause : action_icon_play);
			break;
	}
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	xbmc_destroy();
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			send_request("input_up");
			break;
		case CONTROLLING_TYPE_SEEK:
			send_request("backword_short");
			break;
		case CONTROLLING_TYPE_VOLUME:
			send_request("volume_increment");
			break;
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			send_request("input_down");
			break;
		case CONTROLLING_TYPE_SEEK:
			send_request("forward_short");
			break;
		case CONTROLLING_TYPE_VOLUME:
			send_request("volume_decrement");
			break;
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			send_request("input_select");
			break;
		case CONTROLLING_TYPE_SEEK:
		case CONTROLLING_TYPE_VOLUME:
			send_request("play_pause");
			break;
	}
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			send_request("input_left");
			break;
		case CONTROLLING_TYPE_SEEK:
			send_request("backword_long");
			break;
		case CONTROLLING_TYPE_VOLUME:
			send_request("volume_max");
			break;
	}
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			send_request("input_right");
			break;
		case CONTROLLING_TYPE_SEEK:
			send_request("forward_long");
			break;
		case CONTROLLING_TYPE_VOLUME:
			send_request("volume_min");
			break;
	}
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
	send_request("input_back");
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_type = controlling_type == CONTROLLING_TYPE_VOLUME ? CONTROLLING_TYPE_KEYPAD : controlling_type + 1;
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

static void handle_update_timer(void *data) {
	send_request("update");
	update_timer = app_timer_register(5000, handle_update_timer, NULL);
}

static void window_load(Window *window) {
	controlling_type = persist_exists(KEY_XBMC_CONTROLLING_TYPE) ? persist_read_int(KEY_XBMC_CONTROLLING_TYPE) : CONTROLLING_TYPE_KEYPAD;

	action_icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
	action_icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
	action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
	action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
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

	title_layer = text_layer_create((GRect) { .origin = { 4, 0 }, .size = { bounds.size.w - 28, 52 } });
	text_layer_set_text(title_layer, player.title);
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	status_text_layer = text_layer_create((GRect) { .origin = { 4, 54 }, .size = bounds.size });
	text_layer_set_text(status_text_layer, "Status:");
	text_layer_set_text_color(status_text_layer, GColorBlack);
	text_layer_set_background_color(status_text_layer, GColorClear);
	text_layer_set_font(status_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	status_layer = text_layer_create((GRect) { .origin = { 4, 54 + 15 }, .size = bounds.size });
	text_layer_set_text(status_layer, "Loading...");
	text_layer_set_text_color(status_layer, GColorBlack);
	text_layer_set_background_color(status_layer, GColorClear);
	text_layer_set_font(status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	volume_text_layer = text_layer_create((GRect) { .origin = { 4, 108 }, .size = bounds.size });
	text_layer_set_text(volume_text_layer, "Volume:");
	text_layer_set_text_color(volume_text_layer, GColorBlack);
	text_layer_set_background_color(volume_text_layer, GColorClear);
	text_layer_set_font(volume_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

	volume_layer = text_layer_create((GRect) { .origin = { 58, 102 }, .size = bounds.size });
	text_layer_set_text(volume_layer, "0%");
	text_layer_set_text_color(volume_layer, GColorBlack);
	text_layer_set_background_color(volume_layer, GColorClear);
	text_layer_set_font(volume_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	volume_bar = progress_bar_layer_create((GRect) { .origin = { 110, 58 }, .size = { 8, 76 } });
	progress_bar_layer_set_orientation(volume_bar, ProgressBarOrientationVertical);
	progress_bar_layer_set_range(volume_bar, 0, 100);
	progress_bar_layer_set_frame_color(volume_bar, GColorBlack);
	progress_bar_layer_set_bar_color(volume_bar, GColorBlack);

	seek_bar = progress_bar_layer_create((GRect) { .origin = { 4, 138 }, .size = { 115, 8 } });
	progress_bar_layer_set_orientation(seek_bar, ProgressBarOrientationHorizontal);
	progress_bar_layer_set_range(seek_bar, 0, 100);
	progress_bar_layer_set_frame_color(seek_bar, GColorBlack);
	progress_bar_layer_set_bar_color(seek_bar, GColorBlack);

	layer_add_child(window_layer, text_layer_get_layer(title_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(status_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(volume_layer));
	layer_add_child(window_layer, volume_bar);
	layer_add_child(window_layer, seek_bar);

	update_timer = app_timer_register(5000, handle_update_timer, NULL);
}

static void window_unload(Window *window) {
	gbitmap_destroy(action_icon_volume_up);
	gbitmap_destroy(action_icon_volume_down);
	gbitmap_destroy(action_icon_play);
	gbitmap_destroy(action_icon_pause);
	gbitmap_destroy(action_icon_forward);
	gbitmap_destroy(action_icon_rewind);
	gbitmap_destroy(action_icon_up);
	gbitmap_destroy(action_icon_down);
	gbitmap_destroy(action_icon_select);
	action_bar_layer_destroy(action_bar);
	text_layer_destroy(title_layer);
	text_layer_destroy(status_text_layer);
	text_layer_destroy(status_layer);
	text_layer_destroy(volume_text_layer);
	text_layer_destroy(volume_layer);
	progress_bar_layer_destroy(volume_bar);
	progress_bar_layer_destroy(seek_bar);
	
	app_timer_cancel(update_timer);
}
