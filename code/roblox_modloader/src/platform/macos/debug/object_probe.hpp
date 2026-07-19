#pragma once

#include <cstddef>
#include <string>

namespace rml::debug
{
	[[nodiscard]] bool safe_read(const void* address, void* out, std::size_t size);
	[[nodiscard]] std::string rtti_name(const void* object);
	void dump_object(const char* label, const void* object, std::size_t span);
}
