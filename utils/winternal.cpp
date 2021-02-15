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
        #ifdef WIN32
            return reinterpret_cast<peb *>(__readfsdword(0x30))->ldr->entry;
        #else
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
