#pragma once

#ifndef DUMPER_COMMON_HPP
	#define DUMPER_COMMON_HPP

// clang-format off

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <cinttypes>
#include <cstddef>
#include <cstdint>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <atomic>
#include <mutex>
#include <thread>

#include <memory>

#include <sstream>
#include <string>
#include <string_view>

#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <optional>
#include <utility>
#include <variant>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <Psapi.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// clang-format on

#endif // DUMPER_COMMON_HPP
