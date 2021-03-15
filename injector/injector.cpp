#include <winternal.h>
#include <string_utils.h>
#include <patchii_config.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <client/load_info.h>
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
		return_as_code(PROC_ID_STOI_EXCEPTION);
	}
	
	if (!proc_id)
		return_as_code(INVALID_PROC_ID);
	
	HANDLE proc_open = OpenProcess(PROCESS_ALL_ACCESS, false, proc_id);
	if (!proc_open)
		return_as_code(FAILED_TO_OPEN_PROCESS);
	
	#if defined( PATCHII_LOADAS_MANUALMAPPED )
	{
		PIMAGE_NT_HEADERS nt_header = pe_get_ntheaderptr(client_bin);
		
		constexpr std::ptrdiff_t sc_offset_entrypoint = 0x6;

		#ifdef _M_IX86
		constexpr std::ptrdiff_t sc_offset_loadinfo   = 0xB;
		std::uint8_t shellcode[] =
		{
			0x55,						   // push   ebp
			0x89, 0xE5,					   // mov    ebp, esp
			0x51,						   // push   ecx
			0x52,						   // push   edx
			0xB9, 0x00, 0x00, 0x00, 0x00,  // mov    ecx, dll_alloc + nt_header->OptionalHeader.AddressOfEntryPoint
			0xBA, 0x00, 0x00, 0x00, 0x00,  // mov    edx, load_info_alloc
			0x52,						   // push   edx
			0x6A, 0x01,					   // push   0x1
			0x6A, 0x00,					   // push   0x0
			0xFF, 0xD1,					   // call   ecx
			0x31, 0xC0,					   // xor    eax, eax
			0x5A,						   // pop    edx
			0x59,						   // pop    ecx
			0x5D,						   // pop    ebp
			0xC2, 0x00, 0x00			   // ret    0x0
		};
		#elif _M_X64
		constexpr std::ptrdiff_t sc_offset_loadinfo   = 0x10;
		std::uint8_t shellcode[] =
		{
			0x48, 0x83, 0xEC, 0x28,										// sub    rsp, 0x28
			0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// movabs rax, 0x0000000000000000
			0x49, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// movabs r8,  0x0000000000000000
			0xBA, 0x01, 0x00, 0x00, 0x00,								// mov    edx, 0x1
			0x31, 0xC9,													// xor    ecx, ecx
			0xFF, 0xD0,													// call   rax
			0x48, 0x83, 0xC4, 0x28,										// add    rsp, 0x28
			0xC2, 0x00, 0x00											// ret    0x0
		};
		#endif

		std::uint8_t *alloc_address = reinterpret_cast<std::uint8_t *>(VirtualAllocEx(proc_open, nullptr, sizeof(shellcode) + sizeof(load_info_t) + nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (!alloc_address)
			return_as_code(FAILED_TO_ALLOCATE);
		
		std::uint8_t          *shellcode_alloc = alloc_address;
		load_info_t           *load_info_alloc = reinterpret_cast<load_info_t *>(alloc_address + sizeof(shellcode));
		std::uint8_t          *dll_alloc       = alloc_address + sizeof(shellcode) + sizeof(load_info_t);
		PIMAGE_SECTION_HEADER  sect_header     = reinterpret_cast<PIMAGE_SECTION_HEADER>(nt_header + 1);
		for (std::size_t idx = 0; idx < nt_header->FileHeader.NumberOfSections; idx++)
		{
			if (!WriteProcessMemory(proc_open, dll_alloc + sect_header[idx].VirtualAddress, client_bin + sect_header[idx].PointerToRawData, sect_header[idx].SizeOfRawData, nullptr))
				return_as_code(FAILED_TO_COPY_SECTION);
		}

		load_info_t loader_info =
		{
			&LoadLibraryA,
			&GetProcAddress,
			dll_alloc,
			reinterpret_cast<std::ptrdiff_t>(dll_alloc - nt_header->OptionalHeader.ImageBase),
			reinterpret_cast<PIMAGE_BASE_RELOCATION>(dll_alloc + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress),
			reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(dll_alloc + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress),
			reinterpret_cast<decltype(load_info_t::DllMain)>(dll_alloc + nt_header->OptionalHeader.AddressOfEntryPoint)
		};

		if (!WriteProcessMemory(proc_open, load_info_alloc, &loader_info, sizeof(load_info_t), nullptr))
			return_as_code(FAILED_TO_WRITE_LOADER_INFO);

		*reinterpret_cast<void **>(shellcode + sc_offset_entrypoint) = loader_info.DllMain;
		*reinterpret_cast<void **>(shellcode + sc_offset_loadinfo)   = load_info_alloc;

		if (!WriteProcessMemory(proc_open, shellcode_alloc, shellcode, sizeof(shellcode), nullptr))
			return_as_code(FAILED_TO_WRITE_SHELLCODE);

		HANDLE shellcode_execute = CreateRemoteThread(proc_open, nullptr, NULL, LPTHREAD_START_ROUTINE(shellcode_alloc), nullptr, NULL, nullptr);
		if (!shellcode_execute)
			return_as_code(FAILED_TO_EXECUTE_SHELLCODE);

		if (WaitForSingleObject(shellcode_execute, INFINITE))
			return_as_code(FAILED_TO_WAIT_SHELLCODE);
	}
	#elif defined( PATCHII_LOADAS_LOADLIB )
	{
		std::wstring dll_path;
		do
		{
			dll_path = std::filesystem::temp_directory_path().c_str() + random_wstring() + L".dll";
		} while (std::filesystem::exists(dll_path));

		std::ofstream dll_write(dll_path, std::ios::binary | std::ios::out);
		if (!dll_write.is_open())
			return_as_code(FAILED_TO_OPEN_FILE);

		dll_write.write(reinterpret_cast<const char *>(client_bin), sizeof(client_bin));
		dll_write.close();

		if (!remote_LoadLibraryW(proc_open, dll_path))
			return_as_code(FAILED_TO_REMOTE_INJECT);
	}
	#endif
	
	return_as_code(SUCCESSFUL);
}