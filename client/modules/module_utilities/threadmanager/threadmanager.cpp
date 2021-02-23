#include "threadmanager.h"
#include <imgui.h>

static bool window_visible = false;

bool threadmanager_load()
{
	return true;
}

bool threadmanager_unload()
{
	window_visible = false;

	return true;
}

void threadmanager_toggle()
{
	window_visible = !window_visible;
}

void threadmanager_draw_window()
{
	if (!window_visible)
		return;

	if (ImGui::Begin("Thread Manager", &window_visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse))
	{

	}
	ImGui::End();
}
