#include "patchii.h"
#include "globals.h"

#include <Windows.h>
#include <console.h>
#include <string_utils.h>
#include <arch.h>
#include <dx9imgui_window.h>
#include <MinHook.h>
#include <patchii_version.h>

#include "api_hooks.h"
#include "modules/modules.h"
#include "bin_header/patchii_img_128x201.h"

enum class e_close_mode
{
	UNLOAD,
	HIDE
};

static bool patchii_about_window_visible = false;
static bool patchii_is_hidden            = false;
static bool patchii_ufocus_cpu_limiter   = false;
static bool patchii_ufocus_no_render     = false;
static int  patchii_close_mode           = static_cast<int>(e_close_mode::HIDE);
static bool patchii_should_limit_cpu     = false;

static std::vector<patchii_module_base *> patchii_modules {};

LRESULT CALLBACK patchii_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
			return TRUE;

		case WM_CLOSE:
		{
			switch (static_cast<e_close_mode>(patchii_close_mode))
			{
				case e_close_mode::UNLOAD:
					dx9imgui_window::get().start_dispose();
					break;
				case e_close_mode::HIDE:
					dx9imgui_window::get().hide();
					patchii_is_hidden = true;
					return TRUE;
			}

			break;
		}

		case WM_ACTIVATE:
		{
			if (patchii_ufocus_cpu_limiter || patchii_ufocus_no_render)
			{
				static HWND wnd_handle = dx9imgui_window::get().get_wnd_handle();
				bool is_focused = GetForegroundWindow() == wnd_handle;

				patchii_should_limit_cpu = !is_focused && patchii_ufocus_cpu_limiter;

				if (patchii_ufocus_no_render)
					dx9imgui_window::get().render_toggle(is_focused);
			}

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}

	return FALSE;
}

void patchii_draw_window_about()
{
	if (ImGui::Begin("About", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize))
	{
		if (static LPDIRECT3DTEXTURE9 patchii_image = nullptr; patchii_image || (patchii_image = dx9imgui_window::get().make_texture_from_memory(patchii_img_bin, sizeof(patchii_img_bin), 128, 201)) )
			ImGui::Image(patchii_image, ImVec2{ 128.f, 201.f });

		ImGui::SameLine();
		ImGui::Text(
			PATCHII_DESCRIPTION "\n"
			"\n"
			"Dear IMGui - ocornut\n"
			"MinHook - TsudaKageyu\n"
			"\n"
		);

		ImGui::Text("Repository:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4{ .67f, .84f, .90f, 1.f }, "https://github.com/rogueeeee/patchii2");
		if (ImGui::IsItemClicked())
			ShellExecuteW(NULL, L"open", L"https://github.com/rogueeeee/patchii2", nullptr, nullptr, SW_SHOW);
	}
	ImGui::End();
}

void patchii_draw_imgui()
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::TextColored(ImVec4{ .69f, .61f, .85f, 1.f }, "patchii");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(PATCHII_DESCRIPTION);

		ImGui::Separator();

		if (ImGui::BeginMenu("Module"))
		{
			for (auto mod : patchii_modules)
			{
				bool is_loaded = mod->is_loaded();
				if (ImGui::BeginMenu( ((is_loaded ? "[O] " : "[X] ") + mod->name).c_str() ))
				{
					if (is_loaded)
					{
						if (ImGui::MenuItem("Unload"))
							mod->unload();
					}
					else
					{
						if (ImGui::MenuItem("Load"))
							mod->load();
					}

					mod->draw_imgui_module_options();
					ImGui::EndMenu();
				}
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			for (auto mod : patchii_modules)
				if (mod->is_loaded())
					mod->draw_imgui_tools();

			ImGui::EndMenu();
		}

		for (auto mod : patchii_modules)
			if (mod->is_loaded())
				mod->draw_imgui_mainmenubar();

		if (ImGui::BeginMenu("Settings"))
		{
			// Close Mode
			if (ImGui::BeginMenu("On Close"))
			{
				ImGui::RadioButton("Unload", &patchii_close_mode, static_cast<int>(e_close_mode::UNLOAD));
				ImGui::RadioButton("Hide (F5 to return)", &patchii_close_mode, static_cast<int>(e_close_mode::HIDE));
				ImGui::EndMenu();
			}

			// Console toggle
			static bool toggle_console = IsWindowVisible(reinterpret_cast<HWND>(console::get_hwnd()));
			if (ImGui::Checkbox("Show console", &toggle_console))
				ShowWindow((HWND)console::get_hwnd(), toggle_console ? SW_SHOW : SW_HIDE);

			// CPU Limiter
			ImGui::Checkbox("Unfocused CPU Limiter", &patchii_ufocus_cpu_limiter);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Limits CPU usage when the window is not focused");

			// No Render
			ImGui::Checkbox("Unfocused No Render", &patchii_ufocus_no_render);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Disables rendering when the window is not focused");

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("About"))
			patchii_about_window_visible ^= true;

		if (ImGui::MenuItem("Unload"))
			dx9imgui_window::get().start_dispose();
	}
	ImGui::EndMainMenuBar();

	if (patchii_about_window_visible)
		patchii_draw_window_about();

	for (auto mod : patchii_modules)
		if (mod->is_loaded())
			mod->draw_imgui();
}

void patchii_update()
{
	if (patchii_is_hidden && GetAsyncKeyState(VK_F5))
	{
		dx9imgui_window::get().show();
		patchii_is_hidden = false;
	}

	for (auto mod : patchii_modules)
		if (mod->is_loaded())
			mod->update();
	
	if (patchii_should_limit_cpu)
		Sleep(100);
}

void patchii_dxreset()
{
	for (auto mod : patchii_modules)
		if (mod->is_loaded())
			mod->dxreset();
}

bool patchii_run()
{
	console::set_color(console::color::LPURPLE);
	std::cout << "\n\npatchii2\n";
	console::set_color(console::color::LYELLOW);
	std::cout << "build: " __DATE__ " " __TIME__ "\n\n";
	console::set_color();

	console::status_print stat_dx9imp("Initializing DirectX 9 imports...");
	if (!dx9imgui_window::get().import_dx9())
	{
		stat_dx9imp.fail();
		console::print_error("Failed to import DirectX 9 libraries. Make sure that DirectX 9 is installed. Refer: https://github.com/rogueeeee/patchii2#usage");
		return false;
	}
	stat_dx9imp.ok();

	std::cout << "\nInitializing patchii...";
	
	console::status_print stat_wincreate("Creating window...");

	HMODULE proc_mod_handle = GetModuleHandleW(NULL);
	std::cout << "\nTarget Module Handle: 0x" << proc_mod_handle;
	if (!proc_mod_handle)
	{
		stat_wincreate.fail();
		return false;
	}
	
	if (!dx9imgui_window::get().initialize_window(patchii_update, patchii_draw_imgui, patchii_dxreset, patchii_wndproc, proc_mod_handle, random_wstring(), 700, 500, WS_OVERLAPPEDWINDOW))
	{
		stat_wincreate.fail();
		return false;
	}
	
	stat_wincreate.ok();
	
	console::status_print stat_mhinit("Initializing MinHook...");
	if (MH_Initialize() != MH_OK)
	{
		stat_mhinit.fail();
		return false;
	}
	stat_mhinit.ok();
	
	std::cout << "\nCreating API hooks...";
	patchii_apihooks_enable();

	std::cout << "\nRunning patchii...";

	std::cout << "\nLoading modules...";
	patchii_get_registered_modules(patchii_modules);

	// Hide the console window at this point as it is not as necessary
	ShowWindow(reinterpret_cast<HWND>(console::get_hwnd()), SW_HIDE);

	dx9imgui_window::get().show();
	dx9imgui_window::get().run();
	
	ShowWindow(reinterpret_cast<HWND>(console::get_hwnd()), SW_SHOW);

	std::cout << "\nUnloading modules...";
	for (auto mod : patchii_modules)
	{
		if (mod->is_loaded())
			mod->unload();

		delete mod;
	}

	patchii_modules.clear();

	std::cout << "\nDisabling API hooks...";
	patchii_apihooks_disable();

	std::cout << "\nShutting down MinHook...";
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	std::cout << "\nPatchii has been unloaded";

	return true;
}
