#include <utils/winternal.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>

#include "injector_return_code.h"
#include "binaries/client_binary.h"

#define return_as_code(returncode) return static_cast<int>(inj_ret##::##returncode);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	static_assert(sizeof(client_bin) >= sizeof(IMAGE_DOS_HEADER), "Binary headers was not implemented. Refer: https://github.com/rogueeeee/patchii2#Building-Injector");
	
	if (!pe_validate_dosheader(client_bin))
		return_as_code(INVALID_DOS_HEADER);

	DWORD client_image_size = pe_get_ntheaderptr(client_bin)->OptionalHeader.SizeOfImage;
	if (!client_image_size)
		return_as_code(INVALID_IMAGE_SIZE);

	DWORD proc_id = 0;
	
	#ifdef _DEBUG
	if (!proc_id)
	#endif
	proc_id = std::stoi(lpCmdLine);
	
	if (!proc_id)
	{
		return_as_code(INVALID_PROC_ID);
	}
	
	HANDLE proc_open = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	if (!proc_open)
		return_as_code(FAILED_TO_OPEN_PROCESS);
	
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
			
			*reinterpret_cast<DWORD*>(client_bin + (current_reloc->VirtualAddress + (entry_array[idx] & 0x0FFF) )) += proc_alloc_address - pe_get_ntheaderptr(client_bin)->OptionalHeader.ImageBase;
		}
	}

	return_as_code(SUCCESSFUL);
}