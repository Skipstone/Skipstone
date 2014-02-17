#pragma once
#include "../../common.h"

void plexstatus_init(Player p, Player c);
void plexstatus_destroy(void);
void plexstatus_in_received_handler(DictionaryIterator *iter);
void plexstatus_out_sent_handler(DictionaryIterator *sent);
void plexstatus_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
