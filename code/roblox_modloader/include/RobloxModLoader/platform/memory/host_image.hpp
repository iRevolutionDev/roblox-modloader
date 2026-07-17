#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace rml::platform
{
	[[nodiscard]] std::filesystem::path module_path_containing(const void* address);
	[[nodiscard]] std::filesystem::path executable_path();
	[[nodiscard]] std::string_view studio_image_name();
	[[nodiscard]] std::uintptr_t studio_preferred_image_base();
	[[nodiscard]] void* acquire_main_window();
}
