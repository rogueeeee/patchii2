#include <Windows.h>
#include <console.h>

#include <memory>

#include "globals.h"
#include "patchii.h"

struct entry_info_t
{
    HMODULE hModule;
    DWORD   ul_reason_for_call;
    LPVOID  lpReserved;
} entry_info { 0 };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call != DLL_PROCESS_ATTACH)
        return TRUE;

    entry_info = { hModule, ul_reason_for_call, lpReserved };
    HANDLE entry_thread = CreateThread(nullptr, NULL, [](LPVOID params) -> DWORD
    {
        globals::dll_handle = entry_info.hModule;
        globals::dll_base   = entry_info.hModule ? reinterpret_cast<std::uint8_t *>(entry_info.hModule) : nullptr /* used later when manually mapped and lpReserved is used for passing info */ ;
        
        if (console::initialize())
        {
            console::print_warning("Do not close this window until patchii is completely unloaded.");

            std::cout << "\nEntry point:"
                << "\n\t0x" << entry_info.hModule
                << "\n\t" << entry_info.ul_reason_for_call
                << "\n\t0x" << entry_info.lpReserved;

            patchii_run();

            FreeConsole();
        }
        else
        {
            MessageBoxW(nullptr, L"Failed to initialize console", L"", MB_ICONERROR);
        }
        
        if (entry_info.hModule)
            FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(entry_info.hModule), 1);

        return NULL;
    },
    nullptr, NULL, nullptr);

    if (entry_thread)
        CloseHandle(entry_thread);

    return TRUE;
}

