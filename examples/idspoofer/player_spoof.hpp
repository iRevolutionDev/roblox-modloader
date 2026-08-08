#pragma once

#include <cstdint>

namespace idspoofer::player
{
	[[nodiscard]] bool install(std::int64_t user_id) noexcept;
	void uninstall() noexcept;
}
