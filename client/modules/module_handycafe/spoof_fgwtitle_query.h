#pragma once

#include <winternal.h>

bool spoof_fgwtitle_query_load(ldr_data_table_entry *hndy_entry);
bool spoof_fgwtitle_query_unload();
bool spoof_fgwtitle_query_is_loaded();
void spoof_fgwtitle_query_toggle_window();
void spoof_fgwtitle_query_draw_window();