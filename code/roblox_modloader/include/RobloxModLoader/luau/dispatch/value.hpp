#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau
{
	using RefId = std::uint64_t;
	inline constexpr RefId kInvalidRef = 0;

	struct InstanceHandle
	{
		std::uintptr_t address{};
	};

	struct LuauRefHandle
	{
		RefId id{kInvalidRef};
	};

	using Value = std::variant<std::monostate, bool, double, std::string, InstanceHandle, LuauRefHandle>;

	[[nodiscard]] inline bool is_nil(const Value& value) noexcept
	{
		return std::holds_alternative<std::monostate>(value);
	}
}
