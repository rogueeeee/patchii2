#pragma once

#if __has_include("bin_x86.h") && defined(WIN32)
	#include "bin_x86.h"
#elif __has_include("bin_x64.h") && !defined(WIN32)
	#include "bin_x64.h"
#else
	unsigned char client_bin[] = { 0x00 };
#endif