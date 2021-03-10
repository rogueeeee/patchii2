#include <patchii_config.h>
#include <Windows.h>
#include <console.h>
#include <memory>
#include "globals.h"
#include "patchii.h"
#include "load_info.h"

struct entry_info_t
{
    HMODULE      hModule;
    DWORD        ul_reason_for_call;
    load_info_t *load_info;
} entry_info { 0 };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call != DLL_PROCESS_ATTACH)
        return TRUE;

    entry_info = { hModule, ul_reason_for_call, reinterpret_cast<load_info_t *>(lpReserved) };
    
    // TODO: implement reloc and iat fix when manually mapped
    #if defined ( PATCHII_LOADAS_MANUALMAPPED )
    {
    }
    #endif

    HANDLE entry_thread = CreateThread(nullptr, NULL, [](LPVOID params) -> DWORD
    {
        globals::dll.handle = entry_info.hModule;
        globals::dll.base   = entry_info.hModule ? reinterpret_cast<std::uint8_t *>(entry_info.hModule) : entry_info.load_info->dll_base;
        
        if (console::initialize())
        {
            console::print_warning("Do not close this window until patchii is completely unloaded.");

            std::cout << "\nEntry point:"
                      << "\n\t>>> Module Handle: 0x" << entry_info.hModule
                      << "\n\t>>>        Reason: "   << entry_info.ul_reason_for_call
                      << "\n\t>>>   Loader info: 0x" << entry_info.load_info;

            patchii_run();
            FreeConsole();
        }
        else
        {
            MessageBoxW(nullptr, L"patchii\n\nFailed to initialize console", L"", MB_ICONERROR);
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

