#pragma once
#include "../../common.h"

void plex_init(Player p);
void plex_destroy(void);
void plex_in_received_handler(DictionaryIterator *iter);
void plex_out_sent_handler(DictionaryIterator *sent);
void plex_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
