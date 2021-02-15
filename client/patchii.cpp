#include "patchii.h"
#include "globals.h"

#include <Windows.h>
#include <utils/console.h>
#include <utils/string_utils.h>
#include <utils/arch.h>

#include <patchii_version.h>

#include "ui/ui.h"
#include "modules/modules.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

enum class e_close_mode
{
	UNLOAD,
	HIDE
};

static bool         patchii_is_hidden          = false;
static bool         patchii_ufocus_cpu_limiter = true;
static bool         patchii_ufocus_no_render   = false;
static int          patchii_close_mode         = static_cast<int>(e_close_mode::HIDE);

static std::vector<patchii_module *> patchii_modules;

bool patchii_run()
{
	console::set_color(console::color::LPURPLE);
	std::cout << "\n\npatchii2\n";
	console::set_color(console::color::LYELLOW);
	std::cout << "build: " __DATE__ " " __TIME__ "\n\n";
	console::set_color();

	std::cout << "Initializing patchii..."
		"\nHandle: 0x" << globals::dll_handle;

	// Make the instance available globally
	globals::window_instance = &patchii_window;

	std::cout << "\nLoading modules...";
	patchii_modules = patchii_get_registered_modules();

	console::status_print stat_wincreate("Creating window...");

	HMODULE proc_mod_handle = GetModuleHandleW(NULL);
	std::cout << "\nTarget Module Handle: 0x" << proc_mod_handle;
	if (!proc_mod_handle)
	{
		stat_wincreate.fail();
		return false;
	}
	
	if (!patchii_window.initialize(proc_mod_handle, random_wstring(), 700, 500))
	{
		stat_wincreate.fail();
		return false;
	}
	
	stat_wincreate.ok();

	// Hide the console window at this point as it is not as necessary
	ShowWindow(reinterpret_cast<HWND>(console::get_hwnd()), SW_HIDE);

	patchii_window.show();
	patchii_window.run();
	
	std::cout << "\nUnloading modules...";
	for (auto mod : patchii_modules)
	{
		if (mod->is_loaded())
			mod->unload();

		delete mod;
	}

	patchii_modules.clear();

	std::cout << "\nPatchii has been unloaded";

	return true;
}

LRESULT CALLBACK patchii_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return TRUE;

	switch (uMsg)
	{
		case WM_CLOSE:
		{
			switch (static_cast<e_close_mode>(patchii_close_mode))
			{
				case e_close_mode::UNLOAD:
					patchii_window.start_dispose();
					break;
				case e_close_mode::HIDE:
					patchii_window.hide();
					patchii_is_hidden = true;
					return TRUE;
			}

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void patchii_draw_imgui()
{
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::TextColored(ImVec4{ .69f, .61f, .85f, 1.f }, "patchii");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(PATCHII_DESCRIPTION);

		ImGui::Separator();

		if (ImGui::MenuItem("Module"))
		{
			for (auto mod : patchii_modules)
			{
				if (ImGui::BeginMenu(mod->name.c_str()))
				{
					if (ImGui::MenuItem("Load"))
						mod->load();

					if (ImGui::MenuItem("Unload"))
						mod->unload();

					mod->draw_imgui_module_options();

					ImGui::EndMenu();
				}
			}
		}

		if (ImGui::BeginMenu("Tools"))
		{
			for (auto mod : patchii_modules)
				if (mod->is_loaded())
					mod->draw_imgui_tools();

			ImGui::EndMenu();
		}

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
			static bool toggle_console = IsWindowVisible((HWND)console::get_hwnd());
			if (ImGui::Checkbox("Toggle console", &toggle_console))
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
			ui::about.toggle();

		if (ImGui::MenuItem("Unload"))
			patchii_window.start_dispose();

		for (auto mod : patchii_modules)
			if (mod->is_loaded())
				mod->draw_imgui_mainmenubar();

	}
	ImGui::EndMainMenuBar();

	for (auto mod : patchii_modules)
		if (mod->is_loaded())
			mod->draw_imgui();

	for (auto ui_wnd : patchii_imguiwindow::instances)
		if (ui_wnd->visible)
			ui_wnd->imgui_draw_callback();
}

void patchii_update()
{
	if (patchii_is_hidden && GetAsyncKeyState(VK_F5))
	{
		patchii_window.show();
		patchii_is_hidden = false;
	}
	
	for (auto mod : patchii_modules)
		if (mod->is_loaded())
			mod->update();

	if (patchii_ufocus_cpu_limiter || patchii_ufocus_no_render)
	{
		static HWND wnd_handle = patchii_window.get_wnd_handle();
		bool is_focused = GetForegroundWindow() == wnd_handle;

		if (!is_focused && patchii_ufocus_cpu_limiter)
			Sleep(100);

		if (patchii_ufocus_no_render)
			patchii_window.render_toggle(is_focused);
	}
}
