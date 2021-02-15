#pragma once

#include <cstdint>
#include <cstddef>

bool load_loadlib(std::uint32_t proc_id);
bool load_manualmap(std::uint32_t proc_id, void *bin, std::size_t bin_size);