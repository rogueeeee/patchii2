#include "module_utilities.h"
#include <Windows.h>
#include <client/api_hooks.h>
#include <console.h>
#include <imgui.h>

#include "api_intercept/api_int_messageboxa.h"
#include "threadmanager/threadmanager.h"

static bool loaded                      = false;
static bool threadmanager_available     = false;
static bool intercept_msgboxa_available = false;

module_utilities::module_utilities()
	: patchii_module_base("utils")
{
}

bool module_utilities::load()
{
	std::cout << "\nLoading utils module...";

	threadmanager_available     = threadmanager_load();
	intercept_msgboxa_available = api_int_messageboxa_load();

	std::cout << "\nutils module has been loaded!";
	loaded = true;
	return true;
}

bool module_utilities::unload()
{
	std::cout << "\nUnloading utils module...";

	if (threadmanager_available)
	{
		threadmanager_unload();
		threadmanager_available = false;
	}

	if (intercept_msgboxa_available)
	{
		api_int_messageboxa_unload();
		intercept_msgboxa_available = false;
	}

	std::cout << "\nutils module has been unloaded!";
	loaded = false;
	return true;
}

bool module_utilities::is_loaded()
{
	return loaded;
}

void module_utilities::draw_imgui_tools()
{
	if (ImGui::MenuItem("Thread Manager", nullptr, nullptr, threadmanager_available))
		threadmanager_toggle();
}

void module_utilities::draw_imgui_module_options()
{
	ImGui::Text("Description");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Contains common utilities that works across targets and platforms");
}

void module_utilities::draw_imgui()
{
	threadmanager_draw_window();
	api_int_messageboxa_draw_window();
}

void module_utilities::draw_imgui_mainmenubar()
{
	if (ImGui::BeginMenu("API Intercept"))
	{
		if (ImGui::MenuItem("TerminateProcess")) {}

		if (ImGui::MenuItem("MessageBoxA", nullptr, nullptr, intercept_msgboxa_available))
			api_int_messageboxa_toggle_window();

		ImGui::EndMenu();
	}
}
