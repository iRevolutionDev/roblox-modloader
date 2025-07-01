#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/config/config_serialization.hpp"

namespace rml::config::serialization {
    std::string log_level_to_string(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::trace: return "trace";
            case LogLevel::debug: return "debug";
            case LogLevel::info: return "info";
            case LogLevel::warn: return "warn";
            case LogLevel::error: return "error";
            case LogLevel::critical: return "critical";
            case LogLevel::off: return "off";
            default: return "info";
        }
    }

    std::optional<LogLevel> string_to_log_level(const std::string_view str) noexcept {
        if (str == "trace") return LogLevel::trace;
        if (str == "debug") return LogLevel::debug;
        if (str == "info") return LogLevel::info;
        if (str == "warn" || str == "warning") return LogLevel::warn;
        if (str == "error") return LogLevel::error;
        if (str == "critical") return LogLevel::critical;
        if (str == "off") return LogLevel::off;
        return std::nullopt;
    }

    toml::table logging_to_toml(const CoreConfig::Logging &logging) {
        toml::table table;

        table.insert_or_assign("level", log_level_to_string(logging.level));
        table.insert_or_assign("enable_console", logging.enable_console);
        table.insert_or_assign("enable_file_logging", logging.enable_file_logging);
        table.insert_or_assign("enable_async_logging", logging.enable_async_logging);
        table.insert_or_assign("log_directory", logging.log_directory.string());
        table.insert_or_assign("log_retention_hours", static_cast<std::int64_t>(logging.log_retention_hours.count()));
        table.insert_or_assign("max_log_files", static_cast<std::int64_t>(logging.max_log_files));

        return table;
    }

    ConfigResult<CoreConfig::Logging> logging_from_toml(const toml::table &table) {
        try {
            CoreConfig::Logging logging;

            if (const auto level_node = table["level"]) {
                if (const auto level_str = level_node.value<std::string>()) {
                    if (const auto level = string_to_log_level(*level_str)) {
                        logging.level = *level;
                    }
                }
            }

            if (const auto console_node = table["enable_console"]) {
                logging.enable_console = console_node.value_or(logging.enable_console);
            }

            if (const auto file_logging_node = table["enable_file_logging"]) {
                logging.enable_file_logging = file_logging_node.value_or(logging.enable_file_logging);
            }

            if (const auto async_logging_node = table["enable_async_logging"]) {
                logging.enable_async_logging = async_logging_node.value_or(logging.enable_async_logging);
            }

            if (const auto dir_node = table["log_directory"]) {
                if (const auto dir_str = dir_node.value<std::string>()) {
                    logging.log_directory = std::filesystem::path(*dir_str);
                }
            }

            if (const auto retention_node = table["log_retention_hours"]) {
                if (const auto hours = retention_node.value<std::int64_t>()) {
                    logging.log_retention_hours = std::chrono::hours(*hours);
                }
            }

            if (const auto max_files_node = table["max_log_files"]) {
                if (const auto max_files = max_files_node.value<std::int64_t>()) {
                    logging.max_log_files = static_cast<std::size_t>(*max_files);
                }
            }

            return logging;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse logging configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    toml::table performance_to_toml(const CoreConfig::Performance &performance) {
        toml::table table;

        table.insert_or_assign("enable_profiling", performance.enable_profiling);
        table.insert_or_assign("thread_pool_size", static_cast<std::int64_t>(performance.thread_pool_size));
        table.insert_or_assign("hook_timeout_ms", performance.hook_timeout.count());

        return table;
    }

    ConfigResult<CoreConfig::Performance> performance_from_toml(const toml::table &table) {
        try {
            CoreConfig::Performance performance;

            if (const auto profiling_node = table["enable_profiling"]) {
                performance.enable_profiling = profiling_node.value_or(performance.enable_profiling);
            }

            if (const auto thread_pool_node = table["thread_pool_size"]) {
                if (const auto size = thread_pool_node.value<std::int64_t>()) {
                    performance.thread_pool_size = static_cast<std::size_t>(*size);
                }
            }

            if (const auto timeout_node = table["hook_timeout_ms"]) {
                if (const auto timeout = timeout_node.value<std::int64_t>()) {
                    performance.hook_timeout = std::chrono::milliseconds(*timeout);
                }
            }

            return performance;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse performance configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    toml::table security_to_toml(const CoreConfig::Security &security) {
        toml::table table;

        table.insert_or_assign("verify_mod_signatures", security.verify_mod_signatures);
        table.insert_or_assign("sandbox_mods", security.sandbox_mods);
        table.insert_or_assign("max_memory_per_mod", static_cast<std::int64_t>(security.max_memory_per_mod));

        return table;
    }

    ConfigResult<CoreConfig::Security> security_from_toml(const toml::table &table) {
        try {
            CoreConfig::Security security;

            if (const auto verify_node = table["verify_mod_signatures"]) {
                security.verify_mod_signatures = verify_node.value_or(security.verify_mod_signatures);
            }

            if (const auto sandbox_node = table["sandbox_mods"]) {
                security.sandbox_mods = sandbox_node.value_or(security.sandbox_mods);
            }

            if (const auto memory_node = table["max_memory_per_mod"]) {
                if (const auto memory = memory_node.value<std::int64_t>()) {
                    security.max_memory_per_mod = static_cast<std::size_t>(*memory);
                }
            }

            return security;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse security configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    toml::table developer_to_toml(const CoreConfig::Developer &developer) {
        toml::table table;

        table.insert_or_assign("debug_mode", developer.debug_mode);
        table.insert_or_assign("enable_hot_reload", developer.enable_hot_reload);
        table.insert_or_assign("verbose_logging", developer.verbose_logging);

        return table;
    }

    ConfigResult<CoreConfig::Developer> developer_from_toml(const toml::table &table) {
        try {
            CoreConfig::Developer developer;

            if (const auto debug_node = table["debug_mode"]) {
                developer.debug_mode = debug_node.value_or(developer.debug_mode);
            }

            if (const auto hot_reload_node = table["enable_hot_reload"]) {
                developer.enable_hot_reload = hot_reload_node.value_or(developer.enable_hot_reload);
            }

            if (const auto verbose_node = table["verbose_logging"]) {
                developer.verbose_logging = verbose_node.value_or(developer.verbose_logging);
            }

            return developer;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse developer configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    toml::table mod_config_to_toml(const ModConfig &mod_config) {
        toml::table table;

        // Basic information
        table.insert_or_assign("name", mod_config.name);
        table.insert_or_assign("version", mod_config.version);
        table.insert_or_assign("author", mod_config.author);
        table.insert_or_assign("description", mod_config.description);

        // Runtime configuration
        toml::table runtime_table;
        runtime_table.insert_or_assign("enabled", mod_config.runtime.enabled);
        runtime_table.insert_or_assign("auto_load", mod_config.runtime.auto_load);
        runtime_table.insert_or_assign("priority", static_cast<std::int64_t>(mod_config.runtime.priority));

        if (mod_config.runtime.dependencies_path) {
            runtime_table.insert_or_assign("dependencies_path", mod_config.runtime.dependencies_path->string());
        }

        table.insert_or_assign("runtime", std::move(runtime_table));

        // Resources configuration
        toml::table resources_table;

        if (mod_config.resources.max_memory_usage) {
            resources_table.insert_or_assign("max_memory_usage",
                                             static_cast<std::int64_t>(*mod_config.resources.max_memory_usage));
        }

        if (mod_config.resources.max_execution_time) {
            resources_table.insert_or_assign("max_execution_time_ms",
                                             mod_config.resources.max_execution_time->
                                             count());
        }

        resources_table.insert_or_assign("assets_path", mod_config.resources.assets_path.string());
        table.insert_or_assign("resources", std::move(resources_table));

        // Custom settings
        if (!mod_config.custom_settings.empty()) {
            toml::table custom_table;
            for (const auto &[key, value]: mod_config.custom_settings) {
                insert_config_value(custom_table, key, value);
            }
            table.insert_or_assign("custom", std::move(custom_table));
        }

        return table;
    }

    ConfigResult<ModConfig> mod_config_from_toml(const toml::table &table) {
        try {
            ModConfig mod_config;
            // Basic information
            if (const auto name_node = table["name"]) {
                mod_config.name = name_node.value_or(std::string{});
            }

            if (const auto version_node = table["version"]) {
                mod_config.version = version_node.value_or(mod_config.version);
            }

            if (const auto author_node = table["author"]) {
                mod_config.author = author_node.value_or(std::string{});
            }

            if (const auto desc_node = table["description"]) {
                mod_config.description = desc_node.value_or(std::string{});
            }

            // Runtime configuration
            if (const auto runtime_node = table["runtime"]; runtime_node && runtime_node.is_table()) {
                const auto &runtime_table = *runtime_node.as_table();

                if (const auto enabled_node = runtime_table["enabled"]) {
                    mod_config.runtime.enabled = enabled_node.value_or(mod_config.runtime.enabled);
                }

                if (const auto auto_load_node = runtime_table["auto_load"]) {
                    mod_config.runtime.auto_load = auto_load_node.value_or(mod_config.runtime.auto_load);
                }

                if (const auto priority_node = runtime_table["priority"]) {
                    if (const auto priority = priority_node.value<std::int64_t>()) {
                        mod_config.runtime.priority = static_cast<std::int32_t>(*priority);
                    }
                }

                if (const auto deps_node = runtime_table["dependencies_path"]) {
                    if (const auto deps_str = deps_node.value<std::string>()) {
                        mod_config.runtime.dependencies_path = std::filesystem::path(*deps_str);
                    }
                }
            }

            // Resources configuration
            if (const auto resources_node = table["resources"]; resources_node && resources_node.is_table()) {
                const auto &resources_table = *resources_node.as_table();

                if (const auto memory_node = resources_table["max_memory_usage"]) {
                    if (const auto memory = memory_node.value<std::int64_t>()) {
                        mod_config.resources.max_memory_usage = static_cast<std::size_t>(*memory);
                    }
                }

                if (const auto time_node = resources_table["max_execution_time_ms"]) {
                    if (const auto time = time_node.value<std::int64_t>()) {
                        mod_config.resources.max_execution_time = std::chrono::milliseconds(*time);
                    }
                }

                if (const auto assets_node = resources_table["assets_path"]) {
                    if (const auto assets_str = assets_node.value<std::string>()) {
                        mod_config.resources.assets_path = std::filesystem::path(*assets_str);
                    }
                }
            }

            // Custom settings
            if (const auto custom_node = table["custom"]; custom_node && custom_node.is_table()) {
                for (const auto &custom_table = *custom_node.as_table(); const auto &[key, value_node]: custom_table) {
                    if (auto value_result = deserialize_config_value(value_node); value_result) {
                        mod_config.custom_settings[std::string(key)] = std::move(*value_result);
                    }
                }
            }

            return mod_config;
        } catch (const std::exception &e) {
            std::cerr << "Failed to parse mod configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    void insert_config_value(toml::table &table, const std::string &key, const ConfigValue &value) {
        std::visit([&table, &key]<typename T0>(const T0 &val) {
            using T = std::decay_t<T0>;
            if constexpr (std::same_as<T, std::filesystem::path>) {
                table.insert_or_assign(key, val.string());
            } else {
                table.insert_or_assign(key, val);
            }
        }, value);
    }

    ConfigResult<ConfigValue> deserialize_config_value(const toml::node &node) {
        try {
            if (node.is_boolean()) {
                return ConfigValue{node.value<bool>().value()};
            }

            if (node.is_integer()) {
                return ConfigValue{node.value<std::int64_t>().value()};
            }

            if (node.is_floating_point()) {
                return ConfigValue{node.value<double>().value()};
            }

            if (node.is_string()) {
                return ConfigValue{node.value<std::string>().value()};
            }

            return std::unexpected(ConfigError::parse_error);
        } catch (const std::exception &e) {
            std::cerr << "Failed to deserialize config value: " << e.what() << std::endl;
            return std::unexpected(ConfigError::parse_error);
        }
    }

    ValidationResult validate_core_config(const CoreConfig &config) {
        ValidationResult result;

        // Validate logging configuration
        if (config.logging.max_log_files == 0) {
            result.warnings.push_back("Max log files is set to 0, logs may accumulate indefinitely");
        }

        if (config.logging.log_retention_hours.count() <= 0) {
            result.errors.push_back("Log retention hours must be positive");
            result.is_valid = false;
        }

        // Validate performance configuration
        if (config.performance.thread_pool_size == 0) {
            result.errors.push_back("Thread pool size must be greater than 0");
            result.is_valid = false;
        }

        if (config.performance.thread_pool_size > std::thread::hardware_concurrency() * 4) {
            result.warnings.push_back("Thread pool size is very large compared to hardware concurrency");
        }

        if (config.performance.hook_timeout.count() <= 0) {
            result.errors.push_back("Hook timeout must be positive");
            result.is_valid = false;
        }

        // Validate security configuration
        if (config.security.max_memory_per_mod == 0) {
            result.warnings.push_back("Max memory per mod is set to 0, no memory limits will be enforced");
        }

        return result;
    }

    ValidationResult validate_mod_config(const ModConfig &config) {
        ValidationResult result;

        // Validate basic information
        if (config.name.empty()) {
            result.errors.push_back("Mod name cannot be empty");
            result.is_valid = false;
        }

        if (config.version.empty()) {
            result.warnings.push_back("Mod version is empty");
        }

        // Validate resources
        if (config.resources.max_memory_usage && *config.resources.max_memory_usage == 0) {
            result.warnings.push_back("Max memory usage is set to 0");
        }

        if (config.resources.max_execution_time && config.resources.max_execution_time->count() <= 0) {
            result.errors.push_back("Max execution time must be positive");
            result.is_valid = false;
        }

        return result;
    }
}
