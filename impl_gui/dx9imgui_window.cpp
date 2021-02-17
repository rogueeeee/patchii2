#include "dx9imgui_window.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return TRUE;

	if (msg == WM_SIZE && wparam != SIZE_MINIMIZED && dx9imgui_window::get().get_device())
	{
		dx9imgui_window::get().present_params.BackBufferWidth  = LOWORD(lparam);
		dx9imgui_window::get().present_params.BackBufferHeight = HIWORD(lparam);
		dx9imgui_window::get().reset();
	}

	if (dx9imgui_window::get().wndproc_callback(hwnd, msg, wparam, lparam))
		return TRUE;

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

dx9imgui_window::dx9imgui_window()
{
}

dx9imgui_window::~dx9imgui_window()
{
}

bool dx9imgui_window::import_dx9()
{
	HMODULE d3d9 = LoadLibraryW(L"d3d9.dll");
	if (!d3d9)
		return false;

	HMODULE d3dx9_43 = LoadLibraryW(L"d3dx9_43.dll");
	if (!d3dx9_43)
		return false;

	this->imported_Direct3DCreate9 = reinterpret_cast<decltype(dx9imgui_window::imported_Direct3DCreate9)>(GetProcAddress(d3d9, "Direct3DCreate9"));
	if (!this->imported_Direct3DCreate9)
		return false;

	this->imported_D3DXCreateTextureFromFileInMemoryEx = reinterpret_cast<decltype(dx9imgui_window::imported_D3DXCreateTextureFromFileInMemoryEx)>(GetProcAddress(d3dx9_43, "D3DXCreateTextureFromFileInMemoryEx"));
	if (!this->imported_D3DXCreateTextureFromFileInMemoryEx)
		return false;

	return true;
}

bool dx9imgui_window::initialize_window(update_fn update_callback_, imgui_draw_callback_fn imgui_draw_callback_, dxreset_callback_fn dxreset_callback_, WNDPROC wndproc_callback_, void *instance_, std::wstring_view class_name_, UINT width, UINT height, DWORD style_, D3DCOLOR clear_color_)
{
	
	this->update_callback = update_callback_;
	this->imgui_draw_callback = imgui_draw_callback_;
	this->dxreset_callback = dxreset_callback_;
	this->wndproc_callback = wndproc_callback_;

	this->instance   = reinterpret_cast<HINSTANCE>(instance_);
	this->class_name = class_name_;
	this->clear_color = clear_color_;

	WNDCLASSEXW wcxw =
	{
		sizeof(WNDCLASSEXW),
		CS_CLASSDC,
		wndproc,
		0,
		0,
		this->instance,
		NULL,
		NULL,
		NULL,
		NULL,
		this->class_name.data(),
		NULL
	};

	if (!RegisterClassExW(&wcxw))
		return false;

	this->window = CreateWindowExW(
		0, this->class_name.c_str(), L"", style_,
		GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2,
		GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2,
		width, height,
		NULL, NULL, this->instance, NULL);

	if (!this->window)
		return false;

	this->dxdirect = this->imported_Direct3DCreate9(D3D_SDK_VERSION);

	if (!this->dxdirect)
		return false;

	this->present_params.Windowed      = true;
	this->present_params.SwapEffect    = D3DSWAPEFFECT_DISCARD;
	this->present_params.hDeviceWindow = this->window;

	if (this->dxdirect->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, this->window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &this->present_params, &this->dxdevice) != D3D_OK)
		return false;

	if (!ImGui::CreateContext() || !ImGui_ImplWin32_Init(this->window) || !ImGui_ImplDX9_Init(this->dxdevice))
		return false;

	ImGui::StyleColorsDark();
	ImGui::GetIO().IniFilename = nullptr;

	this->running = true;

	return true;
}

bool dx9imgui_window::dispose()
{
	if (this->queued_dispose && !this->running)
		return true;

	if (!this->running)
		return false;
	
	this->running = false;

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	
	this->dxdevice->Release();
	this->dxdirect->Release();

	DestroyWindow(this->window);
	UnregisterClassW(this->class_name.c_str(), this->instance);

	return true;
}

bool dx9imgui_window::start_dispose()
{
	this->queued_dispose = true;
	return true;
}

void dx9imgui_window::run()
{
	MSG msg = {};
	bool run_loop = true;
	while (this->running && run_loop)
	{
		while (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if (msg.message == WM_QUIT)
				run_loop = false;
		}

		if (this->should_render)
		{
			this->dxdevice->SetRenderState(D3DRS_ZENABLE, FALSE);
			this->dxdevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			this->dxdevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
			this->dxdevice->Clear(NULL, NULL, D3DCLEAR_TARGET, this->clear_color, 1.f, NULL);

			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			this->imgui_draw_callback();

			ImGui::EndFrame();

			if (this->dxdevice->BeginScene() == D3D_OK)
			{
				ImGui::Render();
				ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				this->dxdevice->EndScene();
			}

			if (this->dxdevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST && this->dxdevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
				this->reset();
		}
		
		this->update_callback();

		if (this->queued_dispose)
			this->dispose();
	}
}

bool dx9imgui_window::is_running()
{
	return this->running;
}

void dx9imgui_window::render_toggle(bool should_render_)
{
	this->should_render = should_render_;
}

LPDIRECT3DTEXTURE9 dx9imgui_window::make_texture_from_memory(void *bin, UINT bin_size, UINT width, UINT height)
{
	LPDIRECT3DTEXTURE9 result = nullptr;

	if (this->imported_D3DXCreateTextureFromFileInMemoryEx(this->dxdevice, bin, bin_size, width, height, D3DX_DEFAULT, D3DUSAGE_DYNAMIC, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &result) != D3D_OK)
	{
		DWORD err = GetLastError();
		return nullptr;
	}

	return result;
}

LPDIRECT3DDEVICE9 dx9imgui_window::get_device()
{
	return this->dxdevice;
}

HWND dx9imgui_window::get_wnd_handle()
{
	return this->window;
}

void dx9imgui_window::show()
{
	ShowWindow(this->window, SW_SHOW);
}

void dx9imgui_window::hide()
{
	ShowWindow(this->window, SW_HIDE);
}

void dx9imgui_window::reset()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	this->dxdevice->Reset(&this->present_params);
	ImGui_ImplDX9_CreateDeviceObjects();
	this->dxreset_callback();
}

dx9imgui_window &dx9imgui_window::get()
{
	return dx9imgui_window::single_instance;
}

dx9imgui_window dx9imgui_window::single_instance;