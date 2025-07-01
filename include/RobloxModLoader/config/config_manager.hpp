#pragma once

#include "config_types.hpp"
#include <toml++/toml.hpp>

namespace rml::config {
    template<typename T>
    concept Configurable = requires(T &t, const toml::table &table)
    {
        { t.from_toml(table) } -> std::same_as<ConfigResult<void> >;
        { t.to_toml() } -> std::same_as<toml::table>;
        { t.validate() } -> std::same_as<ValidationResult>;
    };

    class IConfigurable {
    public:
        virtual ~IConfigurable() = default;

        virtual ConfigResult<void> from_toml(const toml::table &table) = 0;

        virtual toml::table to_toml() const = 0;

        virtual ValidationResult validate() const = 0;
    };

    class ConfigManager final {
    public:
        ConfigManager() = default;

        ~ConfigManager() = default;

        // Non-copyable, movable
        ConfigManager(const ConfigManager &) = delete;

        ConfigManager &operator=(const ConfigManager &) = delete;

        ConfigManager(ConfigManager &&) = default;

        ConfigManager &operator=(ConfigManager &&) = default;

        ConfigResult<void> load_config(const std::filesystem::path &config_path);

        ConfigResult<void> save_config(const std::filesystem::path &config_path) const;

        [[nodiscard]] const CoreConfig &get_core_config() const;

        ConfigResult<void> update_core_config(CoreConfig config);

        ConfigResult<void> register_mod_config(const std::string &mod_name, ModConfig config);

        [[nodiscard]] std::optional<ModConfig> get_mod_config(const std::string &mod_name) const;

        ConfigResult<void> update_mod_config(const std::string &mod_name, const ModConfig &config);

        bool remove_mod_config(const std::string &mod_name);

        [[nodiscard]] std::vector<std::string> get_registered_mods() const;

        [[nodiscard]] ValidationResult validate_all() const;

        void watch_config_file(const std::filesystem::path &config_path,
                               std::function<void()> callback);

        void stop_watching();

        static ConfigResult<void> create_default_config(const std::filesystem::path &config_path);

    private:
        mutable std::shared_mutex m_config_mutex;
        CoreConfig m_core_config;
        std::unordered_map<std::string, ModConfig> m_mod_configs;

        std::atomic<bool> m_watching{false};
        std::unique_ptr<std::jthread> m_watch_thread;
        std::function<void()> m_change_callback;

        ConfigResult<void> load_core_config(const toml::table &table);

        ConfigResult<void> load_mod_configs(const toml::table &table);

        [[nodiscard]] toml::table core_config_to_toml() const;

        [[nodiscard]] toml::table mod_configs_to_toml() const;

        void watch_thread_func(const std::filesystem::path &config_path, std::stop_token stop_token);
    };

    ConfigManager &get_config_manager();
}
