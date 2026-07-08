#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace rml::memory
{
	[[nodiscard]] std::string demangle(const char* mangled);
	[[nodiscard]] void* loaded_module(std::initializer_list<std::string_view> names);
	[[nodiscard]] void* resolve_export_exact(void* module, const char* mangled_name);
}
