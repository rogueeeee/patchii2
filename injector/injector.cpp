#include <utils/winternal.h>
#include <utils/string_utils.h>

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "injector_return_code.h"
#include "binaries/client_binary.h"

#include "inj_utils.h"

#define return_as_code(returncode) return static_cast<int>(inj_ret##::##returncode);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	static_assert(sizeof(client_bin) >= sizeof(IMAGE_DOS_HEADER), "Binary headers was not implemented. Refer: https://github.com/rogueeeee/patchii2#Building-Injector");
	
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

	#if 0 // Manual mapper code, will implement in the future

	DWORD client_image_size = pe_get_ntheaderptr(client_bin)->OptionalHeader.SizeOfImage;
	if (!client_image_size)
		return_as_code(INVALID_IMAGE_SIZE);

	std::uintptr_t proc_alloc_address = reinterpret_cast<std::uintptr_t>(VirtualAllocEx(proc_open, nullptr, client_image_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!proc_alloc_address)
		return_as_code(FAILED_TO_ALLOCATE);
	
	PIMAGE_BASE_RELOCATION current_reloc = nullptr;
	while (pe_image_base_reloc_next(client_bin, current_reloc))
	{
		std::size_t entry_count = (current_reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		WORD *entry_array = reinterpret_cast<WORD *>(client_bin + current_reloc->VirtualAddress);

		for (std::size_t idx = 0; idx < entry_count; idx++)
		{
			DWORD entry_type = entry_array[idx] & 0xF000;
			
			if (entry_type == IMAGE_REL_BASED_ABSOLUTE)
				continue;

			if (entry_type != IMAGE_REL_BASED_HIGHLOW)
				return_as_code(UNHANDLED_RELOCATION_TYPE);
			
			*reinterpret_cast<DWORD*>(client_bin + (static_cast<std::uintptr_t>(current_reloc->VirtualAddress) + (entry_array[idx] & 0x0FFF) )) += proc_alloc_address - pe_get_ntheaderptr(client_bin)->OptionalHeader.ImageBase;
		}
	}

	PIMAGE_IMPORT_DESCRIPTOR current_import_desc = nullptr;
	while (pe_image_import_descriptor_next(client_bin, current_import_desc))
	{
		PIMAGE_THUNK_DATA current_orig  = nullptr;
		PIMAGE_THUNK_DATA current_first = nullptr;

		HMODULE loaded_lib = remote_LoadLibraryA(proc_open, reinterpret_cast<LPCSTR>(client_bin + current_import_desc->Name));
		if (!loaded_lib)
			return_as_code(LOAD_MODULE_DEPENDENCY_FAILED);


		while (pe_image_thunk_data_next(client_bin, current_import_desc, current_orig, current_first))
		{
			if (IMAGE_SNAP_BY_ORDINAL(current_orig->u1.Ordinal))
			{

			}
			else
			{
				
			}
		}
	}

	return_as_code(SUCCESSFUL);

	#endif
}