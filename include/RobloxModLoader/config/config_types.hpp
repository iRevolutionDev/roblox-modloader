#pragma once

#include <string>
#include <filesystem>
#include <chrono>
#include <optional>
#include <variant>
#include <expected>
#include <unordered_map>

namespace rml::config {
    enum class LogLevel {
        trace = 0,
        debug = 1,
        info = 2,
        warn = 3,
        error = 4,
        critical = 5,
        off = 6
    };

    enum class ConfigError {
        file_not_found,
        parse_error,
        validation_error,
        io_error
    };

    template<typename T>
    using ConfigResult = std::expected<T, ConfigError>;
    using ConfigValue = std::variant<
        bool,
        std::int64_t,
        double,
        std::string,
        std::filesystem::path
    >;

    struct CoreConfig {
        struct Logging {
            LogLevel level{LogLevel::info};
            bool enable_console{true};
            bool enable_file_logging{true};
            bool enable_async_logging{true};
            std::filesystem::path log_directory{"logs"};
            std::chrono::hours log_retention_hours{24 * 7}; // 1 week
            std::size_t max_log_files{10};
        } logging;

        struct Performance {
            bool enable_profiling{false};
            std::size_t thread_pool_size{4};
            std::chrono::milliseconds hook_timeout{5000};
        } performance;

        struct Security {
            bool verify_mod_signatures{false};
            bool sandbox_mods{true};
            std::size_t max_memory_per_mod{100 * 1024 * 1024}; // 100MB
        } security;

        struct Developer {
            bool debug_mode{false};
            bool enable_hot_reload{false};
            bool verbose_logging{false};
        } developer;
    };

    struct ModConfig {
        std::string name;
        std::string version{"1.0.0"};
        std::string author;
        std::string description;

        struct Runtime {
            bool enabled{true};
            bool auto_load{true};
            std::int32_t priority{0};
            std::optional<std::filesystem::path> dependencies_path;
        } runtime;

        struct Resources {
            std::optional<std::size_t> max_memory_usage;
            std::optional<std::chrono::milliseconds> max_execution_time;
            std::filesystem::path assets_path{"assets"};
        } resources;

        struct DataModelContext {
            std::vector<std::string> standalone;
            std::vector<std::string> edit;
            std::vector<std::string> client;
            std::vector<std::string> server;
        } datamodel_context;

        std::unordered_map<std::string, ConfigValue> custom_settings;
    };

    struct ValidationResult {
        bool is_valid{true};
        std::vector<std::string> errors;
        std::vector<std::string> warnings;

        [[nodiscard]] explicit operator bool() const noexcept {
            return is_valid;
        }
    };
}
