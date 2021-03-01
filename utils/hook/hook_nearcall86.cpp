#include <utils/hook.h>
#include <Windows.h>
#include <cstddef>
#include <cstdint>

bool hook_nearcall86(void *call_location, void *hook_func, void **original_address)
{
	constexpr std::ptrdiff_t instruction_size = 5;

	if (!call_location || !hook_func)
		return false;

	if (*reinterpret_cast<std::uint8_t *>(call_location) != 0xE8)
		return false;

	std::uintptr_t next_instruction_address = reinterpret_cast<std::uintptr_t>(reinterpret_cast<std::uint8_t *>(call_location) + instruction_size);
	std::uintptr_t *operand                 = reinterpret_cast<std::uintptr_t *>(reinterpret_cast<std::uint8_t *>(call_location) + 0x1);

	if (original_address)
		*original_address = reinterpret_cast<void *>(next_instruction_address + *operand);

	DWORD prot = NULL;
	if (!VirtualProtect(operand, sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &prot))
		return false;

	*operand = next_instruction_address - reinterpret_cast<std::uintptr_t>(hook_func);

	if (!VirtualProtect(operand, sizeof(std::uintptr_t), prot, &prot))
		return false;

	return true;
}