#pragma once
#include "../../common.h"

void wdtv_init(Player p);
void wdtv_destroy(void);
void wdtv_in_received_handler(DictionaryIterator *iter);
void wdtv_out_sent_handler(DictionaryIterator *sent);
void wdtv_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
