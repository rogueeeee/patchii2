#include "module_threadman.h"

#include <imgui.h>

static bool is_tm_loaded	 = true;
static bool show_thread_menu = false;

module_threadman::module_threadman()
	: patchii_module_base("threadmanager")
{
}

bool module_threadman::load()
{
	is_tm_loaded = true;
	return true;
}

bool module_threadman::unload()
{
	is_tm_loaded = false;
	return true;
}

bool module_threadman::is_loaded()
{
	return is_tm_loaded;
}

void module_threadman::draw_imgui_tools()
{
	if (ImGui::MenuItem("Thread Manager"))
		show_thread_menu ^= true;
}

void module_threadman::draw_imgui()
{
}

void module_threadman::update()
{
}
