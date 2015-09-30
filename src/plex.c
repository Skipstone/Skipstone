#include <pebble.h>
#include "plex.h"
#include "libs/pebble-assist.h"
#include "generated/appinfo.h"
#include "generated/keys.h"
#include "skipstone.h"
#include "players.h"
#include "windows/win-plex.h"
#include "windows/win-plexstatus.h"

static PlexClient *clients = NULL;

static uint8_t num_clients = 0;
static uint8_t current_client = 0;

static char *error = NULL;

void plex_init(void) {
	win_plex_init();
	win_plexstatus_init();
}

void plex_deinit(void) {
	free_safe(error);
	free_safe(clients);
	win_plexstatus_deinit();
	win_plex_deinit();
}

void plex_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple = dict_find(iter, APP_KEY_METHOD);
	if (!tuple) return;
	free_safe(error);
	switch (tuple->value->uint8) {
		case KEY_METHOD_ERROR: {
			tuple = dict_find(iter, APP_KEY_STATUS);
			error = malloc(tuple->length);
			strncpy(error, tuple->value->cstring, tuple->length);
			plex_reload_data_and_mark_dirty();
			break;
		}
		case KEY_METHOD_SIZE:
			free_safe(clients);
			num_clients = dict_find(iter, APP_KEY_INDEX)->value->uint8;
			clients = malloc(sizeof(PlexClient) * num_clients);
			if (clients == NULL) num_clients = 0;
			break;
		case KEY_METHOD_DATA: {
			if (!plex_clients_count()) break;
			uint8_t index = dict_find(iter, APP_KEY_INDEX)->value->uint8;
			PlexClient *client = plex_client_get(index);
			client->index = index;
			strncpy(client->title, dict_find(iter, APP_KEY_TITLE)->value->cstring, sizeof(client->title) - 1);
			strncpy(client->subtitle, dict_find(iter, APP_KEY_SUBTITLE)->value->cstring, sizeof(client->subtitle) - 1);
			LOG("plex: %d '%s' '%s'", client->index, client->title, client->subtitle);
			plex_reload_data_and_mark_dirty();
			if (plex_clients_count() == 1) win_plexstatus_push();
			break;
		}
		case KEY_METHOD_STATUS: {
			if (!win_plexstatus_is_loaded()) break;
			tuple = dict_find(iter, APP_KEY_TITLE);
			if (tuple) win_plexstatus_set_title(tuple->value->cstring);
			tuple = dict_find(iter, APP_KEY_SUBTITLE);
			if (tuple) win_plexstatus_set_subtitle(tuple->value->cstring);
			tuple = dict_find(iter, APP_KEY_STATUS);
			if (tuple) win_plexstatus_set_status(tuple->value->cstring);
			tuple = dict_find(iter, APP_KEY_SEEK);
			if (tuple) win_plexstatus_set_seek(tuple->value->uint8);
			win_plexstatus_reload_data_and_mark_dirty();
		}
	}
}

void plex_reload_data_and_mark_dirty() {
	win_plex_reload_data_and_mark_dirty();
	win_plexstatus_reload_data_and_mark_dirty();
}

void plex_request(uint8_t request) {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, APP_KEY_INDEX, plex_client_get_current_index());
	dict_write_uint8(iter, APP_KEY_PLAYER, players_get_current_index());
	dict_write_uint8(iter, APP_KEY_METHOD, KEY_METHOD_PLEX);
	dict_write_uint8(iter, APP_KEY_REQUEST, request);
	dict_write_end(iter);
	app_message_outbox_send();
	plex_reload_data_and_mark_dirty();
}

char* plex_get_error() {
	return (error == NULL && !plex_clients_count()) ? "Loading clients..." : error;
}

uint8_t plex_clients_count() {
	return num_clients;
}

PlexClient* plex_client_get(uint8_t index) {
	return (index < plex_clients_count()) ? &clients[index] : NULL;
}

PlexClient* plex_client_get_current() {
	return &clients[current_client];
}

uint8_t plex_client_get_current_index() {
	return current_client;
}

void plex_client_set_current(uint8_t index) {
	current_client = index;
}
