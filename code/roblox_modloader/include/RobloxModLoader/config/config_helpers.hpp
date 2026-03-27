#pragma once

#include "config.hpp"
#include <functional>
#include <memory>

namespace rml::config::helpers {
    class ConfigWatcher final {
    public:
        explicit ConfigWatcher(std::filesystem::path config_path,
                               std::function<void()> on_change = nullptr)
            : m_config_path(std::move(config_path))
              , m_on_change(std::move(on_change)) {
            start_watching();
        }

        ~ConfigWatcher() {
            stop_watching();
        }

        // Non-copyable, movable
        ConfigWatcher(const ConfigWatcher &) = delete;

        ConfigWatcher &operator=(const ConfigWatcher &) = delete;

        ConfigWatcher(ConfigWatcher &&) = default;

        ConfigWatcher &operator=(ConfigWatcher &&) = default;

    private:
        std::filesystem::path m_config_path;
        std::function<void()> m_on_change;

        void start_watching() const {
            if (m_on_change) {
                get_config_manager().watch_config_file(m_config_path, m_on_change);
            } else {
                get_config_manager().watch_config_file(m_config_path, []() {
                    LOG_INFO("Configuration file reloaded");
                });
            }
        }

        static void stop_watching() {
            get_config_manager().stop_watching();
        }
    };

    class ModConfigBuilder final {
    public:
        ModConfigBuilder() = default;

        ModConfigBuilder &name(std::string name) {
            m_config.name = std::move(name);
            return *this;
        }

        ModConfigBuilder &version(std::string version) {
            m_config.version = std::move(version);
            return *this;
        }

        ModConfigBuilder &author(std::string author) {
            m_config.author = std::move(author);
            return *this;
        }

        ModConfigBuilder &description(std::string description) {
            m_config.description = std::move(description);
            return *this;
        }

        ModConfigBuilder &enabled(const bool enabled = true) {
            m_config.runtime.enabled = enabled;
            return *this;
        }

        ModConfigBuilder &auto_load(const bool auto_load = true) {
            m_config.runtime.auto_load = auto_load;
            return *this;
        }

        ModConfigBuilder &priority(const std::int32_t priority) {
            m_config.runtime.priority = priority;
            return *this;
        }

        ModConfigBuilder &dependencies_path(std::filesystem::path path) {
            m_config.runtime.dependencies_path = std::move(path);
            return *this;
        }

        ModConfigBuilder &max_memory(std::size_t bytes) {
            m_config.resources.max_memory_usage = bytes;
            return *this;
        }

        ModConfigBuilder &max_execution_time(std::chrono::milliseconds time) {
            m_config.resources.max_execution_time = time;
            return *this;
        }

        ModConfigBuilder &assets_path(std::filesystem::path path) {
            m_config.resources.assets_path = std::move(path);
            return *this;
        }

        template<typename T>
        ModConfigBuilder &custom_setting(const std::string &key, T &&value) {
            if constexpr (std::same_as<std::decay_t<T>, std::filesystem::path>) {
                m_config.custom_settings[key] = ConfigValue{value};
            } else {
                m_config.custom_settings[key] = ConfigValue{std::forward<T>(value)};
            }
            return *this;
        }

        [[nodiscard]] ModConfig build() const {
            return m_config;
        }

        [[nodiscard]] ConfigResult<void> register_mod(const std::string &mod_name) const {
            return get_config_manager().register_mod_config(mod_name, m_config);
        }

    private:
        ModConfig m_config;
    };

    template<typename T>
    [[nodiscard]] T get_custom_setting(const ModConfig &config, const std::string &key, T default_value = T{}) {
        if (const auto it = config.custom_settings.find(key); it != config.custom_settings.end()) {
            if (std::holds_alternative<T>(it->second)) {
                return std::get<T>(it->second);
            }
        }
        return default_value;
    }

    [[nodiscard]] inline std::filesystem::path get_custom_path(const ModConfig &config,
                                                               const std::string &key,
                                                               std::filesystem::path default_value = {}) {
        if (const auto it = config.custom_settings.find(key); it != config.custom_settings.end()) {
            if (std::holds_alternative<std::filesystem::path>(it->second)) {
                return std::get<std::filesystem::path>(it->second);
            }
            if (std::holds_alternative<std::string>(it->second)) {
                return std::filesystem::path(std::get<std::string>(it->second));
            }
        }
        return default_value;
    }

    [[nodiscard]] inline ConfigResult<void> load_mod_config_from_file(const std::string &mod_name,
                                                                      const std::filesystem::path &config_path) {
        try {
            if (!std::filesystem::exists(config_path)) {
                return std::unexpected(ConfigError::file_not_found);
            }

            auto parse_result = toml::parse_file(config_path.string());
            if (!parse_result) {
                return std::unexpected(ConfigError::parse_error);
            }

            if (auto mod_config_result = serialization::mod_config_from_toml(parse_result.table()); mod_config_result) {
                return get_config_manager().register_mod_config(mod_name, std::move(*mod_config_result));
            } else {
                return std::unexpected(mod_config_result.error());
            }
        } catch (const std::exception &) {
            return std::unexpected(ConfigError::io_error);
        }
    }

    [[nodiscard]] inline ConfigResult<void> save_mod_config_to_file(const std::string &mod_name,
                                                                    const std::filesystem::path &config_path) {
        try {
            const auto mod_config = get_config_manager().get_mod_config(mod_name);
            if (!mod_config) {
                return std::unexpected(ConfigError::validation_error);
            }

            // Create directory if it doesn't exist
            if (const auto parent_path = config_path.parent_path(); !parent_path.empty()) {
                std::filesystem::create_directories(parent_path);
            }

            const auto toml_table = serialization::mod_config_to_toml(*mod_config);

            std::ofstream file(config_path);
            if (!file.is_open()) {
                return std::unexpected(ConfigError::io_error);
            }

            file << toml_table;

            if (!file.good()) {
                return std::unexpected(ConfigError::io_error);
            }

            return {};
        } catch (const std::exception &) {
            return std::unexpected(ConfigError::io_error);
        }
    }
}
