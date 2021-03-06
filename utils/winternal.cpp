#include "winternal.h"

#include <intrin.h>

struct peb_ldr_data
{
private:
    std::uint32_t  pad0[2];
    std::uintptr_t pad1;
public:
    ldr_data_table_entry *entry;
};

struct peb
{
private:
    std::uint8_t   pad0[4];
    std::uintptr_t pad1[2];
public:
    peb_ldr_data *ldr;
};

bool ldr_data_table_entry_next(ldr_data_table_entry *&dest)
{
    static ldr_data_table_entry *first_entry = [] () -> auto
    {
        #ifdef _M_IX86
            return reinterpret_cast<peb *>(__readfsdword(0x30))->ldr->entry;
        #elif _M_X64
            return reinterpret_cast<peb *>(__readgsqword(0x60))->ldr->entry;
        #endif
    }();

    if (!dest)
    {
        if (!first_entry)
            return false;

        dest = first_entry;
        return true;
    }
    
    if (dest->next == nullptr || dest->next == first_entry)
        return false;

    dest = dest->next;
    return true;
}

ldr_data_table_entry *ldr_data_table_entry_find(const wchar_t *name)
{
    ldr_data_table_entry *entry = nullptr;
    while (ldr_data_table_entry_next(entry))
    {
        if (!entry->dll_base)
            continue;

        if (wcscmp(entry->base_dll_name, name) == 0)
            return entry;
    }

    return nullptr;
}

bool ldr_data_table_entry_find(const wchar_t *name, ldr_data_table_entry *&dest)
{
    dest = ldr_data_table_entry_find(name);

    if (!dest)
        return false;

    return true;
}

bool pe_validate_dosheader(void *base)
{
    return reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_magic == IMAGE_DOS_SIGNATURE;
}

PIMAGE_DOS_HEADER pe_get_dosheaderptr(void *base)
{
    return reinterpret_cast<PIMAGE_DOS_HEADER>(base);
}

PIMAGE_NT_HEADERS pe_get_ntheaderptr(void *base)
{
    return reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uintptr_t>(base) + pe_get_dosheaderptr(base)->e_lfanew);
}

bool pe_image_base_reloc_next(void *base, PIMAGE_BASE_RELOCATION &dest)
{
    DWORD reloc_size = pe_get_ntheaderptr(base)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

    if (!reloc_size)
        return false;
    
    PIMAGE_BASE_RELOCATION first_reloc_entry = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<std::uintptr_t>(base) + pe_get_ntheaderptr(base)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

    if (!dest)
    {
        if (!first_reloc_entry->VirtualAddress)
            return false;

        dest = first_reloc_entry;
        return true;
    }

    PIMAGE_BASE_RELOCATION next_reloc_entry = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<std::uintptr_t>(dest) + dest->SizeOfBlock);

    if (reinterpret_cast<std::uintptr_t>(next_reloc_entry) >= reinterpret_cast<std::uintptr_t>(first_reloc_entry) + reloc_size)
        return false;

    dest = next_reloc_entry;
    return true;
}

bool pe_image_import_descriptor_next(void *base, PIMAGE_IMPORT_DESCRIPTOR &dest)
{
    if (!dest)
    {
        dest = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(reinterpret_cast<std::uintptr_t>(base) + pe_get_ntheaderptr(base)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        if (!dest->Name)
            return false;

        return true;
    }
    
    if (!++dest->Name)
        return false;

    return true;
}

bool pe_image_thunk_data_next(void *base, PIMAGE_IMPORT_DESCRIPTOR descriptor, PIMAGE_THUNK_DATA &dest_orig, PIMAGE_THUNK_DATA &dest_first)
{
    if (!dest_orig || !dest_first)
    {
        dest_orig  = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<std::uintptr_t>(base) + descriptor->OriginalFirstThunk);
        dest_first = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<std::uintptr_t>(base) + descriptor->FirstThunk);

        if (!dest_orig->u1.AddressOfData)
            return false;

        return true;
    }

    if (!++dest_orig->u1.AddressOfData)
        return false;

    return false;
}
