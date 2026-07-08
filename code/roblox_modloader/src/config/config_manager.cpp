#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/config/config_serialization.hpp"
#include "config_manager.hpp"

RML_LOG_SCOPE("ConfigManager");

namespace rml::config {
    ConfigResult<void> ConfigManager::load_config(const std::filesystem::path &config_path) {
        std::unique_lock lock(m_config_mutex);

        try {
            if (!std::filesystem::exists(config_path)) {
                return std::unexpected(ConfigError::file_not_found);
            }

            auto parse_result = toml::parse_file(config_path.string());
            if (!parse_result) {
                RML_ERROR("Failed to parse configuration file: {}", parse_result.error().description());
                return std::unexpected(ConfigError::parse_error);
            }

            const auto &table = parse_result.table();

            if (auto core_result = load_core_config(table); !core_result) {
                return core_result;
            }

            return {};
        } catch (const std::exception &e) {
            RML_ERROR("Exception while loading configuration: {}", e.what());
            return std::unexpected(ConfigError::io_error);
        }
    }

    ConfigResult<void> ConfigManager::save_config(const std::filesystem::path &config_path) const {
        std::shared_lock lock(m_config_mutex);

        try {
            if (const auto parent_path = config_path.parent_path(); !parent_path.empty()) {
                std::filesystem::create_directories(parent_path);
            }

            toml::table root_table;
            root_table.insert_or_assign("core", core_config_to_toml());

            std::ofstream file(config_path);
            if (!file.is_open()) {
                RML_ERROR("Failed to open configuration file for writing: {}", config_path.string());
                return std::unexpected(ConfigError::io_error);
            }

            file << root_table;

            if (!file.good()) {
                RML_ERROR("Failed to write configuration file: {}", config_path.string());
                return std::unexpected(ConfigError::io_error);
            }

            return {};
        } catch (const std::exception &e) {
            RML_ERROR("Exception while saving configuration: {}", e.what());
            return std::unexpected(ConfigError::io_error);
        }
    }

    const CoreConfig &ConfigManager::get_core_config() const {
        std::shared_lock lock(m_config_mutex);
        return m_core_config;
    }

    ConfigResult<void> ConfigManager::update_core_config(CoreConfig config) {
        if (const auto validation = serialization::validate_core_config(config); !validation) {
            return std::unexpected(ConfigError::validation_error);
        }

        std::unique_lock lock(m_config_mutex);
        m_core_config = std::move(config);

        return {};
    }

    ValidationResult ConfigManager::validate_all() const {
        std::shared_lock lock(m_config_mutex);
        return serialization::validate_core_config(m_core_config);
    }

    void ConfigManager::watch_config_file(const std::filesystem::path &config_path,
                                          std::function<void()> callback) {
        m_watcher.start(config_path, std::chrono::milliseconds(1000),
                        [this, config_path, callback = std::move(callback)] {
                            if (const auto result = load_config(config_path); !result) {
                                RML_ERROR("Failed to reload configuration after file change");
                                return false;
                            }

                            if (callback) {
                                callback();
                            }

                            return true;
                        });
    }

    void ConfigManager::stop_watching() {
        m_watcher.stop();
    }

    ConfigResult<void> ConfigManager::create_default_config(const std::filesystem::path &config_path) {
        try {
            if (const auto parent_path = config_path.parent_path(); !parent_path.empty()) {
                std::filesystem::create_directories(parent_path);
            }

            const ConfigManager temp_manager;
            if (auto result = temp_manager.save_config(config_path); !result) {
                return result;
            }

            return {};
        } catch (const std::exception &e) {
            RML_ERROR("Exception while creating default configuration: {}", e.what());
            return std::unexpected(ConfigError::io_error);
        }
    }

    ConfigResult<void> ConfigManager::load_core_config(const toml::table &table) {
        if (const auto core_node = table["core"]; core_node && core_node.is_table()) {
            const auto &core_table = *core_node.as_table();
            if (const auto logging_node = core_table["logging"]; logging_node && logging_node.is_table()) {
                if (auto result = serialization::logging_from_toml(*logging_node.as_table()); result) {
                    m_core_config.logging = std::move(*result);
                } else {
                    return std::unexpected(result.error());
                }
            }
            if (const auto perf_node = core_table["performance"]; perf_node && perf_node.is_table()) {
                if (auto result = serialization::performance_from_toml(*perf_node.as_table()); result) {
                    m_core_config.performance = std::move(*result);
                } else {
                    return std::unexpected(result.error());
                }
            }
            if (const auto security_node = core_table["security"]; security_node && security_node.is_table()) {
                if (auto result = serialization::security_from_toml(*security_node.as_table()); result) {
                    m_core_config.security = std::move(*result);
                } else {
                    return std::unexpected(result.error());
                }
            }
            if (const auto dev_node = core_table["developer"]; dev_node && dev_node.is_table()) {
                if (auto result = serialization::developer_from_toml(*dev_node.as_table()); result) {
                    m_core_config.developer = std::move(*result);
                } else {
                    return std::unexpected(result.error());
                }
            }
        }

        return {};
    }

    toml::table ConfigManager::core_config_to_toml() const {
        toml::table core_table;

        core_table.insert_or_assign("logging", serialization::logging_to_toml(m_core_config.logging));
        core_table.insert_or_assign("performance", serialization::performance_to_toml(m_core_config.performance));
        core_table.insert_or_assign("security", serialization::security_to_toml(m_core_config.security));
        core_table.insert_or_assign("developer", serialization::developer_to_toml(m_core_config.developer));

        return core_table;
    }

    ConfigManager &get_config_manager() {
        static ConfigManager instance;
        return instance;
    }
}
