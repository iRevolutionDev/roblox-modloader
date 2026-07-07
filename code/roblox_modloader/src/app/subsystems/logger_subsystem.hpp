#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/logger/logger.hpp"

namespace rml
{
	class LoggerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			Logger::init();
			return {};
		}

		void shutdown() override
		{
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "Logger";
		}
	};
}
