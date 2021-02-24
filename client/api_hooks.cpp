#include "api_hooks.h"
#include <Windows.h>
#include <intrin.h>
#include <unordered_map>
#include <vector>
#include <console.h>
#include <MinHook.h>
#include <winternal.h>
#include <client/globals.h>

struct api_hook_container_t
{
	union
	{
		void *handle;
		void *target;
		void *fnentry;
	};

	std::vector<void *> callbacks;
	std::vector<void *> callback_add_queue;
	std::vector<void *> callback_remove_queue;

	void update_callbacks()
	{
		if (!callback_add_queue.empty())
		{
			for (auto newcb : callback_add_queue)
				callbacks.push_back(newcb);
			callback_add_queue.clear();
		}

		if (!callback_remove_queue.empty())
		{
			std::vector<void *> new_callbacks;
			for (auto currcb : callbacks)
			{
				for (auto remcb : callback_remove_queue)
					if (remcb == currcb)
						goto LBL_NEXT_ITERATION;

				new_callbacks.push_back(currcb);
				LBL_NEXT_ITERATION:;
			}

			callbacks = new_callbacks;
			callback_remove_queue.clear();
		}
	}
};

static std::unordered_map<const char *, api_hook_container_t> api_hooks;
static void *end_of_dll   = nullptr;
static void *start_of_dll = nullptr;

// Checks if the call is from our client
#define filter_api_calls() (_ReturnAddress() >= start_of_dll && _ReturnAddress() <= end_of_dll)

static decltype(MessageBoxA) *o_MessageBoxA = nullptr;
int __stdcall hk_MessageBoxA(HWND hwnd, LPCSTR lptext, LPCSTR lpcaption, UINT utype)
{
	static api_hook_container_t &container = api_hooks["MessageBoxA"];

	if (filter_api_calls())
		return o_MessageBoxA(hwnd, lptext, lpcaption, utype);
	
	container.update_callbacks();

	api_hook_event e { _ReturnAddress() };
	for (auto callback : container.callbacks)
	{
		reinterpret_cast<void(__stdcall *)(api_hook_event &, HWND &, LPCSTR &, LPCSTR &, UINT &)>(callback)(e, hwnd, lptext, lpcaption, utype);

		if (e.flags & api_hook_flags::END_CALLBACK)
			break;
	}
	
	int result = 0;
	if (~e.flags & api_hook_flags::DONT_CALL_ORIGINAL)
		result = o_MessageBoxA(hwnd, lptext, lpcaption, utype);

	return (e.flags & api_hook_flags::USE_EVENT_RETURN) ? e.ret_val.i32 : result;
}

static decltype(TerminateProcess) *o_TerminateProcess = nullptr;
BOOL __stdcall hk_TerminateProcess(HANDLE hproc, UINT exitcode)
{
	static api_hook_container_t &container = api_hooks["TerminateProcess"];

	if (filter_api_calls())
		o_TerminateProcess(hproc, exitcode);

	container.update_callbacks();

	api_hook_event e{ _ReturnAddress() };
	for (auto callback : container.callbacks)
	{
		reinterpret_cast<void(__stdcall *)(api_hook_event &, HANDLE &, UINT &)>(callback)(e, hproc, exitcode);

		if (e.flags & api_hook_flags::END_CALLBACK)
			break;
	}

	bool result = false;
	if (~e.flags & api_hook_flags::DONT_CALL_ORIGINAL)
		result = o_TerminateProcess(hproc, exitcode);

	return (e.flags & api_hook_flags::USE_EVENT_RETURN) ? e.ret_val.i32 : result;
}

#define m_create_apihook_helper(mod, function) create_apihook_helper(mod, #function, &hk_##function, reinterpret_cast<void **>(&o_##function))
void create_apihook_helper(const wchar_t *mod, const char *proc, void *hk, void **original)
{
	if (void *target_api; console::status_print(std::string("Hooking: ") + proc).autoset(MH_CreateHookApiEx(mod, proc, hk, original, &target_api) == MH_OK))
	{
		api_hooks[proc].handle    = target_api;
		api_hooks[proc].callbacks = {};
	}
}

bool patchii_apihooks_enable()
{
	m_create_apihook_helper(L"User32.dll", MessageBoxA);
	m_create_apihook_helper(L"Kernel32.dll", TerminateProcess);

	console::status_print stat_commitapi("Committing all API hooks");
	stat_commitapi.autoset(MH_EnableHook(MH_ALL_HOOKS) == MH_OK);

	start_of_dll = globals::dll_base;
	end_of_dll   = globals::dll_base + pe_get_ntheaderptr(globals::dll_handle)->OptionalHeader.SizeOfImage;

	return true;
}

bool patchii_apihooks_disable()
{
	for (auto hooked_api : api_hooks)
		console::status_print(std::string("Disabling API Hook: ") + hooked_api.first).autoset(MH_DisableHook(hooked_api.second.handle) == MH_OK);

	return true;
}

bool patchii_apihooks_register(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;

	api_hooks[api_name].callback_add_queue.push_back(callback);
	std::cout << "\nAPI callback registered for " << api_name << " with callback address 0x" << callback;

	return true;
}

bool patchii_apihooks_unregister(const char *api_name, void *callback)
{
	if (api_hooks.find(api_name) == api_hooks.end())
		return false;

	api_hooks[api_name].callback_remove_queue.push_back(callback);
	std::cout << "\nAPI callback unregistered for " << api_name << " with callback address 0x" << callback;

	return true;
}
