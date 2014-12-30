#pragma once

#include <pebble.h>

typedef enum {
	MediaPlayerPLEX,
	MediaPlayerVLC,
	MediaPlayerXBMC,
	MediaPlayerWDTV,
} MediaPlayer;

typedef struct {
	uint8_t index;
	char title[24];
	MediaPlayer mediaplayer;
} Player;

void players_init(void);
void players_deinit(void);
void players_in_received_handler(DictionaryIterator *iter);
void players_reload_data_and_mark_dirty();
void players_request();
uint8_t players_count();
void players_count_set(uint8_t count);
char* players_get_error();
Player* players_get(uint8_t index);
Player* players_get_current();
uint8_t players_get_current_index();
void players_set_current(uint8_t index);
char* player_to_str(MediaPlayer mediaplayer);
