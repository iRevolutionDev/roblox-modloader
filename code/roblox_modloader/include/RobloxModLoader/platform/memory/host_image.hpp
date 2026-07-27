#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace rml::platform
{
	[[nodiscard]] RML_EXPORT std::filesystem::path module_path_containing(const void* address);
	[[nodiscard]] RML_EXPORT std::filesystem::path executable_path();
	[[nodiscard]] RML_EXPORT std::string_view studio_image_name();
	[[nodiscard]] RML_EXPORT std::uintptr_t studio_preferred_image_base();
	[[nodiscard]] RML_EXPORT void* acquire_main_window();
}
