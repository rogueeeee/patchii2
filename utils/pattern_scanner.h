#pragma once

#include <cstdint>
#include <cstddef>

std::uint8_t *ida_pattern_scan(void *base, std::size_t size, const char *pattern, const char *mask);