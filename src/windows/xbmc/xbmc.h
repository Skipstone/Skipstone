#pragma once
#include "../../common.h"

void xbmc_init(Player p);
void xbmc_destroy(void);
void xbmc_in_received_handler(DictionaryIterator *iter);
void xbmc_out_sent_handler(DictionaryIterator *sent);
void xbmc_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
