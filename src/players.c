#include <pebble.h>
#include "players.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "windows/win-players.h"

static Player *players = NULL;

static uint8_t num_players = 0;
static uint8_t current_player = 0;

static char *error = NULL;

void players_init(void) {
	win_players_init();

	current_player = persist_read_int(KEY_PERSIST_LAST_USED_PLAYER);
}

void players_deinit(void) {
	persist_write_int(KEY_PERSIST_LAST_USED_PLAYER, current_player);
	free_safe(error);
	free_safe(players);
	win_players_deinit();
}

void players_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	free_safe(error);
	switch (tuple->value->uint8) {
		case KEY_METHOD_ERROR: {
			tuple = dict_find(iter, APP_KEY_STATUS);
			error = malloc(tuple->length);
			strncpy(error, tuple->value->cstring, tuple->length);
			players_reload_data_and_mark_dirty();
			break;
		}
		case KEY_METHOD_SIZE:
			free_safe(players);
			num_players = dict_find(iter, APP_KEY_INDEX)->value->uint8;
			players = malloc(sizeof(Player) * num_players);
			if (players == NULL) num_players = 0;
			break;
		case KEY_METHOD_DATA: {
			if (!players_count()) break;
			uint8_t index = dict_find(iter, APP_KEY_INDEX)->value->uint8;
			Player *player = players_get(index);
			player->index = index;
			player->mediaplayer = dict_find(iter, APP_KEY_PLAYER)->value->uint8;
			strncpy(player->title, dict_find(iter, APP_KEY_TITLE)->value->cstring, sizeof(player->title) - 1);
			LOG("player: %d '%s' '%s'", player->index, player->title, player_to_str(player->mediaplayer));
			players_reload_data_and_mark_dirty();
			if (index == current_player) win_players_push_player();
			break;
		}
	}
}

void players_reload_data_and_mark_dirty() {
	win_players_reload_data_and_mark_dirty();
}

void players_request() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_REQUESTPLAYERS);
	dict_write_end(iter);
	app_message_outbox_send();
	players_reload_data_and_mark_dirty();
}

uint8_t players_count() {
	return num_players;
}

void players_count_set(uint8_t count) {
	num_players = count;
}

char* players_get_error() {
	return (error == NULL && !players_count()) ? "Loading..." : error;
}

Player* players_get(uint8_t index) {
	return (index < players_count()) ? &players[index] : NULL;
}

Player* players_get_current() {
	return &players[current_player];
}

uint8_t players_get_current_index() {
	return current_player;
}

void players_set_current(uint8_t index) {
	current_player = index;
}

char* player_to_str(MediaPlayer mediaplayer) {
	switch (mediaplayer) {
		case MediaPlayerPLEX:
			return "Plex";
		case MediaPlayerVLC:
			return "VLC";
		case MediaPlayerXBMC:
			return "XBMC";
		case MediaPlayerWDTV:
			return "WDTV";
		default:
			return "";
	}
}
