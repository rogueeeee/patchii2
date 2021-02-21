#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>
#include <string>
#include <vector>

class dx9imgui_window
{
	using imgui_draw_callback_fn   = void(*)(void);
	using update_fn                = void(*)(void);
	using dxreset_callback_fn        = void(*)(void);

public:
	dx9imgui_window();
	~dx9imgui_window();

public:
	
	bool import_dx9();

	bool initialize_window(
		update_fn update_callback_, imgui_draw_callback_fn imgui_draw_callback_, dxreset_callback_fn dxreset_callback_, WNDPROC wndproc_callback_,
		void *instance, std::wstring_view classname_, UINT width, UINT height,
		DWORD style_ = (WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX), D3DCOLOR clear_color_ = D3DCOLOR_ARGB(0, 60, 60, 60)
	);

	bool dispose();
	bool start_dispose();

	void run();
	bool is_running();

	void render_toggle(bool should_render_);
	
	LPDIRECT3DTEXTURE9 make_texture_from_memory(void *bin, UINT bin_size, UINT width, UINT height);
	LPDIRECT3DDEVICE9  get_device();
	HWND               get_wnd_handle();

	void show();
	void hide();

	void reset();

private:
	bool     queued_dispose = false;
	bool     running        = false;
	bool     should_render  = true;
	D3DCOLOR clear_color;

private:
	decltype(Direct3DCreate9)                     *imported_Direct3DCreate9                     = nullptr;
	decltype(D3DXCreateTextureFromFileInMemoryEx) *imported_D3DXCreateTextureFromFileInMemoryEx = nullptr;
	static LRESULT WINAPI wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	update_fn              update_callback;
	imgui_draw_callback_fn imgui_draw_callback;
	dxreset_callback_fn    dxreset_callback;
	WNDPROC                wndproc_callback;

private:
	D3DPRESENT_PARAMETERS present_params = {};
	LPDIRECT3D9           dxdirect   = nullptr;
	LPDIRECT3DDEVICE9     dxdevice   = nullptr;
	HWND                  window     = nullptr;
	HINSTANCE             instance   = nullptr;
	std::wstring          class_name;

public:
	static dx9imgui_window &get();

private:
	static dx9imgui_window single_instance;
};