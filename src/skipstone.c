#include <pebble.h>
#include "skipstone.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "appmessage.h"
#include "players.h"
#include "plex.h"
#include "vlc.h"
#include "xbmc.h"
#include "wdtv.h"

static void timer_callback(void *data);
static AppTimer *timer = NULL;

GBitmap *action_icon_volume_up = NULL;
GBitmap *action_icon_volume_down = NULL;
GBitmap *action_icon_play = NULL;
GBitmap *action_icon_pause = NULL;
GBitmap *action_icon_play_pause = NULL;
GBitmap *action_icon_forward = NULL;
GBitmap *action_icon_rewind = NULL;
GBitmap *action_icon_next = NULL;
GBitmap *action_icon_previous = NULL;
GBitmap *action_icon_up = NULL;
GBitmap *action_icon_down = NULL;
GBitmap *action_icon_select = NULL;
GBitmap *action_icon_back = NULL;
GBitmap *action_icon_home = NULL;
GBitmap *action_icon_settings = NULL;

void skipstone_init(void) {
	timer = app_timer_register(1000, timer_callback, NULL);

	appmessage_init();
	players_init();
	plex_init();
	vlc_init();
	xbmc_init();
	wdtv_init();

	action_icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
	action_icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
	action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
	action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
	action_icon_play_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY_PAUSE);
	action_icon_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FORWARD);
	action_icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);
	action_icon_next = gbitmap_create_with_resource(RESOURCE_ID_ICON_NEXT);
	action_icon_previous = gbitmap_create_with_resource(RESOURCE_ID_ICON_PREVIOUS);
	action_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
	action_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
	action_icon_select = gbitmap_create_with_resource(RESOURCE_ID_ICON_SELECT);
	action_icon_back = gbitmap_create_with_resource(RESOURCE_ID_ICON_BACK);
	action_icon_home = gbitmap_create_with_resource(RESOURCE_ID_ICON_HOME);
	action_icon_settings = gbitmap_create_with_resource(RESOURCE_ID_ICON_SETTINGS);
}

void skipstone_deinit(void) {
	gbitmap_destroy_safe(action_icon_volume_up);
	gbitmap_destroy_safe(action_icon_volume_down);
	gbitmap_destroy_safe(action_icon_play);
	gbitmap_destroy_safe(action_icon_pause);
	gbitmap_destroy_safe(action_icon_play_pause);
	gbitmap_destroy_safe(action_icon_forward);
	gbitmap_destroy_safe(action_icon_rewind);
	gbitmap_destroy_safe(action_icon_next);
	gbitmap_destroy_safe(action_icon_previous);
	gbitmap_destroy_safe(action_icon_up);
	gbitmap_destroy_safe(action_icon_down);
	gbitmap_destroy_safe(action_icon_select);
	gbitmap_destroy_safe(action_icon_back);
	gbitmap_destroy_safe(action_icon_home);
	gbitmap_destroy_safe(action_icon_settings);
	wdtv_deinit();
	xbmc_deinit();
	vlc_deinit();
	plex_deinit();
	players_deinit();
}

void skipstone_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_TYPE);
	if (!tuple) return;
	switch (tuple->value->uint8) {
		case KEY_TYPE_PLAYERS:
			players_in_received_handler(iter);
			break;
		case KEY_TYPE_PLEX:
			plex_in_received_handler(iter);
			break;
		case KEY_TYPE_VLC:
			vlc_in_received_handler(iter);
			break;
		case KEY_TYPE_XBMC:
			xbmc_in_received_handler(iter);
			break;
		case KEY_TYPE_WDTV:
			wdtv_in_received_handler(iter);
			break;
	}
}

void skipstone_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason) {
	LOG("Phone unreachable! Make sure the Pebble app is running.");
}

void skipstone_reload_data_and_mark_dirty() {
	plex_reload_data_and_mark_dirty();
	vlc_reload_data_and_mark_dirty();
	xbmc_reload_data_and_mark_dirty();
	wdtv_reload_data_and_mark_dirty();
}

void skipstone_request(uint8_t method, uint8_t request) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_PLAYER, players_get_current_index());
	dict_write_uint8(iter, APP_KEY_METHOD, method);
	dict_write_uint8(iter, APP_KEY_REQUEST, request);
	dict_write_end(iter);
	app_message_outbox_send();
}

void skipstone_short_vibe() {
	static const uint32_t const segments[] = { 100 };
	vibes_enqueue_custom_pattern((VibePattern) { .durations = segments, .num_segments = ARRAY_LENGTH(segments) });
}

static void timer_callback(void *data) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_READY);
	dict_write_end(iter);
	app_message_outbox_send();
}
