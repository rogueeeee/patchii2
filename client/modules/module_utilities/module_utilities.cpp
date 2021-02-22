#include "module_utilities.h"
#include <Windows.h>
#include <client/api_hooks.h>
#include <console.h>
#include <imgui.h>

#include "api_intercept/api_int_messageboxa.h"

static bool loaded                      = false;
static bool intercept_msgboxa_available = false;

module_utilities::module_utilities()
	: patchii_module_base("utils")
{
}

bool module_utilities::load()
{
	std::cout << "\nLoading utils module...";

	intercept_msgboxa_available = api_int_messageboxa_load();

	std::cout << "\nutils module has been loaded!";
	loaded = true;
	return true;
}

bool module_utilities::unload()
{
	std::cout << "\nUnloading utils module...";

	api_int_messageboxa_unload();
	intercept_msgboxa_available = false;
	
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
		if (ImGui::MenuItem("TerminateProcess")) {}

		if (intercept_msgboxa_available && ImGui::MenuItem("MessageBoxA"))
			api_int_messageboxa_togglewindow();

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
	api_int_messageboxa_drawwindow();
}
