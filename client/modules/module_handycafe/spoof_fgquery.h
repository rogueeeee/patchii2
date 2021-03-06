#pragma once

#include <winternal.h>

bool spoof_fgquery_load(ldr_data_table_entry *hndy_entry);
bool spoof_fgquery_unload();
bool spoof_fgquery_is_loaded();
void spoof_fgquery_toggle_window();
void spoof_fgquery_draw_window();