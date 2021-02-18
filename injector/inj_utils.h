#pragma once

#if 0
#include <Windows.h>

HMODULE remote_LoadLibraryA(HANDLE proc_handle, LPCSTR file_name);
FARPROC remote_GetProcAddress(HANDLE proc_handle, HMODULE mod_handle, LPCSTR proc_name);
#endif