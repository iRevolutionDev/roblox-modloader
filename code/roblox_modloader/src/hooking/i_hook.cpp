#include "RobloxModLoader/hooking/i_hook.hpp"

#include <format>

namespace rml
{
	std::string HookError::describe() const
	{
		if (system_error)
			return std::format("{} at 0x{:X}: {}", context, address, system_error.message());

		return std::format("{} at 0x{:X}: {}", context, address, status_message);
	}

	HookError HookError::from_status(std::string context, const std::uintptr_t address, std::string status_message)
	{
		return HookError{.context = std::move(context), .address = address, .status_message = std::move(status_message)};
	}

	HookError HookError::from_system_error(std::string context, const std::uintptr_t address, std::error_code error)
	{
		return HookError{.context = std::move(context), .address = address, .system_error = std::move(error)};
	}
}
