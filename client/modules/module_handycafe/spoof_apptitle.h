#pragma once

#include <winternal.h>

bool spoof_apptitle_load(ldr_data_table_entry *hndy_entry);
bool spoof_apptitle_unload();
bool spoof_apptitle_is_loaded();
void spoof_apptitle_toggle_window();
void spoof_apptitle_draw_window();