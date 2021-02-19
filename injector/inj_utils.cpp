#include "inj_utils.h"
#include <TlHelp32.h>

bool remote_LoadLibraryW(HANDLE proc_handle, std::wstring path)
{
    HANDLE      ll_crt     = nullptr;
    std::size_t path_size  = (path.length() + 1) * sizeof(wchar_t);
    bool        is_success = false;

    LPVOID path_alloc = VirtualAllocEx(proc_handle, nullptr, path_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!path_alloc)
        return false;

    if (!WriteProcessMemory(proc_handle, path_alloc, path.c_str(), path_size, nullptr))
        goto LBL_FAIL_B;

    ll_crt = CreateRemoteThread(proc_handle, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(&LoadLibraryW), path_alloc, NULL, nullptr);
    if (!ll_crt)
        goto LBL_FAIL_B;

    if (WaitForSingleObject(ll_crt, 30000) & WAIT_FAILED)
        goto LBL_FAIL_A;

    is_success = true;
    LBL_FAIL_A:
    CloseHandle(ll_crt);
    LBL_FAIL_B:
    VirtualFreeEx(proc_handle, path_alloc, NULL, MEM_RELEASE);

    return is_success;
}

#if 0
HMODULE remote_LoadLibraryA(HANDLE proc_handle, LPCSTR file_name)
{
    SIZE_T fname_alloc_size = strlen(file_name) + 1;

    LPVOID fname_alloc_address = VirtualAllocEx(proc_handle, nullptr, fname_alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!fname_alloc_address)
        return nullptr;

    if (!WriteProcessMemory(proc_handle, fname_alloc_address, file_name, fname_alloc_size, nullptr))
        return nullptr;

    HANDLE ll_crt = CreateRemoteThread(proc_handle, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(&LoadLibraryA), fname_alloc_address, NULL, nullptr);
    if (!ll_crt)
        return nullptr;

    WaitForSingleObject(ll_crt, INFINITE);

    VirtualFreeEx(proc_handle, fname_alloc_address, NULL, MEM_RELEASE);

    DWORD proc_id = GetProcessId(proc_handle);
    if (!proc_id)
        return nullptr;

    HANDLE mod_snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, proc_id);
    
    if (!mod_snap)
        return nullptr;

    std::wstring uni_file_name;
    {
        wchar_t _uni_file_name[MAX_PATH] = { L'\0' };
        MultiByteToWideChar(CP_UTF8, NULL, file_name, -1, _uni_file_name, MAX_PATH);
        uni_file_name = _uni_file_name;
    }

    HMODULE        result    = nullptr;
    MODULEENTRY32W mod_entry = { sizeof(MODULEENTRY32W) };
    while (Module32NextW(mod_snap, &mod_entry))
    {
        if (mod_entry.szModule == uni_file_name)
        {
            result = reinterpret_cast<HMODULE>(mod_entry.modBaseAddr);
            break;
        }
    }

    CloseHandle(mod_snap);

    return result;
}

FARPROC remote_GetProcAddress(HANDLE proc_handle, HMODULE mod_handle, LPCSTR proc_name)
{
    SIZE_T proc_name_len = strlen(proc_name) + 1;

    #ifdef _M_IX86
        constexpr std::ptrdiff_t scoff_proc_name_alloc_addr = 1;
        constexpr std::ptrdiff_t scoff_mod_handle           = 6;
        constexpr std::ptrdiff_t scoff_GetProcAddress       = 11;
        constexpr std::ptrdiff_t scoff_return_set           = 16;
        constexpr std::ptrdiff_t scoff_return_buff          = 23;

        std::uint8_t shellcode[] =
        {
            0x68, 0x00, 0x00, 0x00, 0x00, // push proc_name_alloc_addr
            0x68, 0x00, 0x00, 0x00, 0x00, // push mod_handle
            0xE8, 0x00, 0x00, 0x00, 0x00, // call GetProcAddress
            0xA3, 0x00, 0x00, 0x00, 0x00, // mov  shellcode_alloc + sizeof(shellcode) - sizeof(FARPROC), eax
            0xC2, 0x04, 0x00,             // ret 4
            0x00, 0x00, 0x00, 0x00        // return buffer
        };
    #elif _M_X64
        constexpr std::ptrdiff_t scoff_proc_name_alloc_addr = 3;
        constexpr std::ptrdiff_t scoff_mod_handle           = 14;
        constexpr std::ptrdiff_t scoff_GetProcAddress       = 23;
        constexpr std::ptrdiff_t scoff_return_set           = 31;
        constexpr std::ptrdiff_t scoff_return_buff          = 40;

        std::uint8_t shellcode[] =
        {
            0x48, 0xC7, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       // mov  rcx, mod_handle
            0x48, 0xC7, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       // mov  rdx, proc_name_alloc_addr
            0xE9, 0x00, 0x00, 0x00, 0x00,                                           // jmp  GetProcAddress relative address
            0x48, 0x89, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov  shellcode_alloc + sizeof(shellcode) - sizeof(FARPROC), rax
            0xC3,                                                                   // ret
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         // return buffer
        };
    #endif

    std::uintptr_t shellcode_alloc = reinterpret_cast<std::uintptr_t>(VirtualAllocEx(proc_handle, nullptr, sizeof(shellcode) + proc_name_len, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));

    if (!shellcode_alloc)
        return nullptr;

    if (!WriteProcessMemory(proc_handle, reinterpret_cast<LPVOID>(shellcode_alloc + sizeof(shellcode)), proc_name, proc_name_len, nullptr))
        return nullptr;

    *reinterpret_cast<const char **> (shellcode + scoff_proc_name_alloc_addr) = reinterpret_cast<const char *>(shellcode_alloc + sizeof(shellcode));
    *reinterpret_cast<HMODULE *>     (shellcode + scoff_mod_handle)           = mod_handle;
    *reinterpret_cast<FARPROC *>     (shellcode + scoff_return_set)           = reinterpret_cast<FARPROC>(shellcode_alloc + sizeof(shellcode) - sizeof(FARPROC));

    #ifdef _M_IX86
        *reinterpret_cast<decltype(GetProcAddress) **>(shellcode + scoff_GetProcAddress) = &GetProcAddress;
    #else
        // *reinterpret_cast<std::int32_t*>(shellcode + scoff_GetProcAddress) = reinterpret_cast<std::uintptr_t>(&GetProcAddress) - reinterpret_cast<std::int32_t>(shellcode_alloc + scoff_GetProcAddress + 0x5);
    #endif

    if (!WriteProcessMemory(proc_handle, reinterpret_cast<LPVOID>(shellcode_alloc), shellcode, sizeof(shellcode), nullptr))
        return nullptr;

    HANDLE sc_crt = CreateRemoteThread(proc_handle, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(shellcode_alloc), nullptr, NULL, nullptr);

    if (sc_crt)
        return nullptr;

    WaitForSingleObject(sc_crt, INFINITE);

    FARPROC result = nullptr;

    if (!ReadProcessMemory(proc_handle, reinterpret_cast<LPCVOID>(shellcode_alloc + sizeof(shellcode) - sizeof(FARPROC)), &result, sizeof(FARPROC), nullptr))
        return nullptr;

    VirtualFreeEx(proc_handle, reinterpret_cast<LPVOID>(shellcode_alloc), NULL, MEM_RELEASE);

    return result;
}
#endif