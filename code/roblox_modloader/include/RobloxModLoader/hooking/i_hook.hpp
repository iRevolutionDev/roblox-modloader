#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <system_error>

#include <MinHook.h>

namespace rml
{
	struct HookError
	{
		std::string context;
		std::uintptr_t address{};
		MH_STATUS minhook_status{MH_UNKNOWN};
		std::error_code system_error{};

		[[nodiscard]] std::string describe() const;

		[[nodiscard]] static HookError from_minhook(std::string context, std::uintptr_t address, MH_STATUS status);

		[[nodiscard]] static HookError from_system_error(std::string context, std::uintptr_t address, std::error_code error);
	};

	class IHook
	{
	public:
		IHook() = default;

		virtual ~IHook() = default;

		IHook(const IHook&) = delete;

		IHook& operator=(const IHook&) = delete;

		IHook(IHook&&) = delete;

		IHook& operator=(IHook&&) = delete;

		[[nodiscard]] virtual std::expected<void, HookError> enable() = 0;

		[[nodiscard]] virtual std::expected<void, HookError> disable() = 0;

		[[nodiscard]] virtual bool is_enabled() const = 0;
	};
}
