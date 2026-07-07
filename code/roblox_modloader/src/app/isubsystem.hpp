#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml
{
	struct SubsystemError
	{
		std::string subsystem_name;
		std::string message;
	};

	class ISubsystem
	{
	public:
		ISubsystem() = default;

		virtual ~ISubsystem() = default;

		ISubsystem(const ISubsystem&) = delete;
		ISubsystem& operator=(const ISubsystem&) = delete;
		ISubsystem(ISubsystem&&) = delete;
		ISubsystem& operator=(ISubsystem&&) = delete;

		[[nodiscard]] virtual std::expected<void, SubsystemError> initialize() = 0;
		virtual void shutdown() = 0;
		[[nodiscard]] virtual std::string_view name() const noexcept = 0;
	};
}
