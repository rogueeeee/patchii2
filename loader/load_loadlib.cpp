#include "load.h"
#include <utils/console.h>
#include <sstream>

#include <Windows.h>

bool load_loadlib(std::uint32_t proc_id)
{
	HANDLE	proc_open			  = nullptr;
	wchar_t path_buffer[MAX_PATH] = { '\0' };
	DWORD	path_size			  = 0;
	LPVOID  path_adr              = nullptr;
	HANDLE  proc_crt              = nullptr;
	bool    is_success            = false;

	{
		console::status_print stat_open("Creating process handle...");
		proc_open = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);

		if (!proc_open)
		{
			stat_open.fail();
			return false;
		}

		stat_open.ok();
	}

	{
		console::status_print stat_path("Loading full path...");
		path_size = GetFullPathNameW(L"patchii_client.dll", MAX_PATH - sizeof(wchar_t), path_buffer, nullptr);

		if (!path_size || path_size > MAX_PATH)
		{
			stat_path.fail();
			return false;
		}

		++path_size *= sizeof(wchar_t);
		std::wcout << L"\n >> " << path_buffer;
		stat_path.ok();
	}
	
	{
		console::status_print stat_alloc("Allocating " + std::to_string(path_size) + " byte(s) of memory...");
		path_adr = VirtualAllocEx(proc_open, nullptr, path_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (!path_adr)
		{
			stat_alloc.fail();
			return false;
		}
		stat_alloc.ok();
	}

	{
		console::status_print stat_write("Writing path...");
		if (!WriteProcessMemory(proc_open, path_adr, path_buffer, path_size, nullptr))
		{
			stat_write.fail();
			goto LBL_CLEANUP;
		}
		stat_write.ok();
	}

	{
		console::status_print stat_load("Loading module...");
		proc_crt = CreateRemoteThread(proc_open, nullptr, NULL, LPTHREAD_START_ROUTINE(&LoadLibraryW), path_adr, NULL, nullptr);

		if (!proc_crt || WaitForSingleObject(proc_crt, INFINITE))
			goto LBL_CLEANUP;

		stat_load.ok();
	}

	is_success = true;
	LBL_CLEANUP:
	console::status_print stat_cleanup("Cleaning up...");
	
	if (proc_crt)
		CloseHandle(proc_crt);

	if (!VirtualFreeEx(proc_open, path_adr, NULL, MEM_RELEASE))
	{
		stat_cleanup.fail();
		std::stringstream adrtostr{};
		adrtostr << path_adr;
		console::print_warning("\nDe-allocation failed. Memory leak at 0x" + adrtostr.str() + " with size of " + std::to_string(path_size) + " byte(s)\n");
	}
	else
	{
		stat_cleanup.ok();
	}

	CloseHandle(proc_open);

	return is_success;
}