#pragma once

#include <cstdint>
#include <cstddef>

std::uint8_t *pattern_scan(void *start, std::size_t size, const char *pattern, const char *mask);