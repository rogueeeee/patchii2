#include "spoof_fgwtitle_query.h"

enum class spoof_mode
{
    UNAVAILABLE,
    ANSII,
    WUNICODE
};

spoof_mode mode = spoof_mode::UNAVAILABLE;

bool spoof_fgwtitle_query_load(ldr_data_table_entry *hndy_entry)
{
    return false;
}

bool spoof_fgwtitle_query_unload()
{
    return false;
}

bool spoof_fgwtitle_query_is_loaded()
{
    return mode != spoof_mode::UNAVAILABLE;
}

void spoof_fgwtitle_query_toggle_window()
{
}

void spoof_fgwtitle_query_draw_window()
{
}
