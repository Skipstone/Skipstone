#include <pebble.h>
#include "xbmc.h"
#include "../libs/pebble-assist.h"
#include "../common.h"

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

static Player player;

typedef enum {
  CONTROLLING_TYPE_KEYPAD,
  CONTROLLING_TYPE_SEEK,
  CONTROLLING_TYPE_VOLUME
} CONTROLLING_TYPE;

static CONTROLLING_TYPE controlling_type;

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
    {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_up);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_down);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_select);
      break;
    }
    case CONTROLLING_TYPE_SEEK:
    {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_rewind);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_forward);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);
      break;
    }
    case CONTROLLING_TYPE_VOLUME:
    {
      action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_volume_up);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_volume_down);
      action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);
      break;
    }
  }
}

static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	xbmc_destroy();
	window_stack_pop(true);
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
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

	title_layer = text_layer_create((GRect) { .origin = { 5, 0 }, .size = { bounds.size.w - 28, 52 } });
	text_layer_set_text(title_layer, player.title);
	text_layer_set_text_color(title_layer, GColorBlack);
	text_layer_set_background_color(title_layer, GColorClear);
	text_layer_set_font(title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	layer_add_child(window_layer, text_layer_get_layer(title_layer));
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
}
