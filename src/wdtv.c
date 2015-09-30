#include <pebble.h>
#include "wdtv.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "skipstone.h"
#include "players.h"
#include "windows/win-wdtv.h"

void wdtv_init(void) {
	win_wdtv_init();
}

void wdtv_deinit(void) {
	win_wdtv_deinit();
}

void wdtv_in_received_handler(DictionaryIterator *iter) {
}

void wdtv_reload_data_and_mark_dirty() {
	win_wdtv_reload_data_and_mark_dirty();
}

void wdtv_request(uint8_t request) {
	skipstone_request(KEY_METHOD_WDTV, request);
	wdtv_reload_data_and_mark_dirty();
}
