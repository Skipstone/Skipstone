#include <pebble.h>
#include "appmessage.h"
#include "libs/pebble-assist.h"
#include "skipstone.h"

static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	app_message_open_max();
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	skipstone_in_received_handler(iter);
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	skipstone_out_failed_handler(failed, reason);
}
