#pragma once

#include "RobloxModLoader/config/config_types.hpp"
#include "filesystem/file_watcher.hpp"

#include <functional>
#include <shared_mutex>
#include <toml++/toml.hpp>

namespace rml::config {
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

        ConfigResult<void> load_or_create(const std::filesystem::path &config_path);

        ConfigResult<void> save_config(const std::filesystem::path &config_path) const;

        [[nodiscard]] const CoreConfig &get_core_config() const;

        ConfigResult<void> update_core_config(CoreConfig config);

        [[nodiscard]] ValidationResult validate_all() const;

        void watch_config_file(const std::filesystem::path &config_path,
                               std::function<void()> callback);

        void stop_watching();

    private:
        mutable std::shared_mutex m_config_mutex;
        CoreConfig m_core_config;

        filesystem::FileWatcher m_watcher;

        ConfigResult<void> load_core_config(const toml::table &table);

        [[nodiscard]] toml::table core_config_to_toml() const;

        static void backup_broken_config(const std::filesystem::path &config_path);

        ConfigResult<void> write_defaults(const std::filesystem::path &config_path);
    };

    ConfigManager &get_config_manager();
}
