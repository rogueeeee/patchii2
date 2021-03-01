#pragma once

#include <cstdint>
#include <Windows.h>
#include <winternl.h>

struct ldr_data_table_entry
{
public:
	ldr_data_table_entry *next;
private:
	std::uintptr_t pad0[5];
public:
	std::uint8_t *dll_base;
private:
	std::uintptr_t pad1;
public:
	std::uintptr_t size_of_image;
private:
	std::uint16_t pad2[2];
public: 
	const wchar_t *full_dll_name;
private:
	std::uint16_t pad3[2];
public:
	const wchar_t *base_dll_name;
};

bool ldr_data_table_entry_next(ldr_data_table_entry *&dest);

bool pe_validate_dosheader(void *base);
PIMAGE_DOS_HEADER pe_get_dosheaderptr(void *base);
PIMAGE_NT_HEADERS pe_get_ntheaderptr(void *base);
bool pe_image_base_reloc_next(void *base, PIMAGE_BASE_RELOCATION &dest);
bool pe_image_import_descriptor_next(void *base, PIMAGE_IMPORT_DESCRIPTOR &dest);
bool pe_image_thunk_data_next(void *base, PIMAGE_IMPORT_DESCRIPTOR descriptor, PIMAGE_THUNK_DATA &dest_orig, PIMAGE_THUNK_DATA &dest_first);