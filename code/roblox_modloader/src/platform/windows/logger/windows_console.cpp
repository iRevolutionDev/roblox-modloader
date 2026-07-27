#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/logger/platform_console.hpp"
#include "config/config_manager.hpp"

#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/msvc_sink.h"

namespace rml::logger
{
	class ConsoleHandleSink final : public spdlog::sinks::base_sink<std::mutex>
	{
	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			formatter_->format(msg, formatted);

			if (m_handle == INVALID_HANDLE_VALUE)
			{
				acquire_handle();

				if (m_handle == INVALID_HANDLE_VALUE)
				{
					if (m_pending.size() < k_max_pending)
						m_pending.emplace_back(formatted.data(), formatted.size());
					return;
				}

				for (const auto& entry : m_pending)
					write(entry.data(), entry.size());

				m_pending.clear();
				m_pending.shrink_to_fit();
			}

			write(formatted.data(), formatted.size());
		}

		void flush_() override
		{
		}

	private:
		static constexpr std::size_t k_max_pending = 4096;

		void write(const char* data, const std::size_t size) const
		{
			DWORD written = 0;
			WriteFile(m_handle, data, static_cast<DWORD>(size), &written, nullptr);
		}

		void acquire_handle()
		{
			m_handle = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

			if (m_handle == INVALID_HANDLE_VALUE)
				return;

			if (DWORD mode = 0; GetConsoleMode(m_handle, &mode))
				SetConsoleMode(m_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}

		HANDLE m_handle{INVALID_HANDLE_VALUE};
		std::vector<std::string> m_pending;
	};

	class WindowsConsole final : public IPlatformConsole
	{
	public:
		void open() override
		{
			if (m_console_allocated)
				return;

			if (const auto& config = rml::config::get_config_manager().get_core_config(); !config.logging.enable_console)
				return;

			if (!AllocConsole())
				return;

			m_console_allocated = true;

			freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
			freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
			freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

			SetConsoleOutputCP(CP_UTF8);

			if (const HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE); std_out != INVALID_HANDLE_VALUE)
			{
				if (DWORD mode = 0; GetConsoleMode(std_out, &mode))
					SetConsoleMode(std_out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
			}
		}

		std::shared_ptr<spdlog::sinks::sink> make_console_sink() override
		{
			return std::make_shared<ConsoleHandleSink>();
		}

		std::vector<spdlog::sink_ptr> make_diagnostic_sinks() override
		{
			auto debugger_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
			debugger_sink->set_pattern("[%H:%M:%S.%e] [%l] [%n] %v");

			return {std::move(debugger_sink)};
		}

	private:
		bool m_console_allocated = false;
	};

	std::unique_ptr<IPlatformConsole> create_platform_console()
	{
		return std::make_unique<WindowsConsole>();
	}

	std::tm local_time(const std::time_t time)
	{
		std::tm result{};
		localtime_s(&result, &time);
		return result;
	}
}
