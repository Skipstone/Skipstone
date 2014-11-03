#pragma once

#include <pebble.h>

void wdtv_init(void);
void wdtv_deinit(void);
void wdtv_in_received_handler(DictionaryIterator *iter);
void wdtv_reload_data_and_mark_dirty();
void wdtv_request(uint8_t request);
