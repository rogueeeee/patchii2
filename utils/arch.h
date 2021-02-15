#pragma once

enum class arch
{
	x86,
	x64
};

#ifdef WIN32
	#define ARCH_IS_X86 true
	#define ARCH_IS_X64 false

	#define ARCH_EC arch::x86
	#define ARCH_STR "x86"

	#define ARCH_XEC arch::x64
	#define ARCH_XSTR "x64"
#else
	#define ARCH_IS_X86 false
	#define ARCH_IS_X64 true

	#define ARCH_EC arch::x64
	#define ARCH_STR "x64"

	#define ARCH_XEC arch::x86
	#define ARCH_XSTR "x86"
#endif