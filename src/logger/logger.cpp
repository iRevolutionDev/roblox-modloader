#include "common.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "spdlog/sinks/stdout_color_sinks-inl.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/async.h"

void logger::open_console() {
    AllocConsole();

    freopen_s(reinterpret_cast<FILE **>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE **>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE **>(stderr), "CONOUT$", "w", stderr);
}

void logger::init() {
    open_console();

    set_async_mode();
    spdlog::set_pattern("[%T] [%^%L%$] [%n] [%s:%#] %v");

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/roblox_modloader.log", 0, 0);
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    const auto new_logger = std::make_shared<spdlog::logger>("roblox_modloader",
                                                             spdlog::sinks_init_list{
                                                                 console_sink, file_sink, msvc_sink
                                                             });
    register_logger(new_logger);

    new_logger->set_level(spdlog::level::debug);
    new_logger->flush_on(spdlog::level::debug);
    new_logger->set_pattern("[%T] [%^%L%$] [%s:%#] %v");

    g_logger = new_logger;
}

void logger::set_async_mode() {
    spdlog::init_thread_pool(8192, 1); // queue with 8192 items and 1 backing thread.
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

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/roblox_modloader.log", 0, 0);
    auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    auto new_logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{
                                                           console_sink, file_sink, msvc_sink
                                                       });

    new_logger->set_level(spdlog::level::debug);
    new_logger->flush_on(spdlog::level::debug);

    return new_logger;
}
