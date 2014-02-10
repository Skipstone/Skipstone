#include <pebble.h>
#include "appmessage.h"
#include "common.h"
#include "libs/pebble-assist.h"
#include "windows/playerlist.h"
#include "windows/plex/plex.h"
#include "windows/vlc/vlc.h"
#include "windows/xbmc/xbmc.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open(128 /* inbound_size */, 64 /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	if (dict_find(iter, KEY_LIST)) {
		playerlist_in_received_handler(iter);
		return;
	}
	Tuple *player_tuple = dict_find(iter, KEY_PLAYER);
	if (player_tuple) {
		switch (player_tuple->value->int16) {
			case MediaPlayerPLEX:
				plex_in_received_handler(iter);
				break;
			case MediaPlayerVLC:
				vlc_in_received_handler(iter);
				break;
			case MediaPlayerXBMC:
				xbmc_in_received_handler(iter);
				break;
		}
	}
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped, %d", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	if (dict_find(sent, KEY_LIST)) {
		playerlist_out_sent_handler(sent);
		return;
	}
	Tuple *player_tuple = dict_find(sent, KEY_PLAYER);
	if (player_tuple) {
		switch (player_tuple->value->int16) {
			case MediaPlayerPLEX:
				plex_out_sent_handler(sent);
				break;
			case MediaPlayerVLC:
				vlc_out_sent_handler(sent);
				break;
			case MediaPlayerXBMC:
				xbmc_out_sent_handler(sent);
				break;
		}
	}
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	if (dict_find(failed, KEY_LIST)) {
		playerlist_out_failed_handler(failed, reason);
		return;
	}
	Tuple *player_tuple = dict_find(failed, KEY_PLAYER);
	if (player_tuple) {
		switch (player_tuple->value->int16) {
			case MediaPlayerPLEX:
				plex_out_failed_handler(failed, reason);
				break;
			case MediaPlayerVLC:
				vlc_out_failed_handler(failed, reason);
				break;
			case MediaPlayerXBMC:
				xbmc_out_failed_handler(failed, reason);
				break;
		}
	}
}
