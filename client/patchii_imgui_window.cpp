#include "patchii_imgui_window.h"

patchii_imguiwindow::patchii_imguiwindow(callback_fn *callback_)
	: imgui_draw_callback(callback_)
{
	patchii_imguiwindow::instances.push_back(this);
}

void patchii_imguiwindow::toggle()
{
	this->visible ^= true;
}