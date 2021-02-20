#pragma once

#if __has_include("inj_binary_x64.h")
	#include "inj_binary_x64.h"
#else
	unsigned char inj_bin_x64[] = { 0x00 };
#endif

#if __has_include("inj_binary_x86.h")
	#include "inj_binary_x86.h"
#else
	unsigned char inj_bin_x86[] = { 0x00 };
#endif