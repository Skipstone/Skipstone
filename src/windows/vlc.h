#pragma once
#include "../common.h"

void vlc_init(Player p);
void vlc_destroy(void);
void vlc_in_received_handler(DictionaryIterator *iter);
