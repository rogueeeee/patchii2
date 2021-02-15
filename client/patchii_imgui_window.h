#pragma once

#include <vector>
#include <third_party/dearimgui/imgui.h>

class patchii_imguiwindow
{
	using callback_fn = void(void);

public:
	patchii_imguiwindow(callback_fn *callback_);

	void toggle();

public:
	callback_fn *imgui_draw_callback = nullptr;
	bool visible = false;

public:
	inline static std::vector<patchii_imguiwindow *> instances;
};