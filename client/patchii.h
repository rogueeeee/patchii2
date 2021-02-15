#pragma once

#include <utils/dx9imgui_window.h>

bool patchii_run();

LRESULT CALLBACK patchii_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void patchii_draw_imgui();
void patchii_update();

inline dx9imgui_window patchii_window = dx9imgui_window(patchii_update, patchii_draw_imgui, patchii_wndproc);