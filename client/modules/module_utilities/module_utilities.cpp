#include "module_utilities.h"
#include <Windows.h>
#include <client/api_hooks.h>
#include <console.h>
#include <imgui.h>

static bool loaded = false;

static bool intercept_msgboxa_window                    = false;
static bool intercept_msgboxa_available                 = false;
static bool intercept_msgboxa_active                    = false;
static bool intercept_msgboxa_return                    = false;
static int  intercept_msgboxa_default_return            = 0;
static bool intercept_msgboxa_nullhwnd                  = false;
static bool intercept_msgboxa_new_lpcap                 = false;
static char intercept_msgboxa_new_lpcap_buff[MAX_PATH]  = { 0 };
static bool intercept_msgboxa_new_lptext                = false;
static char intercept_msgboxa_new_lptext_buff[MAX_PATH] = { 0 };

void __stdcall apicb_MessageBoxA(api_hook_event &e, HWND &hwnd, LPCSTR &lptext, LPCSTR &lpcaption, UINT &utype)
{
	if (!intercept_msgboxa_active)
		return;

	if (intercept_msgboxa_return)
	{
		e.ret_val.i32 = intercept_msgboxa_default_return;
		e.flags = api_hook_flags::END | api_hook_flags::USE_EVENT_RETURN;
		return;
	}

	if (intercept_msgboxa_nullhwnd)
		hwnd = nullptr;

	if (intercept_msgboxa_new_lpcap)
		lpcaption = intercept_msgboxa_new_lpcap_buff;

	if (intercept_msgboxa_new_lptext)
		lptext = intercept_msgboxa_new_lptext_buff;

	e.flags = api_hook_flags::END;
	return;
}

module_utilities::module_utilities()
	: patchii_module_base("utils")
{
}

bool module_utilities::load()
{
	std::cout << "\nLoading utils module...";
	intercept_msgboxa_available = console::status_print("Adding API callback for MessageBoxA").autoset(patchii_apihooks_register("MessageBoxA", &apicb_MessageBoxA));

	std::cout << "\nutils module has been loaded!";
	loaded = true;
	return true;
}

bool module_utilities::unload()
{
	std::cout << "\nUnloading utils module...";

	console::status_print("Removing API callback: MessageBoxA").autoset(patchii_apihooks_unregister("MessageBoxA", &apicb_MessageBoxA));
	intercept_msgboxa_window         = false;
	intercept_msgboxa_available      = false;
	intercept_msgboxa_active         = false;
	intercept_msgboxa_nullhwnd       = false;
	intercept_msgboxa_new_lpcap      = false;
	intercept_msgboxa_new_lptext     = false;
	intercept_msgboxa_return         = false;
	intercept_msgboxa_default_return = 0;
	ZeroMemory(intercept_msgboxa_new_lpcap_buff, MAX_PATH);
	ZeroMemory(intercept_msgboxa_new_lptext_buff, MAX_PATH);

	std::cout << "\nutils module has been unloaded!";
	loaded = false;
	return false;
}

bool module_utilities::is_loaded()
{
	return loaded;
}

void module_utilities::draw_imgui_tools()
{
	if (ImGui::BeginMenu("API Intercept (utils)"))
	{
		if (ImGui::MenuItem("TerminateProcess Intercept")) {}

		if (intercept_msgboxa_available && ImGui::MenuItem("MessageBoxA Intercept"))
			intercept_msgboxa_window ^= true;

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Thread Manager"))
	{
	}
}

void module_utilities::draw_imgui_module_options()
{
	ImGui::Text("Description");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Contains common utilities that works across targets and platforms");
}

void module_utilities::draw_imgui()
{
	if (intercept_msgboxa_window)
	{
		if (ImGui::Begin("Intercept API: MessageBoxA", &intercept_msgboxa_window, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Checkbox("Intercept", &intercept_msgboxa_active);

			ImGui::NewLine();

			static const char *returnopt[] = { "NULL", "IDOK", "IDCANCEL", "IDABORT", "IDRETRY", "IDIGNORE", "IDYES", "IDNO", "???", "???", "IDTRYAGAIN", "IDCONTINUE" };
			ImGui::Checkbox("Immidiately return", &intercept_msgboxa_return);
			ImGui::SameLine();
			ImGui::Combo("##intercept_msgboxa_default_return", &intercept_msgboxa_default_return, returnopt, IM_ARRAYSIZE(returnopt));

			ImGui::Checkbox("Force no parent handle", &intercept_msgboxa_nullhwnd);

			ImGui::InputText("##intercept_msgboxa_new_lptext_buff", intercept_msgboxa_new_lptext_buff, MAX_PATH);
			ImGui::SameLine();
			ImGui::Checkbox("Change Text", &intercept_msgboxa_new_lptext);

			ImGui::InputText("##intercept_msgboxa_new_lpcap_buff", intercept_msgboxa_new_lpcap_buff, MAX_PATH);
			ImGui::SameLine();
			ImGui::Checkbox("Change Caption", &intercept_msgboxa_new_lpcap);
		}
		ImGui::End();
	}
}
