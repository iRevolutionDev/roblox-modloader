#pragma once

#include <string>
#include <string_view>

namespace rml::memory
{
	[[nodiscard]] std::string demangle(const char* mangled);
	[[nodiscard]] std::string demangle_signature(const char* mangled);
	[[nodiscard]] std::string normalize_signature(std::string_view demangled);
}
