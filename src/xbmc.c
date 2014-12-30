#include <pebble.h>
#include "xbmc.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "skipstone.h"
#include "players.h"
#include "windows/win-xbmc.h"

void xbmc_init(void) {
	win_xbmc_init();
}

void xbmc_deinit(void) {
	win_xbmc_deinit();
}

void xbmc_in_received_handler(DictionaryIterator *iter) {
	if (!win_xbmc_is_loaded()) return;

	Tuple *tuple;

	tuple = dict_find(iter, APP_KEY_TITLE);
	if (tuple) win_xbmc_set_title(tuple->value->cstring);

	tuple = dict_find(iter, APP_KEY_STATUS);
	if (tuple) win_xbmc_set_status(tuple->value->cstring);

	tuple = dict_find(iter, APP_KEY_VOLUME);
	if (tuple) win_xbmc_set_volume(tuple->value->uint8);

	tuple = dict_find(iter, APP_KEY_SEEK);
	if (tuple) win_xbmc_set_seek(tuple->value->uint8);

	xbmc_reload_data_and_mark_dirty();
}

void xbmc_reload_data_and_mark_dirty() {
	win_xbmc_reload_data_and_mark_dirty();
}

void xbmc_request(uint8_t request) {
	skipstone_request(KEY_METHOD_XBMC, request);
	xbmc_reload_data_and_mark_dirty();
}
