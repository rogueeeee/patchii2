#pragma once

#include <Windows.h>
#include <utils/dx9imgui_window.h>

namespace globals
{
	inline HMODULE          dll_handle      = nullptr;
	inline dx9imgui_window *window_instance = nullptr;
}