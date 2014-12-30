#pragma once

#include <pebble.h>

void vlc_init(void);
void vlc_deinit(void);
void vlc_in_received_handler(DictionaryIterator *iter);
void vlc_reload_data_and_mark_dirty();
void vlc_request(uint8_t request);
