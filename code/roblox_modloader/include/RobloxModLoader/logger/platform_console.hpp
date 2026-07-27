#pragma once

#include <ctime>
#include <memory>
#include <spdlog/sinks/sink.h>
#include <vector>

namespace rml::logger
{
	class IPlatformConsole
	{
	public:
		virtual ~IPlatformConsole() = default;

		virtual void open() = 0;
		[[nodiscard]] virtual std::shared_ptr<spdlog::sinks::sink> make_console_sink() = 0;
		[[nodiscard]] virtual std::vector<spdlog::sink_ptr> make_diagnostic_sinks() = 0;
	};

	[[nodiscard]] std::unique_ptr<IPlatformConsole> create_platform_console();
	[[nodiscard]] std::tm local_time(std::time_t time);
}
