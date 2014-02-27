#pragma once
#include "../../common.h"

void atv_init(Player p);
void atv_destroy(void);
void atv_in_received_handler(DictionaryIterator *iter);
void atv_out_sent_handler(DictionaryIterator *sent);
void atv_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
