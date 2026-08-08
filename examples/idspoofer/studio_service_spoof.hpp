#pragma once

#include <cstdint>

namespace idspoofer::studio_service
{
	[[nodiscard]] bool install(std::int64_t user_id) noexcept;
	void uninstall() noexcept;
}
