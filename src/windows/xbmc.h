#pragma once
#include "../common.h"

void xbmc_init(Player p);
void xbmc_destroy(void);
void xbmc_in_received_handler(DictionaryIterator *iter);
