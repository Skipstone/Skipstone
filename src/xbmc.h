#pragma once

#include <pebble.h>

void xbmc_init(void);
void xbmc_deinit(void);
void xbmc_in_received_handler(DictionaryIterator *iter);
void xbmc_reload_data_and_mark_dirty();
void xbmc_request(uint8_t request);
