#pragma once

void win_xbmc_init(void);
void win_xbmc_push(void);
void win_xbmc_deinit(void);
bool win_xbmc_is_loaded(void);
void win_xbmc_reload_data_and_mark_dirty(void);
void win_xbmc_set_title(const char *str);
void win_xbmc_set_status(const char *str);
void win_xbmc_set_volume(uint8_t val);
void win_xbmc_set_seek(uint8_t val);
