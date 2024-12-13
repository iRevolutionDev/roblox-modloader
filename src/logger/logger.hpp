#pragma once

#define LOGGER_NAME "roblox_modloader"

#define LOG_INFO(...) SPDLOG_LOGGER_INFO(g_logger, __VA_ARGS__)
#define LOG_WARN(...) SPDLOG_LOGGER_WARN(g_logger, __VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_LOGGER_ERROR(g_logger, __VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(g_logger, __VA_ARGS__)
#define LOG_TRACE(...) SPDLOG_LOGGER_TRACE(g_logger, __VA_ARGS__)

#define LOG_INFO_CLASS(...) SPDLOG_LOGGER_INFO(logger::get_logger(logger::strip_class_prefix(typeid(*this).name())), __VA_ARGS__)
#define LOG_WARN_CLASS(...) SPDLOG_LOGGER_WARN(logger::get_logger(logger::strip_class_prefix(typeid(*this).name())), __VA_ARGS__)
#define LOG_ERROR_CLASS(...) SPDLOG_LOGGER_ERROR(logger::get_logger(logger::strip_class_prefix(typeid(*this).name())), __VA_ARGS__)
#define LOG_DEBUG_CLASS(...) SPDLOG_LOGGER_DEBUG(logger::get_logger(logger::strip_class_prefix(typeid(*this).name())), __VA_ARGS__)
#define LOG_TRACE_CLASS(...) SPDLOG_LOGGER_TRACE(logger::get_logger(logger::strip_class_prefix(typeid(*this).name())), __VA_ARGS__)

class logger {
public:
    static void open_console();

    static void init();

    static std::shared_ptr<spdlog::logger> get_logger(const std::string &name);

    static std::string strip_class_prefix(const std::string &name);

    static void set_async_mode();
};

inline std::shared_ptr<spdlog::logger> g_logger{};
