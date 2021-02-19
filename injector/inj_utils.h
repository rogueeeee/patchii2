#pragma once
#include <Windows.h>
#include <string>

bool remote_LoadLibraryW(HANDLE proc_handle, std::wstring path);

#if 0
HMODULE remote_LoadLibraryA(HANDLE proc_handle, LPCSTR file_name);
FARPROC remote_GetProcAddress(HANDLE proc_handle, HMODULE mod_handle, LPCSTR proc_name);
#endif