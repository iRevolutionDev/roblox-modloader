#pragma once

#ifndef COMMON_INC
	#define COMMON_INC

// clang-format off

#if defined(_WIN32)
#define RML_WINDOWS
#elif defined(__linux__)
#define RML_LINUX
#elif defined(__APPLE__)
#define RML_MACOS
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#if defined(_WIN32)
	#include <winsock2.h>
	#include <Windows.h>
#else
	#include <dlfcn.h>
	#include <limits.h>
	#include <unistd.h>
#  if defined(__linux__)
#    include <link.h>
#    include <elf.h>
#  elif defined(__APPLE__)
#    include <mach-o/dyld.h>
#    include <mach-o/loader.h>
#  endif
#endif

#include <cinttypes>
#include <cstddef>
#include <cstdint>

#include <chrono>
#include <ctime>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <limits>
#include <thread>
#include <ranges>
#include <expected>
#include <span>
#include <queue>

#include <memory>
#include <new>

#include <sstream>
#include <string>
#include <string_view>

#include <algorithm>
#include <functional>
#include <utility>

#include <set>
#include <unordered_set>
#include <stack>
#include <vector>

#include <typeinfo>
#include <type_traits>

#include <exception>
#include <stdexcept>

#include <any>
#include <optional>
#include <variant>
#include <array>
#include <concepts>

#include <format>
#include <regex>
#include <stop_token>
#include <cstdio>


// Platform specific GUI / debugging / process APIs
#if defined(_WIN32)
#  include <dwmapi.h>
#  include <tchar.h>
#  include <uxtheme.h>

#  include <dbghelp.h>
#  include <Psapi.h>

#  include <tlhelp32.h>
#endif

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"

#include <toml++/toml.hpp>

#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/config/config.hpp"

// Luau
#include <lua.h>
#include <lualib.h>
#include <luau/Compiler.h>
#include <luau/CodeGen.h>

#include "rml_export.hpp"

// clang-format on

using namespace std::chrono_literals;

// Cross-platform placeholders for instance/handle on non-Windows
#if defined(_WIN32)
inline HINSTANCE g_hinstance{};
inline HANDLE g_main_thread{};
#else
using HINSTANCE = void*;
using HANDLE = void*;
inline HINSTANCE g_hinstance{nullptr};
inline HANDLE g_main_thread{nullptr};
#endif
inline std::atomic_bool g_running{false};

#endif
