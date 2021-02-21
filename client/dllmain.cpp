#include <Windows.h>
#include <console.h>

#include "globals.h"
#include "patchii.h"

// TODO: bound to a single flashdrive hwid to prevent from getting leaked
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call != DLL_PROCESS_ATTACH)
        return TRUE;

    HANDLE entry_thread = CreateThread(nullptr, NULL, [](LPVOID hmod) -> DWORD
    {
        globals::dll_handle = reinterpret_cast<HMODULE>(hmod);
        bool console_init_result = console::initialize();
        
        if (!console_init_result)
        {
            MessageBoxW(nullptr, L"Failed to initialize console", L"", MB_ICONERROR);
            goto LBL_IN_INIT_UNLOAD;
        }
        
        console::print_warning("Do not close this window until patchii is completely unloaded.");

        patchii_run();

        LBL_IN_INIT_UNLOAD:
        if (console_init_result)
            FreeConsole();

        if (hmod)
            FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hmod), 1);

        return NULL;
    },
    hModule, NULL, nullptr);

    if (entry_thread)
        CloseHandle(entry_thread);

    return TRUE;
}

