#pragma once

// =================================
// Internal Segment - DO NOT MODIFY
// =================================

#if defined( PATCHII_USE_MANUALMAP ) || __has_include("compileflag_manualmap.h") || ( defined( NDEBUG ) && !defined( PATCHII_USE_LOADLIB ) && !__has_include("compileflag_loadlib.h") )
	#define PATCHII_LOADAS_MANUALMAPPED
#endif

#if defined( PATCHII_USE_LOADLIB ) || __has_include("compileflag_loadlib.h") || ( defined( _DEBUG ) && !defined( PATCHII_USE_MANUALMAP ) && !__has_include("compileflag_manualmap.h") )
	#define PATCHII_LOADAS_LOADLIB
#endif

#if defined( PATCHII_LOADAS_MANUALMAPPED ) && defined( PATCHII_LOADAS_LOADLIB )
	#error "Conflicting loading method!"
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
	#define PATCHII_BUILD_VERSION "2.2.0"
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