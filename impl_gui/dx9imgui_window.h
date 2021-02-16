#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <third_party/dearimgui/imgui.h>
#include <third_party/dearimgui/imgui_impl_win32.h>
#include <third_party/dearimgui/imgui_impl_dx9.h>

#include <string>

class dx9imgui_window
{
	using imgui_draw_callback_fn   = void(*)(void);
	using update_fn                = void(*)(void);

public:
	dx9imgui_window(update_fn update_, imgui_draw_callback_fn imgui_draw_callback_, WNDPROC wndproc_callback_);
	~dx9imgui_window();

public:
	bool initialize(void *instance, std::wstring_view classname_, UINT width, UINT height, DWORD style_ = (WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX));
	bool dispose();
	bool start_dispose();

	void run();
	bool is_running();

	void render_toggle(bool should_render_);
	
	LPDIRECT3DTEXTURE9 make_texture_from_memory(void *bin, UINT bin_size, UINT width, UINT height);
	LPDIRECT3DDEVICE9 get_device();
	HWND              get_wnd_handle();

	void show();
	void hide();

private:
	bool queued_dispose = false;
	bool running        = false;
	bool should_render  = true;

private:
	update_fn              update;
	imgui_draw_callback_fn imgui_draw_callback;
	WNDPROC                wndproc_callback;

private:
	D3DPRESENT_PARAMETERS present_params = {};
	LPDIRECT3D9           dxdirect   = nullptr;
	LPDIRECT3DDEVICE9     dxdevice   = nullptr;
	HWND                  window     = nullptr;
	HINSTANCE             instance   = nullptr;
	std::wstring          class_name;
};