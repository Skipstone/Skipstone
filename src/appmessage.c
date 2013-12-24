#include <pebble.h>
#include "appmessage.h"
#include "common.h"
#include "windows/plex.h"
#include "windows/vlc.h"
#include "windows/xbmc.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
	app_message_open(128 /* inbound_size */, 32 /* outbound_size */);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *player_tuple = dict_find(iter, KEY_PLAYER);

	if (player_tuple) {
		switch (player_tuple->value->int16) {
			case MediaPlayerPLEX:
//				plex_in_received_handler(iter);
				break;
			case MediaPlayerVLC:
				vlc_in_received_handler(iter);
				break;
			case MediaPlayerXBMC:
//				xbmc_in_received_handler(iter);
				break;
		}
	}

	// list
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped, %d", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	// outgoing message was delivered
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}
