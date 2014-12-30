#pragma once

void win_vlc_init(void);
void win_vlc_push(void);
void win_vlc_deinit(void);
bool win_vlc_is_loaded(void);
void win_vlc_reload_data_and_mark_dirty(void);
void win_vlc_set_title(const char *str);
void win_vlc_set_status(const char *str);
void win_vlc_set_volume(uint8_t val);
void win_vlc_set_seek(uint8_t val);
