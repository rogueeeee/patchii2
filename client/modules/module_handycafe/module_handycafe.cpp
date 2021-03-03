#include "module_handycafe.h"

#include <console.h>
#include <winternal.h>
#include <imgui.h>

#include "spoof_fgwtitle_query.h"

static bool loaded = false;
static ldr_data_table_entry *hnd_mod_entry = nullptr;

module_handycafe::module_handycafe()
	: patchii_module_base("handycafe")
{
}

bool module_handycafe::load()
{
	if (!console::status_print("Obtaining hndclient module from data table entries...").autoset(ldr_data_table_entry_find(L"hndclient.exe", hnd_mod_entry)))
		return false;

	spoof_fgwtitle_query_load(hnd_mod_entry);

	loaded = true;
	return true;
}

bool module_handycafe::unload()
{
	spoof_fgwtitle_query_unload();

	hnd_mod_entry = nullptr;
	loaded = false;
	return true;
}

bool module_handycafe::is_loaded()
{
	return loaded;
}

void module_handycafe::draw_imgui_mainmenubar()
{
	if (ImGui::BeginMenu("handycafe"))
	{
		if (ImGui::MenuItem("Spoof foreground window title", nullptr, nullptr, spoof_fgwtitle_query_is_loaded()))
			spoof_fgwtitle_query_toggle_window();

		ImGui::EndMenu();
	}
}

void module_handycafe::draw_imgui()
{
	spoof_fgwtitle_query_draw_window();
}
