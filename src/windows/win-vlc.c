#include <pebble.h>
#include "win-vlc.h"
#include "libs/pebble-assist.h"
#include "generated/keys.h"
#include "progress_bar.h"
#include "skipstone.h"
#include "players.h"
#include "vlc.h"

#define TITLE_LENGTH 32
#define STATUS_LENGTH 16
#define VOLUME_LENGTH 8

static void update_timer_callback(void *data);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void update_action_bar_icons();
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;

static AppTimer *update_timer = NULL;

static ActionBarLayer *action_bar = NULL;

static TextLayer *title_layer = NULL;
static TextLayer *status_text_layer = NULL;
static TextLayer *status_layer = NULL;
static TextLayer *volume_text_layer = NULL;
static TextLayer *volume_layer = NULL;

static ProgressBarLayer *volume_bar = NULL;
static ProgressBarLayer *seek_bar = NULL;

static char *title = NULL;
static char *status = NULL;
static char *volume = NULL;

typedef enum {
	CONTROLLING_TYPE_VOLUME,
	CONTROLLING_TYPE_SEEK,
	CONTROLLING_TYPE_PLAYLIST
} CONTROLLING_TYPE;

static CONTROLLING_TYPE controlling_type;

void win_vlc_init(void) {
	controlling_type = persist_read_int(KEY_PERSIST_VLC_CONTROLLING_TYPE);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_vlc_push(void) {
	window_stack_push(window, true);
}

void win_vlc_deinit(void) {
	persist_write_int(KEY_PERSIST_VLC_CONTROLLING_TYPE, controlling_type);
	window_destroy_safe(window);
}

bool win_vlc_is_loaded(void) {
	return window_is_loaded(window);
}

void win_vlc_reload_data_and_mark_dirty(void) {
	if (!window_is_loaded(window)) return;
	update_action_bar_icons();
}

void win_vlc_set_title(const char *str) {
	strncpy(title, str, TITLE_LENGTH);
	text_layer_mark_dirty(title_layer);
}

void win_vlc_set_status(const char *str) {
	strncpy(status, str, STATUS_LENGTH);
	text_layer_mark_dirty(status_layer);
}

void win_vlc_set_volume(uint8_t val) {
	snprintf(volume, VOLUME_LENGTH, "%d%%", val);
	text_layer_mark_dirty(volume_layer);
	progress_bar_layer_set_value(volume_bar, val);
}

void win_vlc_set_seek(uint8_t val) {
	progress_bar_layer_set_value(seek_bar, val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void update_timer_callback(void *data) {
	vlc_request(KEY_REQUEST_REFRESH);
	update_timer = app_timer_register(10000, update_timer_callback, NULL);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_VOLUME:
			vlc_request(KEY_REQUEST_VOLUME_INCREMENT);
			break;
		case CONTROLLING_TYPE_SEEK:
			vlc_request(KEY_REQUEST_BACKWARD_SHORT);
			break;
		case CONTROLLING_TYPE_PLAYLIST:
			vlc_request(KEY_REQUEST_PREV);
			break;
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_VOLUME:
			vlc_request(KEY_REQUEST_VOLUME_DECREMENT);
			break;
		case CONTROLLING_TYPE_SEEK:
			vlc_request(KEY_REQUEST_FORWARD_SHORT);
			break;
		case CONTROLLING_TYPE_PLAYLIST:
			vlc_request(KEY_REQUEST_NEXT);
			break;
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	vlc_request(KEY_REQUEST_PLAY_PAUSE);
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_VOLUME:
			vlc_request(KEY_REQUEST_VOLUME_MAX);
			break;
		case CONTROLLING_TYPE_SEEK:
			vlc_request(KEY_REQUEST_BACKWARD_LONG);
			break;
		case CONTROLLING_TYPE_PLAYLIST:
			vlc_request(KEY_REQUEST_PREV);
			break;
	}
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_VOLUME:
			vlc_request(KEY_REQUEST_VOLUME_MIN);
			break;
		case CONTROLLING_TYPE_SEEK:
			vlc_request(KEY_REQUEST_FORWARD_LONG);
			break;
		case CONTROLLING_TYPE_PLAYLIST:
			vlc_request(KEY_REQUEST_NEXT);
			break;
	}
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_type = (controlling_type + 1) % 3;
	update_action_bar_icons();
	skipstone_short_vibe();
}

static void click_config_provider(void *context) {
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 1000, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 1000, down_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
}

static void update_action_bar_icons() {
	switch (controlling_type) {
		case CONTROLLING_TYPE_VOLUME:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_volume_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_volume_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (strcmp(status, "Playing") == 0) ? action_icon_pause : action_icon_play);
			break;
		case CONTROLLING_TYPE_SEEK:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (strcmp(status, "Playing") == 0) ? action_icon_pause : action_icon_play);
			break;
		case CONTROLLING_TYPE_PLAYLIST:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_previous);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_next);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, (strcmp(status, "Playing") == 0) ? action_icon_pause : action_icon_play);
			break;
	}
}

static void window_load(Window *window) {
	update_timer = app_timer_register(0, update_timer_callback, NULL);

	title = malloc(TITLE_LENGTH);
	strncpy(title, players_get_current()->title, TITLE_LENGTH);

	status = malloc(STATUS_LENGTH);
	strncpy(status, "Loading...", STATUS_LENGTH);

	volume = malloc(VOLUME_LENGTH);
	strncpy(volume, "0%", VOLUME_LENGTH);

	action_bar_layer_create_in_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
	update_action_bar_icons();

	title_layer = text_layer_create(GRect(4, 0, 116, 52));
	text_layer_set_system_font(title_layer, FONT_KEY_GOTHIC_24_BOLD);
	text_layer_set_text(title_layer, title);
	text_layer_set_colors(title_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(title_layer, window);

	status_text_layer = text_layer_create(GRect(4, 54, 64, 22));
	text_layer_set_system_font(status_text_layer, FONT_KEY_GOTHIC_18);
	text_layer_set_text(status_text_layer, "Status:");
	text_layer_set_colors(status_text_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(status_text_layer, window);

	status_layer = text_layer_create(GRect(4, 70, 96, 32));
	text_layer_set_system_font(status_layer, FONT_KEY_GOTHIC_28_BOLD);
	text_layer_set_text(status_layer, status);
	text_layer_set_colors(status_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(status_layer, window);

	volume_text_layer = text_layer_create(GRect(4, 108, 48, 22));
	text_layer_set_system_font(volume_text_layer, FONT_KEY_GOTHIC_18);
	text_layer_set_text(volume_text_layer, "Volume:");
	text_layer_set_colors(volume_text_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(volume_text_layer, window);

	volume_layer = text_layer_create(GRect(54, 99, 64, 32));
	text_layer_set_system_font(volume_layer, FONT_KEY_GOTHIC_28_BOLD);
	text_layer_set_text(volume_layer, volume);
	text_layer_set_colors(volume_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(volume_layer, window);

	volume_bar = progress_bar_layer_create(GRect(110, 58, 8, 76));
	progress_bar_layer_set_orientation(volume_bar, ProgressBarOrientationVertical);
	progress_bar_layer_set_range(volume_bar, 0, 200);
	progress_bar_layer_set_frame_color(volume_bar, GColorBlack);
	progress_bar_layer_set_bar_color(volume_bar, GColorBlack);
	layer_add_child(window_get_root_layer(window), volume_bar);

	seek_bar = progress_bar_layer_create(GRect(4, 138, 115, 8));
	progress_bar_layer_set_orientation(seek_bar, ProgressBarOrientationHorizontal);
	progress_bar_layer_set_range(seek_bar, 0, 100);
	progress_bar_layer_set_frame_color(seek_bar, GColorBlack);
	progress_bar_layer_set_bar_color(seek_bar, GColorBlack);
	layer_add_child(window_get_root_layer(window), seek_bar);
}

static void window_unload(Window *window) {
	app_timer_cancel_safe(update_timer);
	action_bar_layer_destroy_safe(action_bar);
	text_layer_destroy_safe(title_layer);
	text_layer_destroy_safe(status_text_layer);
	text_layer_destroy_safe(status_layer);
	text_layer_destroy_safe(volume_text_layer);
	text_layer_destroy_safe(volume_layer);
	progress_bar_layer_destroy(volume_bar);
	progress_bar_layer_destroy(seek_bar);
	free_safe(title);
	free_safe(status);
	free_safe(volume);
}
