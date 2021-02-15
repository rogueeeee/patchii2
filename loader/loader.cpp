#include <Windows.h>
#include <TlHelp32.h>
#include <utils/console.h>
#include <utils/arch.h>
#include "autodetect.h"
#include "load.h"

int main(int argc, const char **argv)
{
    std::uint32_t proc_id = 0;

    console::initialize();
    console::set_color(console::color::LPURPLE);
    std::cout << "patchii2";
    console::set_color();
    std::cout << " loader\n"
                 "arch: " ARCH_STR;
    
    console::status_print stat_autodetect("Running autodetect...");
    stat_autodetect.autoset(run_autodetect(proc_id));
    
    std::cout << "\n";

    while (!proc_id)
    {
        std::wstring proc_name;
        std::cout << "Process name (manual mode): ";
        std::getline(std::wcin, proc_name);

        HANDLE proc_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        PROCESSENTRY32W proc_entry = { sizeof(PROCESSENTRY32W) };
        while (Process32NextW(proc_snap, &proc_entry))
        {
            if (proc_entry.szExeFile == proc_name)
            {
                proc_id = proc_entry.th32ProcessID;
                break;
            }
        }
        CloseHandle(proc_snap);

        if (!proc_id)
            console::print_error("No process name matched\n");
    }

    console::status_print stat_load("Running module loader...");
    stat_load.autoset(load_loadlib(proc_id));

    std::cout << "\nModule loader finished, this window can now be closed.\n";
    std::cin.get();

    return 0;
}