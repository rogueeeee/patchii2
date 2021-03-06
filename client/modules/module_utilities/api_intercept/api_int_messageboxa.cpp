#include "api_int_messageboxa.h"

#include <Windows.h>
#include <client/api_hooks.h>
#include <console.h>
#include <imgui.h>

static bool window_visible			 = false;
static bool is_active                = false;
static bool log_to_con               = false;
static bool immediately_return       = false;
static int  default_return_value     = 0;
static bool nullhwnd_enabled         = false;
static bool change_caption           = false;
static char caption_buffer[MAX_PATH] = { 0 };
static bool change_text              = false;
static char text_buffer[MAX_PATH]    = { 0 };

void __stdcall apicb_MessageBoxA(api_hook_event &e, HWND &hwnd, LPCSTR &lptext, LPCSTR &lpcaption, UINT &utype)
{
	if (!is_active)
		return;

	if (log_to_con)
	{
		std::cout << "\nAPI Intercept: MessageBoxA"
			"\n\tReturn address: 0x" << e.return_address <<
			"\n\t Window handle: 0x" << hwnd             <<
			"\n\t          Type: "   << utype            <<
			"\n\t       Caption: "   << lpcaption        <<
			"\n[Text]\n"             << lptext           << "\n[/text]";
	}

	if (immediately_return)
	{
		e.ret_val.i32 = default_return_value;
		e.flags |= api_hook_flags::END_CALLBACK | api_hook_flags::USE_EVENT_RETURN | api_hook_flags::DONT_CALL_ORIGINAL;
		return;
	}

	if (nullhwnd_enabled)
		hwnd = nullptr;
	
	if (change_caption)
		lpcaption = caption_buffer;

	if (change_text)
		lptext = text_buffer;

	e.flags |= api_hook_flags::END_CALLBACK;
	return;
}

bool api_int_messageboxa_load()
{
    return console::status_print("[API Intercept] Adding API callback for MessageBoxA").autoset(patchii_apihooks_register("MessageBoxA", &apicb_MessageBoxA));
}

bool api_int_messageboxa_unload()
{
	if (!console::status_print("[API Intercept] Removing API callback: MessageBoxA").autoset(patchii_apihooks_unregister("MessageBoxA", &apicb_MessageBoxA)))
		return false;

	window_visible       = false;
	is_active            = false;
	log_to_con           = false;
	nullhwnd_enabled     = false;
	change_caption       = false;
	change_text          = false;
	immediately_return   = false;
	default_return_value = 0;
	ZeroMemory(caption_buffer, MAX_PATH);
	ZeroMemory(text_buffer, MAX_PATH);

    return true;
}

void api_int_messageboxa_toggle_window()
{
	window_visible = !window_visible;
}

void api_int_messageboxa_draw_window()
{
	if (!window_visible)
		return;

	if (ImGui::Begin("API Intercept: MessageBoxA", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Checkbox("Active", &is_active);
		ImGui::Checkbox("Log Intercept", &log_to_con);
		ImGui::NewLine();

		static const char *returnopt[] = { "NULL", "IDOK", "IDCANCEL", "IDABORT", "IDRETRY", "IDIGNORE", "IDYES", "IDNO", "???", "???", "IDTRYAGAIN", "IDCONTINUE" };
		ImGui::Checkbox("Immediately return", &immediately_return);
		ImGui::SameLine();
		ImGui::Combo("##default_return_value", &default_return_value, returnopt, IM_ARRAYSIZE(returnopt));

		ImGui::Checkbox("Force no parent handle", &nullhwnd_enabled);

		ImGui::InputText("##text_buffer", text_buffer, MAX_PATH);
		ImGui::SameLine();
		ImGui::Checkbox("Change Text", &change_text);

		ImGui::InputText("##caption_buffer", caption_buffer, MAX_PATH);
		ImGui::SameLine();
		ImGui::Checkbox("Change Caption", &change_caption);
	}
	ImGui::End();
}
