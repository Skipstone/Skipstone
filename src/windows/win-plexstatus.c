#include <pebble.h>
#include "win-plexstatus.h"
#include "libs/pebble-assist.h"
#include "generated/keys.h"
#include "progress_bar.h"
#include "skipstone.h"
#include "players.h"
#include "plex.h"

#define TITLE_LENGTH 32
#define SUBTITLE_LENGTH 16
#define STATUS_LENGTH 16

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_double_click_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void update_action_bar_icons();
static void window_load(Window *window);
static void window_unload(Window *window);

static Window *window = NULL;

static ActionBarLayer *action_bar = NULL;

static BitmapLayer *logo_bitmap_layer = NULL;
static GBitmap *logo_bitmap = NULL;

static TextLayer *title_layer = NULL;
static TextLayer *subtitle_layer = NULL;
static TextLayer *status_layer = NULL;
static TextLayer *footer_layer = NULL;

static char *title = NULL;
static char *subtitle = NULL;
static char *status = NULL;

static ProgressBarLayer *seek_bar = NULL;

typedef enum {
	CONTROLLING_TYPE_KEYPAD,
	CONTROLLING_TYPE_SEEK
} CONTROLLING_TYPE;

static CONTROLLING_TYPE controlling_type;

void win_plexstatus_init(void) {
	controlling_type = persist_read_int(KEY_PERSIST_PLEX_CONTROLLING_TYPE);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
}

void win_plexstatus_push(void) {
	window_stack_push(window, true);
}

void win_plexstatus_deinit(void) {
	persist_write_int(KEY_PERSIST_PLEX_CONTROLLING_TYPE, controlling_type);
	window_destroy_safe(window);
}

bool win_plexstatus_is_loaded(void) {
	return window_is_loaded(window);
}

void win_plexstatus_reload_data_and_mark_dirty(void) {
	if (!window_is_loaded(window)) return;
	bool hide_logo = *subtitle;
	layer_set_hidden(bitmap_layer_get_layer(logo_bitmap_layer), hide_logo);
	layer_set_hidden(seek_bar, !hide_logo);
}

void win_plexstatus_set_title(const char *str) {
	strncpy(title, str, TITLE_LENGTH);
	text_layer_mark_dirty(title_layer);
}

void win_plexstatus_set_subtitle(const char *str) {
	strncpy(subtitle, str, SUBTITLE_LENGTH);
	text_layer_mark_dirty(subtitle_layer);
}

void win_plexstatus_set_status(const char *str) {
	strncpy(status, str, STATUS_LENGTH);
	text_layer_mark_dirty(status_layer);
}

void win_plexstatus_set_seek(uint8_t val) {
	progress_bar_layer_set_value(seek_bar, val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			plex_request(KEY_REQUEST_UP);
			break;
		case CONTROLLING_TYPE_SEEK:
			plex_request(KEY_REQUEST_BACKWARD_SHORT);
			break;
	}
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			plex_request(KEY_REQUEST_DOWN);
			break;
		case CONTROLLING_TYPE_SEEK:
			plex_request(KEY_REQUEST_FORWARD_SHORT);
			break;
	}
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			plex_request(KEY_REQUEST_SELECT);
			break;
		case CONTROLLING_TYPE_SEEK:
			plex_request(KEY_REQUEST_PLAY_PAUSE);
			break;
	}
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			plex_request(KEY_REQUEST_LEFT);
			break;
		case CONTROLLING_TYPE_SEEK:
			plex_request(KEY_REQUEST_BACKWARD_LONG);
			break;
	}
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			plex_request(KEY_REQUEST_RIGHT);
			break;
		case CONTROLLING_TYPE_SEEK:
			plex_request(KEY_REQUEST_FORWARD_LONG);
			break;
	}
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	controlling_type = (controlling_type + 1) % 2;
	update_action_bar_icons();
	skipstone_short_vibe();
}

static void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
	plex_request(KEY_REQUEST_BACK);
}

static void click_config_provider(void *context) {
	window_single_repeating_click_subscribe(BUTTON_ID_UP, 1000, up_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 1000, down_single_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_SELECT, 1000, select_single_click_handler);
	window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 500, select_long_click_handler, NULL);
	window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, false, select_double_click_handler);
}

static void update_action_bar_icons() {
	switch (controlling_type) {
		case CONTROLLING_TYPE_KEYPAD:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);
			break;
		case CONTROLLING_TYPE_SEEK:
			action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
			action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play_pause);
			break;
	}
}

static void window_load(Window *window) {
	title = malloc(TITLE_LENGTH);
	strncpy(title, plex_client_get_current()->title, TITLE_LENGTH);

	subtitle = malloc(SUBTITLE_LENGTH);
	strncpy(subtitle, "", SUBTITLE_LENGTH);

	status = malloc(STATUS_LENGTH);
	strncpy(status, "", STATUS_LENGTH);

	action_bar_layer_create_in_window(action_bar, window);
	action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
	update_action_bar_icons();

	title_layer = text_layer_create(GRect(4, 0, 116, 52));
	text_layer_set_system_font(title_layer, FONT_KEY_GOTHIC_24_BOLD);
	text_layer_set_text(title_layer, title);
	text_layer_set_colors(title_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(title_layer, window);

	subtitle_layer = text_layer_create(GRect(4, 86, 114, 28));
	text_layer_set_system_font(subtitle_layer, FONT_KEY_GOTHIC_24_BOLD);
	text_layer_set_text_alignment(subtitle_layer, GTextAlignmentRight);
	text_layer_set_text(subtitle_layer, subtitle);
	text_layer_set_colors(subtitle_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(subtitle_layer, window);

	status_layer = text_layer_create(GRect(4, 54, 116, 32));
	text_layer_set_system_font(status_layer, FONT_KEY_GOTHIC_28_BOLD);
	text_layer_set_text(status_layer, status);
	text_layer_set_colors(status_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(status_layer, window);

	footer_layer = text_layer_create(GRect(4, 128, 116, 20));
	text_layer_set_system_font(footer_layer, FONT_KEY_GOTHIC_18);
	text_layer_set_text(footer_layer, plex_client_get_current()->subtitle);
	text_layer_set_colors(footer_layer, GColorBlack, GColorClear);
	text_layer_add_to_window(footer_layer, window);

	seek_bar = progress_bar_layer_create(GRect(4, 118, 115, 8));
	progress_bar_layer_set_orientation(seek_bar, ProgressBarOrientationHorizontal);
	progress_bar_layer_set_range(seek_bar, 0, 100);
	progress_bar_layer_set_frame_color(seek_bar, GColorBlack);
	progress_bar_layer_set_bar_color(seek_bar, GColorBlack);
	layer_add_child(window_get_root_layer(window), seek_bar);
	layer_set_hidden(seek_bar, true);

	logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLEX_LOGO);
	logo_bitmap_layer = bitmap_layer_create(GRect(24, 44, 80, 80));
	bitmap_layer_set_bitmap(logo_bitmap_layer, logo_bitmap);
	bitmap_layer_add_to_window(logo_bitmap_layer, window);

	plex_request(KEY_REQUEST_REFRESH);
}

static void window_unload(Window *window) {
	action_bar_layer_destroy_safe(action_bar);
	text_layer_destroy_safe(title_layer);
	text_layer_destroy_safe(subtitle_layer);
	text_layer_destroy_safe(status_layer);
	text_layer_destroy_safe(footer_layer);
	progress_bar_layer_destroy(seek_bar);
	bitmap_layer_destroy_safe(logo_bitmap_layer);
	gbitmap_destroy_safe(logo_bitmap);
	free_safe(title);
	free_safe(subtitle);
	free_safe(status);
}
