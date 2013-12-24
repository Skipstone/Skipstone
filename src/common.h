#pragma once

typedef enum {
	MediaPlayerPLEX = 0x0,
	MediaPlayerVLC = 0x1,
	MediaPlayerXBMC = 0x2,
} MediaPlayer;

typedef struct {
	int index;
	char title[24];
	char subtitle[30];
	MediaPlayer player;
} Player;

enum {
	KEY_PLAYER,
	KEY_REQUEST,
	KEY_INDEX,
	KEY_TITLE,
	KEY_STATUS,
	KEY_VOLUME,
	KEY_SEEK,
	KEY_VLC_CONTROLLING_VOLUME,
};
