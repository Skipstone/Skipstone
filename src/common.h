#pragma once

typedef enum {
	MediaPlayerPLEX = 0x0,
	MediaPlayerVLC = 0x1,
	MediaPlayerXBMC = 0x2,
	MediaPlayerWDTV = 0x3,
	MediaPlayerNONE = 0x255,
} MediaPlayer;

typedef struct {
	int index;
	char title[24];
	char subtitle[30];
	MediaPlayer player;
} Player;

enum {
	KEY_LIST,
	KEY_PLAYER,
	KEY_REQUEST,
	KEY_INDEX,
	KEY_TITLE,
	KEY_STATUS,
	KEY_VOLUME,
	KEY_SEEK,
	KEY_CLIENT,
	KEY_VLC_CONTROLLING_VOLUME,
	KEY_XBMC_CONTROLLING_TYPE,
	KEY_PLEX_CONTROLLING_KEYPAD,
	KEY_WDTV_CONTROLLING_TYPE,
};
