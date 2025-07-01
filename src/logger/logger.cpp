#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "spdlog/sinks/stdout_color_sinks-inl.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/async.h"
#include <atomic>
#include <filesystem>
#include <vector>

namespace {
    struct global_logger_holder {
        static std::atomic<std::shared_ptr<spdlog::logger> > logger;
        static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink;
        static std::shared_ptr<spdlog::sinks::daily_file_sink_mt> file_sink;
        static std::shared_ptr<spdlog::sinks::msvc_sink_mt> msvc_sink;
    };

    std::atomic<std::shared_ptr<spdlog::logger> > global_logger_holder::logger;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> global_logger_holder::console_sink;
    std::shared_ptr<spdlog::sinks::daily_file_sink_mt> global_logger_holder::file_sink;
    std::shared_ptr<spdlog::sinks::msvc_sink_mt> global_logger_holder::msvc_sink;

    void ensure_log_directory() {
        std::filesystem::create_directories("logs");
    }

    void init_sinks() {
        if (!global_logger_holder::console_sink) {
            global_logger_holder::console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            global_logger_holder::file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
                "logs/roblox_modloader.log", 0, 0);
            global_logger_holder::msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
        }
    }

    std::vector<spdlog::sink_ptr> get_shared_sinks() {
        init_sinks();
        return {
            global_logger_holder::console_sink,
            global_logger_holder::file_sink,
            global_logger_holder::msvc_sink
        };
    }
}

std::shared_ptr<spdlog::logger> global_logger() {
    auto logger = global_logger_holder::logger.load();
    if (!logger) {
        logger::init();
        logger = global_logger_holder::logger.load();
    }
    return logger;
}

void logger::open_console() {
    static bool console_allocated = false;

    if (console_allocated) {
        return;
    }

    if (const auto &config = rml::config::get_config_manager().get_core_config(); !config.logging.enable_console) {
        return;
    }

    if (AllocConsole()) {
        console_allocated = true;
        freopen_s(reinterpret_cast<FILE **>(stdout), "CONOUT$", "w", stdout);
        freopen_s(reinterpret_cast<FILE **>(stdin), "CONIN$", "r", stdin);
        freopen_s(reinterpret_cast<FILE **>(stderr), "CONOUT$", "w", stderr);
    }
}

void logger::init() {
    static bool logger_initialized = false;

    if (logger_initialized) {
        return;
    }

#if IS_RML
    open_console();
#endif
    ensure_log_directory();

    try {
        if (const auto &config = rml::config::get_config_manager().get_core_config(); config.logging.
            enable_async_logging) {
            set_async_mode();
        }
    } catch (...) {
        set_async_mode();
    }

    const auto sinks = get_shared_sinks();
    const auto new_logger = std::make_shared<spdlog::logger>("roblox_modloader", sinks.begin(), sinks.end());

    spdlog::register_logger(new_logger);

    try {
        const auto &config = rml::config::get_config_manager().get_core_config();
        new_logger->set_level(static_cast<spdlog::level::level_enum>(config.logging.level));
        new_logger->flush_on(static_cast<spdlog::level::level_enum>(config.logging.level));
    } catch (...) {
        new_logger->set_level(spdlog::level::debug);
        new_logger->flush_on(spdlog::level::debug);
    }

    new_logger->set_pattern("[%T] [%^%L%$] [%n] %v");

    global_logger_holder::logger.store(new_logger);
    logger_initialized = true;
}

void logger::set_async_mode() {
    spdlog::init_thread_pool(8192, 1);
}

std::string logger::strip_class_prefix(const std::string &name) {
    if (const std::string prefix = "class "; name.compare(0, prefix.size(), prefix) == 0) {
        return name.substr(prefix.size());
    }
    return name;
}

std::shared_ptr<spdlog::logger> logger::get_logger(const std::string &name) {
    if (auto existing_logger = spdlog::get(name)) {
        return existing_logger;
    }

    ensure_log_directory();
    const auto sinks = get_shared_sinks();
    auto new_logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

    new_logger->set_level(spdlog::level::debug);
    new_logger->flush_on(spdlog::level::debug);
    new_logger->set_pattern("[%T] [%^%L%$] [%n] %v");

    spdlog::register_logger(new_logger);

    return new_logger;
}
