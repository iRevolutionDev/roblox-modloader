#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <string>

#define LOGGER_NAME "RML"

#define LOG_INFO(...) SPDLOG_LOGGER_INFO(global_logger(), __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(global_logger(), __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(global_logger(), __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(global_logger(), __VA_ARGS__)
#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(global_logger(), __VA_ARGS__)

#define RML_LOG_SCOPE(scope_name)                                                                        \
	namespace                                                                                            \
	{                                                                                                    \
		inline const std::shared_ptr<spdlog::logger>& rml_scoped_logger()                                \
		{                                                                                                \
			static const std::shared_ptr<spdlog::logger> instance = rml::Logger::get_logger(scope_name); \
			return instance;                                                                             \
		}                                                                                                \
	}

#define RML_INFO(...) SPDLOG_LOGGER_INFO(rml_scoped_logger(), __VA_ARGS__)
#define RML_WARN(...) SPDLOG_LOGGER_WARN(rml_scoped_logger(), __VA_ARGS__)
#define RML_ERROR(...) SPDLOG_LOGGER_ERROR(rml_scoped_logger(), __VA_ARGS__)
#define RML_DEBUG(...) SPDLOG_LOGGER_DEBUG(rml_scoped_logger(), __VA_ARGS__)
#define RML_TRACE(...) SPDLOG_LOGGER_TRACE(rml_scoped_logger(), __VA_ARGS__)

#define RML_INFO_AT(scope, ...) SPDLOG_LOGGER_INFO(rml::Logger::get_logger(scope), __VA_ARGS__)
#define RML_WARN_AT(scope, ...) SPDLOG_LOGGER_WARN(rml::Logger::get_logger(scope), __VA_ARGS__)
#define RML_ERROR_AT(scope, ...) SPDLOG_LOGGER_ERROR(rml::Logger::get_logger(scope), __VA_ARGS__)
#define RML_DEBUG_AT(scope, ...) SPDLOG_LOGGER_DEBUG(rml::Logger::get_logger(scope), __VA_ARGS__)
#define RML_TRACE_AT(scope, ...) SPDLOG_LOGGER_TRACE(rml::Logger::get_logger(scope), __VA_ARGS__)

namespace rml
{
	class Logger
	{
	public:
		static void open_console();

		static void init();

		RML_EXPORT static std::shared_ptr<spdlog::logger> get_logger(const std::string& name);

		RML_EXPORT static std::string strip_class_prefix(const std::string& name);

		RML_EXPORT static void set_async_mode();
	};
}

RML_EXPORT std::shared_ptr<spdlog::logger> global_logger();
