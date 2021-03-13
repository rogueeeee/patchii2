#pragma once

#include <Windows.h>
#include <cstdint>

struct load_info_t
{
	decltype(LoadLibraryA)   *import_LoadLibraryA   = nullptr;
	decltype(GetProcAddress) *import_GetProcAddress = nullptr;
	std::uint8_t			 *dll_base              = nullptr;
	std::ptrdiff_t            rebase_delta          = NULL;
	PIMAGE_BASE_RELOCATION    image_base_reloc      = nullptr;
	PIMAGE_IMPORT_DESCRIPTOR  image_import_desc     = nullptr;
	BOOL(APIENTRY *DllMain)(HMODULE, DWORD, LPVOID) = nullptr;
};