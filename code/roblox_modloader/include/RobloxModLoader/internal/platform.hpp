#pragma once

#if defined(_WIN32)
	#define RML_WINDOWS 1
#elif defined(__APPLE__)
	#define RML_MACOS 1
#elif defined(__linux__)
	#define RML_LINUX 1
#else
	#error "RobloxModLoader: unsupported target platform"
#endif

#define RML_PLATFORM_DETECTED 1
