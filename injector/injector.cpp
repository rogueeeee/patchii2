#include <winternal.h>
#include <string_utils.h>

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "inj_utils.h"
#include "injector_return_code.h"

#include "binaries/client_binary.h"
static_assert(sizeof(client_bin) >= sizeof(IMAGE_DOS_HEADER), "Injector binary headers was not implemented. Refer: https://github.com/rogueeeee/patchii2#Building-Injector");

#define return_as_code(returncode) return static_cast<int>(inj_ret##::##returncode);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{	
	if (!pe_validate_dosheader(client_bin))
		return_as_code(INVALID_DOS_HEADER);

	DWORD proc_id = 0;
	
	#ifdef _DEBUG // Prevents std::stoi from overriding a custom hardcoded proc_id only in debug mode
	if (!proc_id)
	#endif
	try
	{
		proc_id = std::stoi(lpCmdLine);
	}
	catch (std::exception ex)
	{
		MessageBoxW(nullptr, (std::wstring(L"patchii injector\n\nPROC_ID_STOI_EXCEPTION\n\n") + lpCmdLine).c_str(), L"", MB_ICONERROR);
		return_as_code(PROC_ID_STOI_EXCEPTION);
	}
	
	if (!proc_id)
		return_as_code(INVALID_PROC_ID);
	
	HANDLE proc_open = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	if (!proc_open)
		return_as_code(FAILED_TO_OPEN_PROCESS);
	
	std::wstring dll_path;
	do
	{
		dll_path = std::filesystem::temp_directory_path().c_str() + random_wstring() + L".dll";
	} while(std::filesystem::exists(dll_path));

	std::ofstream dll_write(dll_path, std::ios::binary | std::ios::out);
	if (!dll_write.is_open())
		return_as_code(FAILED_TO_OPEN_FILE);

	dll_write.write(reinterpret_cast<const char *>(client_bin), sizeof(client_bin));
	dll_write.close();
	
	if (!remote_LoadLibraryW(proc_open, dll_path))
		return_as_code(FAILED_TO_REMOTE_INJECT);

	return_as_code(SUCCESSFUL);
}