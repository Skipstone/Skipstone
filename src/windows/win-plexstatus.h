#pragma once

void win_plexstatus_init(void);
void win_plexstatus_push(void);
void win_plexstatus_deinit(void);
void win_plexstatus_reload_data_and_mark_dirty(void);
void win_plexstatus_set_title(const char *str);
void win_plexstatus_set_subtitle(const char *str);
void win_plexstatus_set_status(const char *str);
void win_plexstatus_set_seek(uint8_t val);
