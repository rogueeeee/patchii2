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
        dest = first_entry;
        return true;
    }
    
    if (dest->next == nullptr || dest->next == first_entry)
        return false;

    dest = dest->next;
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
