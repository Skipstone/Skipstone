#include <pebble.h>
#include "vlc.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "skipstone.h"
#include "players.h"
#include "windows/win-vlc.h"

void vlc_init(void) {
	win_vlc_init();
}

void vlc_deinit(void) {
	win_vlc_deinit();
}

void vlc_in_received_handler(DictionaryIterator *iter) {
	if (!win_vlc_is_loaded()) return;

	Tuple *tuple;

	tuple = dict_find(iter, APP_KEY_TITLE);
	if (tuple) win_vlc_set_title(tuple->value->cstring);

	tuple = dict_find(iter, APP_KEY_STATUS);
	if (tuple) win_vlc_set_status(tuple->value->cstring);

	tuple = dict_find(iter, APP_KEY_VOLUME);
	if (tuple) win_vlc_set_volume(tuple->value->uint8);

	tuple = dict_find(iter, APP_KEY_SEEK);
	if (tuple) win_vlc_set_seek(tuple->value->uint8);

	vlc_reload_data_and_mark_dirty();
}

void vlc_reload_data_and_mark_dirty() {
	win_vlc_reload_data_and_mark_dirty();
}

void vlc_request(uint8_t request) {
	skipstone_request(KEY_METHOD_VLC, request);
	vlc_reload_data_and_mark_dirty();
}
