#pragma once

void playerlist_init(void);
void playerlist_destroy(void);
void playerlist_in_received_handler(DictionaryIterator *iter);
void playerlist_in_dropped_handler(AppMessageResult reason);
void playerlist_out_sent_handler(DictionaryIterator *sent);
void playerlist_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
