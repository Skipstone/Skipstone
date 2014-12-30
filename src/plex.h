#pragma once

#include <pebble.h>

typedef struct {
	uint8_t index;
	char title[24];
	char subtitle[32];
} PlexClient;

void plex_init(void);
void plex_deinit(void);
void plex_in_received_handler(DictionaryIterator *iter);
void plex_reload_data_and_mark_dirty();
void plex_request(uint8_t request);
char* plex_get_error();
uint8_t plex_clients_count();
PlexClient* plex_client_get(uint8_t index);
PlexClient* plex_client_get_current();
uint8_t plex_client_get_current_index();
void plex_client_set_current(uint8_t index);
