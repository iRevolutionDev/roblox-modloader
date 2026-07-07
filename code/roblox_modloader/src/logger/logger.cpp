#include "RobloxModLoader/logger/logger.hpp"

#include "RobloxModLoader/common.hpp"
#include "config/config_manager.hpp"
#include "spdlog/async.h"
#include "spdlog/details/fmt_helper.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/formatter.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "utils/directory.hpp"

#include <atomic>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
	struct global_logger_holder
	{
		static std::atomic<std::shared_ptr<spdlog::logger> > logger;
		static std::shared_ptr<spdlog::sinks::stdout_sink_mt> console_sink;
		static std::shared_ptr<spdlog::sinks::daily_file_sink_mt> file_sink;
		static std::shared_ptr<spdlog::sinks::msvc_sink_mt> msvc_sink;
	};

	std::atomic<std::shared_ptr<spdlog::logger> > global_logger_holder::logger;
	std::shared_ptr<spdlog::sinks::stdout_sink_mt> global_logger_holder::console_sink;
	std::shared_ptr<spdlog::sinks::daily_file_sink_mt> global_logger_holder::file_sink;
	std::shared_ptr<spdlog::sinks::msvc_sink_mt> global_logger_holder::msvc_sink;

	namespace ansi
	{
		constexpr auto reset = "\x1b[0m";
		constexpr auto dim = "\x1b[38;5;240m";
		constexpr auto sep = "│";
	}

	const char* level_label(const spdlog::level::level_enum lvl)
	{
		switch (lvl)
		{
		case spdlog::level::trace: return "TRACE";
		case spdlog::level::debug: return "DEBUG";
		case spdlog::level::info: return "INFO ";
		case spdlog::level::warn: return "WARN ";
		case spdlog::level::err: return "ERROR";
		case spdlog::level::critical: return "FATAL";
		default: return "?????";
		}
	}

	const char* level_color(const spdlog::level::level_enum lvl)
	{
		switch (lvl)
		{
		case spdlog::level::trace: return "\x1b[38;5;244m";
		case spdlog::level::debug: return "\x1b[38;5;39m";
		case spdlog::level::info: return "\x1b[38;5;42m";
		case spdlog::level::warn: return "\x1b[1;38;5;220m";
		case spdlog::level::err: return "\x1b[1;38;5;203m";
		case spdlog::level::critical: return "\x1b[1;97;48;5;124m";
		default: return ansi::reset;
		}
	}

	const char* message_color(const spdlog::level::level_enum lvl)
	{
		switch (lvl)
		{
		case spdlog::level::trace: return "\x1b[38;5;244m";
		case spdlog::level::err: return "\x1b[38;5;210m";
		case spdlog::level::critical: return "\x1b[1;38;5;210m";
		default: return ansi::reset;
		}
	}

	std::string source_color(const spdlog::string_view_t name)
	{
		static constexpr int palette[] = {39, 75, 114, 150, 179, 215, 210, 207, 141, 116, 108, 180, 81, 222};
		std::uint32_t hash = 2166136261u;
		for (size_t i = 0; i < name.size(); ++i)
			hash = (hash ^ static_cast<unsigned char>(name.data()[i])) * 16777619u;
		const int code = palette[hash % std::size(palette)];
		return fmt::format("\x1b[1;38;5;{}m", code);
	}

	class rml_console_formatter final : public spdlog::formatter
	{
	public:
		void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override
		{
			namespace fh = spdlog::details::fmt_helper;
			const auto put = [&dest](const spdlog::string_view_t s) {
				spdlog::details::fmt_helper::append_string_view(s, dest);
			};

			const std::time_t t = std::chrono::system_clock::to_time_t(msg.time);
			std::tm tm_buf{};
			localtime_s(&tm_buf, &t);
			const auto secs = std::chrono::time_point_cast<std::chrono::seconds>(msg.time);
			const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(msg.time - secs).count();

			put(ansi::dim);
			fh::pad2(tm_buf.tm_hour, dest);
			dest.push_back(':');
			fh::pad2(tm_buf.tm_min, dest);
			dest.push_back(':');
			fh::pad2(tm_buf.tm_sec, dest);
			dest.push_back('.');
			fh::pad3(static_cast<uint32_t>(ms), dest);
			put(ansi::reset);
			put(" ");
			put(ansi::dim);
			put(ansi::sep);
			put(ansi::reset);
			put(" ");

			put(level_color(msg.level));
			put(level_label(msg.level));
			put(ansi::reset);
			put(" ");
			put(ansi::dim);
			put(ansi::sep);
			put(ansi::reset);
			put(" ");

			put(source_color(msg.logger_name));
			put(msg.logger_name);
			put(ansi::reset);
			put(" ");
			put(ansi::dim);
			put(ansi::sep);
			put(ansi::reset);
			put(" ");

			put(message_color(msg.level));
			put(msg.payload);
			put(ansi::reset);
			put("\n");
		}

		[[nodiscard]] std::unique_ptr<formatter> clone() const override
		{
			return std::make_unique<rml_console_formatter>();
		}
	};

	void ensure_log_directory()
	{
		const auto root = rml::utils::directory::get_module_directory();

		std::filesystem::create_directories(root / "RobloxModLoader" / "logs");
	}

	void init_sinks()
	{
		if (!global_logger_holder::console_sink)
		{
			const auto root = rml::utils::directory::get_module_directory();
			
			const auto log_file = root / "RobloxModLoader" / "logs" / "roblox_modloader.log";

			global_logger_holder::console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
			global_logger_holder::file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file.string(), 0, 0);
			global_logger_holder::msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

			global_logger_holder::console_sink->set_formatter(std::make_unique<rml_console_formatter>());
			global_logger_holder::file_sink->set_pattern("[%H:%M:%S.%e] [%l] [%n] %v");
			global_logger_holder::msvc_sink->set_pattern("[%H:%M:%S.%e] [%l] [%n] %v");
		}
	}

	std::vector<spdlog::sink_ptr> get_shared_sinks()
	{
		init_sinks();
		return {global_logger_holder::console_sink, global_logger_holder::file_sink, global_logger_holder::msvc_sink};
	}
}

std::shared_ptr<spdlog::logger> global_logger()
{
	auto logger = global_logger_holder::logger.load();
	if (!logger)
	{
		logger::init();
		logger = global_logger_holder::logger.load();
	}
	return logger;
}

void logger::open_console()
{
	static bool console_allocated = false;

	if (console_allocated)
	{
		return;
	}

	if (const auto& config = rml::config::get_config_manager().get_core_config(); !config.logging.enable_console)
	{
		return;
	}

	if (AllocConsole())
	{
		console_allocated = true;
		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
		freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
		freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);

		SetConsoleOutputCP(CP_UTF8);

		if (const HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE); std_out != INVALID_HANDLE_VALUE)
		{
			if (DWORD mode = 0; GetConsoleMode(std_out, &mode))
			{
				SetConsoleMode(std_out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
			}
		}
	}
}

void logger::init()
{
	static bool logger_initialized = false;

	if (logger_initialized)
	{
		return;
	}

#if IS_RML
	open_console();
#endif
	ensure_log_directory();

	try
	{
		if (const auto& config = rml::config::get_config_manager().get_core_config(); config.logging.enable_async_logging)
		{
			set_async_mode();
		}
	}
	catch (...)
	{
		set_async_mode();
	}

	const auto sinks = get_shared_sinks();
	const auto new_logger = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());

	spdlog::register_logger(new_logger);

	try
	{
		const auto& config = rml::config::get_config_manager().get_core_config();
		new_logger->set_level(static_cast<spdlog::level::level_enum>(config.logging.level));
		new_logger->flush_on(static_cast<spdlog::level::level_enum>(config.logging.level));
	}
	catch (...)
	{
		new_logger->set_level(spdlog::level::debug);
		new_logger->flush_on(spdlog::level::debug);
	}

	global_logger_holder::logger.store(new_logger);
	logger_initialized = true;
}

void logger::set_async_mode()
{
	spdlog::init_thread_pool(8192, 1);
}

std::string logger::strip_class_prefix(const std::string& name)
{
	if (const std::string prefix = "class "; name.compare(0, prefix.size(), prefix) == 0)
	{
		return name.substr(prefix.size());
	}
	return name;
}

std::shared_ptr<spdlog::logger> logger::get_logger(const std::string& name)
{
	if (auto existing_logger = spdlog::get(name))
	{
		return existing_logger;
	}

	ensure_log_directory();
	const auto sinks = get_shared_sinks();
	auto new_logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

	new_logger->set_level(spdlog::level::debug);
	new_logger->flush_on(spdlog::level::debug);

	spdlog::register_logger(new_logger);

	return new_logger;
}
