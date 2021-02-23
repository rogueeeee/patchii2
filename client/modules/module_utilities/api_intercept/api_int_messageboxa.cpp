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
		std::cout << "\nAPI Intercept MessageBoxA:"
			"\n\tReturn address: " << e.return_address <<
			"\n\t Window handle: " << hwnd <<
			"\n\t          Type: " << utype <<
			"\n\t       Caption: " << lpcaption <<
			"\n\t          Text: " << lptext;
	}

	if (immediately_return)
	{
		e.ret_val.i32 = default_return_value;
		e.flags = api_hook_flags::END | api_hook_flags::USE_EVENT_RETURN;
		return;
	}

	if (nullhwnd_enabled)
		hwnd = nullptr;

	if (change_caption)
		lpcaption = caption_buffer;

	if (change_text)
		lptext = text_buffer;

	e.flags = api_hook_flags::END;
	return;
}

bool api_int_messageboxa_load()
{
    return console::status_print("Adding API callback for MessageBoxA").autoset(patchii_apihooks_register("MessageBoxA", &apicb_MessageBoxA));
}

bool api_int_messageboxa_unload()
{
	console::status_print("Removing API callback: MessageBoxA").autoset(patchii_apihooks_unregister("MessageBoxA", &apicb_MessageBoxA));
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

void api_int_messageboxa_togglewindow()
{
	window_visible = !window_visible;
}

void api_int_messageboxa_drawwindow()
{
	if (window_visible)
	{
		if (ImGui::Begin("Intercept API: MessageBoxA", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize))
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
}
