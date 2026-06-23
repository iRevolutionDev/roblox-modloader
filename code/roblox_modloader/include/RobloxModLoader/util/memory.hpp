#pragma once

#include <cstdint>

namespace rml::utils::memory
{
	[[nodiscard]] inline bool is_valid_pointer(const uintptr_t ptr) noexcept
	{
		return ptr >= 0x10000 && ptr < 0x7FFFFFFFFFFFull && (ptr & 0x7) == 0;
	}
}
