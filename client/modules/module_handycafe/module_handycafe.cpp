#include "module_handycafe.h"

#include <console.h>
#include <winternal.h>
#include <imgui.h>

#include "spoof_fgquery.h"

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

	std::cout << "\n\t>> Entry struct: 0x" << hnd_mod_entry
			  << "\n\t>> Base address: 0x" << reinterpret_cast<void *>(hnd_mod_entry->dll_base)
		      << "\n\t>>   Image Size: " << hnd_mod_entry->size_of_image << " byte(s)";

	spoof_fgquery_load(hnd_mod_entry);

	loaded = true;
	return true;
}

bool module_handycafe::unload()
{
	spoof_fgquery_unload();

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
		if (ImGui::BeginMenu("Spoof"))
		{
			if (ImGui::MenuItem("Foreground Window Query", nullptr, nullptr, spoof_fgquery_is_loaded()))
				spoof_fgquery_toggle_window();

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void module_handycafe::draw_imgui()
{
	spoof_fgquery_draw_window();
}
