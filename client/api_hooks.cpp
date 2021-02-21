#include "api_hooks.h"
#include <Windows.h>
#include <intrin.h>
#include <unordered_map>
#include <vector>
#include <console.h>
#include <MinHook.h>
#include <winternal.h>
#include <client/globals.h>

static std::unordered_map<const char *, std::pair<void*, std::vector<void*>>> api_hooks; // api_hooks[function name] | first = original | second = callbacks
static void *end_of_dll   = nullptr;
static void *start_of_dll = nullptr;

// Checks if the call is from our client
#define filter_api_calls() (_ReturnAddress() >= start_of_dll && _ReturnAddress() <= end_of_dll)

static decltype(MessageBoxA) *o_MessageBoxA = nullptr;
int __stdcall hk_MessageBoxA(HWND hwnd, LPCSTR lptext, LPCSTR lpcaption, UINT utype)
{
	if (filter_api_calls())
		return o_MessageBoxA(hwnd, lptext, lpcaption, utype);

	using callback_t = void(*)(api_hook_event &e, HWND &hwnd, LPCSTR &lptext, LPCSTR &lpcaption, UINT &utype);
	static std::vector<void*> &callbacks = api_hooks["MessageBoxA"].second;

	api_hook_event e;

	for (auto callback : callbacks)
	{
		reinterpret_cast<callback_t>(callback)(e, hwnd, lptext, lpcaption, utype);

		if (e.flags & api_hook_flags::END)
			break;
	}

	int result = 0;
	if (~e.flags & api_hook_flags::DONT_CALL_ORIGINAL)
		result = o_MessageBoxA(hwnd, lptext, lpcaption, utype);

	return (e.flags & api_hook_flags::USE_EVENT_RETURN) ? e.ret_val.i32 : result;
}

#define m_create_apihook_helper(mod, function) create_apihook_helper(mod, #function, &hk_##function, reinterpret_cast<void **>(&o_##function))
void create_apihook_helper(const wchar_t *mod, const char *proc, void *hk, void **original)
{
	console::status_print stat_hooking(std::string("Hooking: ") + proc);
	if (void *target_api; MH_CreateHookApiEx(mod, proc, hk, original, &target_api) == MH_OK)
	{
		api_hooks[proc].first  = target_api;
		api_hooks[proc].second = {};
		stat_hooking.ok();
	}
	else
	{
		stat_hooking.fail();
	}
}

bool patchii_apihooks_enable()
{
	m_create_apihook_helper(L"User32.dll", MessageBoxA);
	
	console::status_print stat_commitapi("Committing all API hooks");
	stat_commitapi.autoset(MH_EnableHook(MH_ALL_HOOKS) == MH_OK);

	start_of_dll = globals::dll_base;
	end_of_dll   = globals::dll_base + pe_get_ntheaderptr(globals::dll_handle)->OptionalHeader.SizeOfImage;

	return true;
}

bool patchii_apihooks_disable()
{
	for (auto hooked_api : api_hooks)
	{
		console::status_print stat_hkdisable(std::string("Disabling API Hook: ") + hooked_api.first);
		stat_hkdisable.autoset(MH_DisableHook(hooked_api.second.first) == MH_OK);
	}

	return true;
}

bool patchii_apihooks_register(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;

	return true;
}

bool patchii_apihooks_unregister(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;


	return true;
}
