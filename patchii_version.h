#pragma once

#ifdef _DEBUG
	#define PATCHII_BUILD_CONFIG "Debug"
#else
	#define PATCHII_BUILD_CONFIG "Release"
#endif

#ifdef WIN32
	#define PATCHII_BUILD_PLATFORM "x86"
#else
	#define PATCHII_BUILD_PLATFORM "x64"
#endif

#define PATCHII_DESCRIPTION \
	"anemic internet cafe poking tool\n" \
	"Build Date: " __DATE__ " " __TIME__ "\n" \
	"Build Type: " PATCHII_BUILD_CONFIG "\n" \
	"Build Platform: " PATCHII_BUILD_PLATFORM
	