#include "RobloxModLoader/logger/platform_console.hpp"

#include "spdlog/sinks/stdout_sinks.h"

namespace rml::logger
{
	class PosixConsole final : public IPlatformConsole
	{
	public:
		void open() override
		{
		}

		std::shared_ptr<spdlog::sinks::sink> make_console_sink() override
		{
			return std::make_shared<spdlog::sinks::stdout_sink_mt>();
		}

		std::vector<spdlog::sink_ptr> make_diagnostic_sinks() override
		{
			return {};
		}
	};

	std::unique_ptr<IPlatformConsole> create_platform_console()
	{
		return std::make_unique<PosixConsole>();
	}

	std::tm local_time(const std::time_t time)
	{
		std::tm result{};
		localtime_r(&time, &result);
		return result;
	}
}
