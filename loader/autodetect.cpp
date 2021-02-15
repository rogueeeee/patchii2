#include "autodetect.h"

#include <vector>
#include <Windows.h>
#include <TlHelp32.h>
#include <utils/console.h>

// Automated architecture detection is disabled since current method involves compiling the loader in both architectures, If the feature of loading the module in an architecture
// agnostic way is implemented these features will be reimplemeneted

using IsWow64Process_t = decltype(IsWow64Process);

bool run_autodetect(std::uint32_t &proc_id)
{
    /*
    std::vector<std::pair<std::wstring, DWORD>> cached_proc_list = {};

    console::status_print stat_cacheproc("Caching process snapshot...");
    HANDLE proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (!proc_snap)
    {
        stat_cacheproc.fail();
        return false;
    }

    PROCESSENTRY32W proc_entry = { sizeof(PROCESSENTRY32W) };
    while (Process32NextW(proc_snap, &proc_entry))
    {
        if (proc_entry.szExeFile && proc_entry.th32ProcessID)
            cached_proc_list.push_back(std::make_pair(std::wstring(proc_entry.szExeFile), proc_entry.th32ProcessID));
    }

    std::cout << "\n" << cached_proc_list.size() << " process cached\n";
    stat_cacheproc.ok();

    CloseHandle(proc_snap);

    std::cout << "Starting enumeration...";

    bool had_arch_mismatch = false;

    for (std::pair<const wchar_t *, arch> mod : patchii_get_preload_modules())
    {
        console::status_print stat_detect(std::wstring(L"Enumerating: ") + mod.first);

        if (mod.second != ARCH_EC)
        {
            stat_detect.custom(" ARCH ", console::color::LRED);
            had_arch_mismatch = true;
            continue;
        }

        for (auto cached_proc : cached_proc_list)
        {
            if (cached_proc.first == mod.first)
            {
                stat_detect.ok();
                proc_id = cached_proc.second;
                return true;
            }
        }

        stat_detect.fail();
    }

    std::cout << "\n";

    if (had_arch_mismatch)
        console::print_warning("Some module auto detect checks were skipped due to architechture mismatch (marked with ARCH), use the appropriate loader architechture for your target. Running as " ARCH_STR "\n");
    */
    return false;
}
