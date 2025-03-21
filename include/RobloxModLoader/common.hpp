#pragma once

#ifndef COMMON_INC
#define COMMON_INC

// clang-format off

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <windows.h>

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
#include <thread>

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

#include <format>

#include <dwmapi.h>
#include <tchar.h>
#include <uxtheme.h>

#include <tlhelp32.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"

#include "RobloxModLoader/logger/logger.hpp"

// clang-format on

using namespace std::chrono_literals;

inline HINSTANCE g_hinstance{};
inline HANDLE g_main_thread{};
inline std::atomic_bool g_running{false};

#ifndef RML_API
#define RML_API __declspec(dllexport)
#endif

#endif
