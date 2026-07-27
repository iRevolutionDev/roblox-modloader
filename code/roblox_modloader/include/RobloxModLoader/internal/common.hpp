#pragma once

#ifndef COMMON_INC
	#define COMMON_INC

// clang-format off

#include "RobloxModLoader/internal/platform.hpp"

#if defined(RML_WINDOWS)
    #include "RobloxModLoader/platform/pch/windows_pch.hpp"
#elif defined(RML_MACOS)
    #include "RobloxModLoader/platform/pch/macos_pch.hpp"
#elif defined(RML_LINUX)
    #include "RobloxModLoader/platform/pch/linux_pch.hpp"
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


#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"

#include <toml++/toml.hpp>

#include "RobloxModLoader/logger/logger.hpp"

#include <lua.h>
#include <lualib.h>

#include "RobloxModLoader/rml_export.hpp"

using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

// clang-format on

#include "RobloxModLoader/internal/loader_state.hpp"

#endif
