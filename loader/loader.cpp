#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <string_utils.h>
#include <dx9imgui_window.h>
#include <patchii_config.h>

#include <injector/injector_return_code.h>
#include "binaries/inj_binaries.h"

static_assert(sizeof(inj_bin_x86) > 1 && sizeof(inj_bin_x64) > 1, "Loader binary headers was not implemented. Refer: https://github.com/rogueeeee/patchii2#building-loader");

using IsWow64Process_t = decltype(IsWow64Process)*;

struct proc_cache_struct
{
	std::string display_name;
	std::string name;
	DWORD id;
	int index;
};

constexpr int wnd_width  = 400;
constexpr int wnd_height = 350;

static int                            selected_proc_idx       = -1;
static bool                           exit_on_load            = true;
static bool                           keep_loadthread_running = true;
static bool                           is_currently_loading    = false;
static int                            load_platform_mode      = 0; // 0 = x86, 1 = x64
static std::wstring                   instance_random_str     = random_wstring();
static std::vector<proc_cache_struct> proc_list_cache {};
static std::string                    status_txt;

static const char *known_target_procnames[] =
{
	"hnclient.exe"
};

bool update_proc_list_cache()
{
	selected_proc_idx = -1;
	proc_list_cache.clear();

	HANDLE hnd = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!hnd)
		return false;
	
	PROCESSENTRY32W proc_entry = { sizeof(PROCESSENTRY32W) };

	int idx = 0;
	while (Process32NextW(hnd, &proc_entry))
	{
		if (static DWORD loader_proc_id = GetCurrentProcessId(); proc_entry.th32ProcessID == loader_proc_id)
			continue;

		std::string mb_proc_name;
		{
			char _mb_proc_name[MAX_PATH] = { '\0' };
			WideCharToMultiByte(CP_UTF8, NULL, proc_entry.szExeFile, -1, _mb_proc_name, MAX_PATH, nullptr, nullptr);
			mb_proc_name = _mb_proc_name;
		}
		
		if (mb_proc_name.substr(mb_proc_name.length() - 4) != ".exe")
			continue;

		proc_list_cache.push_back({
			mb_proc_name + " (" + std::to_string(proc_entry.th32ProcessID) + ")", mb_proc_name, proc_entry.th32ProcessID, idx++
		});
	}

	CloseHandle(hnd);
	return true;
}

void update()
{
	Sleep(1);
}

void draw_imgui()
{
	static auto viewport = []()
	{
		ImGui::SetNextWindowPos(ImVec2{ 0, 0 });
		D3DVIEWPORT9 viewport;
		dx9imgui_window::get().get_device()->GetViewport(&viewport);
		ImGui::SetNextWindowSize(ImVec2{ static_cast<float>(viewport.Width), static_cast<float>(viewport.Height) });

		return viewport;
	}();

	ImGui::Begin("mainwindow", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);

	ImGui::TextColored(ImVec4{ .69f, .61f, .85f, 1.f }, "patchii loader");
	ImGui::SameLine();
	ImGui::Text("| Build: " __DATE__ " " __TIME__);
	
	ImGui::NewLine();

	if (!is_currently_loading)
	{

		ImGui::Text("Process List");

		static char search_buffer[MAX_PATH] = { '\0' };
		ImGui::InputText("", search_buffer, MAX_PATH);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Search for process by name or ID");

		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
			update_proc_list_cache();

		ImGui::SameLine();

		if (ImGui::Button("Auto"))
		{
			for (auto procname : known_target_procnames)
			{
				for (auto proclistitem : proc_list_cache)
				{
					if (proclistitem.name == procname)
					{
						selected_proc_idx = proclistitem.index;
						goto LBL_END_ITERATION;
					}
				}
			}
			LBL_END_ITERATION:;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically select a\n"
			"process based off a predefined\n"
			"list in patchii");

		if (ImGui::BeginListBox("", ImVec2{ static_cast<float>(viewport.Width) - 16.f, 138.f }))
		{
			for (auto proc : proc_list_cache)
			{
				if (search_buffer[0] != '\0' && proc.display_name.find(search_buffer) == std::string::npos)
					continue;

				if (ImGui::Selectable(proc.display_name.c_str(), selected_proc_idx == proc.index))
					selected_proc_idx = proc.index;

				if (selected_proc_idx == proc.index)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}

		ImGui::RadioButton("x86", &load_platform_mode, 0);
		ImGui::SameLine();
		ImGui::RadioButton("x64", &load_platform_mode, 1);

		ImGui::SameLine();
	}

	ImGui::Checkbox("Exit on successful load", &exit_on_load);

	if (!is_currently_loading && selected_proc_idx != -1 && ImGui::Button(("Load to " + proc_list_cache[selected_proc_idx].display_name).c_str()))
		is_currently_loading = true;

	ImGui::Text("%s", status_txt.c_str());

	ImGui::End();
}

LRESULT WINAPI loader_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_CLOSE:
			dx9imgui_window::get().start_dispose();
			return TRUE;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}
	
	return FALSE;
}

void dxreset()
{
}

void loading_thread()
{
	while (keep_loadthread_running)
	{
		if (!is_currently_loading)
		{
			Sleep(1000);
			continue;
		}

		STARTUPINFOW        si {};
		PROCESS_INFORMATION pi {};
		std::wstring        unpack_path;
		std::ofstream       injector_write;
		DWORD               exit_code = -1;

		status_txt = "Creating handle...";
		HANDLE proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, proc_list_cache[selected_proc_idx].id);
		if (!proc_handle)
		{
			status_txt = "Failed to open handle";
			goto LBL_RESET_LOADING_STATE_A;
		}
		
		status_txt = load_platform_mode ? "Unpacking x64 injector..." : "Unpacking x86 injector...";
		unpack_path = std::filesystem::temp_directory_path().c_str() + instance_random_str + L".exe";

		if (!DeleteFileW(unpack_path.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			status_txt = "Cant delete previously unpacked binary";
			goto LBL_RESET_LOADING_STATE_B;
		}

		injector_write = std::ofstream(unpack_path, std::ios::binary | std::ios::out);
		if (!injector_write.is_open())
		{
			status_txt = "Failed to open unpacked binary";
			goto LBL_RESET_LOADING_STATE_B;
		}

		injector_write.write(reinterpret_cast<const char *>(load_platform_mode ? inj_bin_x64 : inj_bin_x86), load_platform_mode ? sizeof(inj_bin_x64) : sizeof(inj_bin_x86));
		injector_write.close();
		
		status_txt = "Starting injector...";
		if (!CreateProcessW(unpack_path.c_str(), (wchar_t *)((instance_random_str + L".exe " + std::to_wstring(proc_list_cache[selected_proc_idx].id)).c_str()), nullptr, nullptr, true, NULL, nullptr, nullptr, &si, &pi))
		{
			status_txt = "Failed to start injector";
			goto LBL_RESET_LOADING_STATE_C;
		}

		status_txt = "Waiting for injector...";

		if (WaitForSingleObject(pi.hProcess, INFINITE) & WAIT_FAILED)
		{
			status_txt = "WaitForSingleObject failed";
			goto LBL_RESET_LOADING_STATE_C;
		}

		if (!GetExitCodeProcess(pi.hProcess, &exit_code))
		{
			status_txt = "Obtaining exit code failed";
			goto LBL_RESET_LOADING_STATE_C;
		}

		#define str_err_code_case(retcode) case inj_ret##::##retcode: \
												status_txt = "Returned: " ## #retcode; \
												break

		switch (static_cast<inj_ret>(exit_code))
		{
			str_err_code_case(INVALID_PROC_ID);
			str_err_code_case(PROC_ID_STOI_EXCEPTION);
			str_err_code_case(FAILED_TO_OPEN_PROCESS);
			str_err_code_case(FAILED_TO_OPEN_FILE);
			str_err_code_case(FAILED_TO_REMOTE_INJECT);

			case inj_ret::SUCCESSFUL:
			{
				status_txt = "Successfully loaded!";
				if (exit_on_load)
					dx9imgui_window::get().start_dispose();
				break;
			}

			default:
				status_txt = "Unhandled exit code";
				break;
		}

		LBL_RESET_LOADING_STATE_C:
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		LBL_RESET_LOADING_STATE_B:
		CloseHandle(proc_handle);
		LBL_RESET_LOADING_STATE_A:
		is_currently_loading = false;
	}
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	if (!dx9imgui_window::get().import_dx9())
	{
		if (MessageBoxW(NULL, L"patchii loader\n\nFailed to import DirectX 9 libraries. Make sure that DirectX 9 is installed.\n\nRefer: https://github.com/rogueeeee/patchii2#usage \nOpen link?", L"", MB_YESNO | MB_ICONERROR) == IDYES)
			ShellExecuteW(NULL, L"open", L"https://github.com/rogueeeee/patchii2#usage", nullptr, nullptr, SW_SHOW);

		return 1;
	}

	if (!dx9imgui_window::get().initialize_window(update, draw_imgui, dxreset, loader_wndproc, hInstance, random_wstring(), wnd_width, wnd_height))
	{
		MessageBoxW(NULL, L"patchii loader\n\n", L"Failed to initialize window", MB_ICONERROR);
		return 2;
	}
		
	std::thread loading_thread_hnd(loading_thread);
	update_proc_list_cache();
	dx9imgui_window::get().show();
	status_txt = "Ready!";
	dx9imgui_window::get().run();
	keep_loadthread_running = false;
	loading_thread_hnd.join();
	
	return 0;
}