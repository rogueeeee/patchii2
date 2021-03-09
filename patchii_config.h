#pragma once

//
// This only applies to release builds.
// All debug builds are loaded with LoadLibrary
//--config:manualmap
#define PATCHII_NOT_MANUALMAPPING
// 

// =================================
// Internal Segment - DO NOT MODIFY
// =================================

#if defined( NDEBUG ) && !defined( PATCHII_NOT_MANUALMAPPING )
	#define PATCHII_LOADAS_MANUALMAPPED
#else
	#define PATCHII_LOADAS_LOADLIB
#endif

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


//
// Build version string
// Reflected by release tags on the main respository
// PATCHII_BUILD_VERSION can be defined for custom builds
#if !defined( PATCHII_BUILD_VERSION )
	#define PATCHII_BUILD_VERSION "2021.03.04.s-1"
#endif
//

#define PATCHII_DESCRIPTION \
	"anemic internet cafe poking tool\n" \
	"Build Version: " PATCHII_BUILD_VERSION "\n" \
	"Build Date: " __DATE__ " " __TIME__ "\n" \
	"Build Type: " PATCHII_BUILD_CONFIG "\n" \
	"Build Platform: " PATCHII_BUILD_PLATFORM

// =================================
// Internal Segment - DO NOT MODIFY
// =================================