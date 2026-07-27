#pragma once

#include "RobloxModLoader/internal/platform.hpp"

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#if defined(RML_WINDOWS)
	#include <Windows.h>
#endif

#include <atomic>

#if defined(RML_WINDOWS)
inline HINSTANCE g_hinstance{};
inline HANDLE g_main_thread{};
#else
using HINSTANCE = void*;
using HANDLE = void*;
inline HINSTANCE g_hinstance{nullptr};
inline HANDLE g_main_thread{nullptr};
#endif
inline std::atomic_bool g_running{false};
