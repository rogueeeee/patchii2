#pragma once

#include <Windows.h>
#include <cstdint>

struct load_info_t
{
	decltype(LoadLibraryA)   *import_LoadLibraryA   = nullptr;
	decltype(GetProcAddress) *import_GetProcAddress = nullptr;
	std::uint8_t			 *dll_base              = nullptr;
};