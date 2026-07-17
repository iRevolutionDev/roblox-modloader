#pragma once

#include <cstddef>
#include <expected>
#include <system_error>

namespace rml::utils
{
	enum class MemoryProtection;
}

namespace rml::platform
{
	[[nodiscard]] std::expected<unsigned long, std::error_code> set_protection(void* address, std::size_t size, utils::MemoryProtection protection);
	void restore_protection(void* address, std::size_t size, unsigned long previous) noexcept;
}
