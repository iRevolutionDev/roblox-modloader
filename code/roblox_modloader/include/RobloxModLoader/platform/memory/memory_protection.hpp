#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstddef>
#include <expected>
#include <system_error>

namespace rml::utils
{
	enum class MemoryProtection
	{
		ReadWrite,
		ExecuteReadWrite,
	};
}

namespace rml::platform
{
	[[nodiscard]] RML_EXPORT std::expected<unsigned long, std::error_code> set_protection(void* address, std::size_t size, utils::MemoryProtection protection);
	RML_EXPORT void restore_protection(void* address, std::size_t size, unsigned long previous) noexcept;
}
