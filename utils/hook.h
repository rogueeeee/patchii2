#pragma once

bool hook_nearcall86(void *call_location, void *hook_func, void **original_address);