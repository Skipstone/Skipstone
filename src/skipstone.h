#pragma once

#include <pebble.h>

extern GBitmap *action_icon_volume_up;
extern GBitmap *action_icon_volume_down;
extern GBitmap *action_icon_play;
extern GBitmap *action_icon_pause;
extern GBitmap *action_icon_play_pause;
extern GBitmap *action_icon_forward;
extern GBitmap *action_icon_rewind;
extern GBitmap *action_icon_next;
extern GBitmap *action_icon_previous;
extern GBitmap *action_icon_up;
extern GBitmap *action_icon_down;
extern GBitmap *action_icon_select;
extern GBitmap *action_icon_back;
extern GBitmap *action_icon_home;
extern GBitmap *action_icon_settings;

void skipstone_init(void);
void skipstone_deinit(void);
void skipstone_in_received_handler(DictionaryIterator *iter);
void skipstone_out_failed_handler(DictionaryIterator *failed, AppMessageResult reason);
void skipstone_reload_data_and_mark_dirty();
void skipstone_request(uint8_t method, uint8_t request);
void skipstone_short_vibe();
