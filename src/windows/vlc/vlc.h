#pragma once
#include "../../common.h"

void vlc_init(Player p);
void vlc_destroy(void);
void vlc_in_received_handler(DictionaryIterator *iter);
void vlc_out_sent_handler(DictionaryIterator *sent);
void vlc_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
