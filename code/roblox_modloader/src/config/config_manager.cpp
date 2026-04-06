#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/config/config_manager.hpp"
#include "RobloxModLoader/config/config_serialization.hpp"

namespace rml::config {
    namespace {
        struct ConfigManagerHolder {
            static std::unique_ptr<ConfigManager> instance;
            static std::once_flag initialized;
        };

        std::unique_ptr<ConfigManager> ConfigManagerHolder::instance;
        std::once_flag ConfigManagerHolder::initialized;
    }

    ConfigResult<void> ConfigManager::load_config(const std::filesystem::path &config_path) {
        std::unique_lock lock(m_config_mutex);

        try {
            if (!std::filesystem::exists(config_path)) {
                return std::unexpected(ConfigError::file_not_found);
            }

            auto parse_result = toml::parse_file(config_path.string());
            if (!parse_result) {
                std::cerr << "Failed to parse configuration file: " << parse_result.error().description() << std::endl;
                return std::unexpected(ConfigError::parse_error);
            }

            const auto &table = parse_result.table();

            // Load core configuration
            if (auto core_result = load_core_config(table); !core_result) {
                return core_result;
            }

            // Load mod configurations
            if (auto mod_result = load_mod_configs(table); !mod_result) {
                return mod_result;
            }

            return {};
        } catch (const std::exception &e) {
            std::cerr << "Exception while loading configuration: " << e.what() << std::endl;
            return std::unexpected(ConfigError::io_error);
        }
    }

    ConfigResult<void> ConfigManager::save_config(const std::filesystem::path &config_path) const {
        std::shared_lock lock(m_config_mutex);

        try {
            // Create directory if it doesn't exist
            if (const auto parent_path = config_path.parent_path(); !parent_path.empty()) {
                std::filesystem::create_directories(parent_path);
            }

            toml::table root_table;

            // Add core configuration
            root_table.insert_or_assign("core", core_config_to_toml());

            // Add mod configurations
            if (!m_mod_configs.empty()) {
                root_table.insert_or_assign("mods", mod_configs_to_toml());
            }

            std::ofstream file(config_path);
            if (!file.is_open()) {
                std::cerr << "Failed to open configuration file for writing: " << config_path.string() << std::endl;
                return std::unexpected(ConfigError::io_error);
            }

            file << root_table;

            if (!file.good()) {
                std::cerr << "Failed to write configuration file: " << config_path.string() << std::endl;
                return std::unexpected(ConfigError::io_error);
            }

            return {};
        } catch (const std::exception &e) {
            std::cerr << "Exception while saving configuration: " << e.what() << std::endl;
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

    ConfigResult<void> ConfigManager::register_mod_config(const std::string &mod_name, ModConfig config) {
        if (mod_name.empty()) {
            return std::unexpected(ConfigError::validation_error);
        }
        if (const auto validation = serialization::validate_mod_config(config); !validation) {
            return std::unexpected(ConfigError::validation_error);
        }

        std::unique_lock lock(m_config_mutex);
        m_mod_configs[mod_name] = std::move(config);

        return {};
    }

    std::optional<ModConfig> ConfigManager::get_mod_config(const std::string &mod_name) const {
        std::shared_lock lock(m_config_mutex);

        if (const auto it = m_mod_configs.find(mod_name); it != m_mod_configs.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    ConfigResult<void> ConfigManager::update_mod_config(const std::string &mod_name, const ModConfig &config) {
        if (mod_name.empty()) {
            return std::unexpected(ConfigError::validation_error);
        }
        if (const auto validation = serialization::validate_mod_config(config); !validation) {
            return std::unexpected(ConfigError::validation_error);
        }

        std::unique_lock lock(m_config_mutex);

        if (!m_mod_configs.contains(mod_name)) {
            return std::unexpected(ConfigError::validation_error);
        }

        m_mod_configs[mod_name] = config;

        return {};
    }

    bool ConfigManager::remove_mod_config(const std::string &mod_name) {
        std::unique_lock lock(m_config_mutex);

        if (const auto erased = m_mod_configs.erase(mod_name); erased > 0) {
            return true;
        }

        return false;
    }

    std::vector<std::string> ConfigManager::get_registered_mods() const {
        std::shared_lock lock(m_config_mutex);

        std::vector<std::string> mod_names;
        mod_names.reserve(m_mod_configs.size());

        for (const auto &name: m_mod_configs | std::views::keys) {
            mod_names.push_back(name);
        }

        return mod_names;
    }

    ValidationResult ConfigManager::validate_all() const {
        std::shared_lock lock(m_config_mutex);

        ValidationResult result;
        const auto core_validation = serialization::validate_core_config(m_core_config);
        if (!core_validation) {
            result.is_valid = false;
            result.errors.insert(result.errors.end(),
                                 core_validation.errors.begin(),
                                 core_validation.errors.end());
        }
        result.warnings.insert(result.warnings.end(),
                               core_validation.warnings.begin(),
                               core_validation.warnings.end());
        for (const auto &[mod_name, mod_config]: m_mod_configs) {
            const auto mod_validation = serialization::validate_mod_config(mod_config);
            if (!mod_validation) {
                result.is_valid = false;
                for (const auto &error: mod_validation.errors) {
                    result.errors.push_back(std::format("[{}] {}", mod_name, error));
                }
            }
            for (const auto &warning: mod_validation.warnings) {
                result.warnings.push_back(std::format("[{}] {}", mod_name, warning));
            }
        }

        return result;
    }

    void ConfigManager::watch_config_file(const std::filesystem::path &config_path,
                                          std::function<void()> callback) {
        stop_watching();

        m_change_callback = std::move(callback);
        m_watching = true;

        m_watch_thread = std::make_unique<std::jthread>(
            [this, config_path](const std::stop_token &stop_token) {
                watch_thread_func(config_path, stop_token);
            }
        );
    }

    void ConfigManager::stop_watching() {
        if (!m_watch_thread) return;

        m_watching = false;
        m_watch_thread->request_stop();
        if (m_watch_thread->joinable()) {
            m_watch_thread->join();
        }
        m_watch_thread.reset();
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
            std::cerr << "Exception while creating default configuration: " << e.what() << std::endl;
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

    ConfigResult<void> ConfigManager::load_mod_configs(const toml::table &table) {
        if (const auto mods_node = table["mods"]; mods_node && mods_node.is_table()) {
            for (const auto &mods_table = *mods_node.as_table(); const auto &[mod_name, mod_node]: mods_table) {
                if (!mod_node.is_table()) continue;

                if (auto result = serialization::mod_config_from_toml(*mod_node.as_table()); result) {
                    m_mod_configs[std::string(mod_name)] = std::move(*result);
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

    toml::table ConfigManager::mod_configs_to_toml() const {
        toml::table mods_table;

        for (const auto &[mod_name, mod_config]: m_mod_configs) {
            mods_table.insert_or_assign(mod_name, serialization::mod_config_to_toml(mod_config));
        }

        return mods_table;
    }

    void ConfigManager::watch_thread_func(const std::filesystem::path &config_path, std::stop_token stop_token) {
        std::filesystem::file_time_type last_write_time;

        try {
            if (std::filesystem::exists(config_path)) {
                last_write_time = std::filesystem::last_write_time(config_path);
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed to get initial file time for config watching: " << e.what() << std::endl;
            return;
        }

        while (!stop_token.stop_requested() && m_watching) {
            try {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                if (!std::filesystem::exists(config_path)) {
                    continue;
                }

                if (const auto current_write_time = std::filesystem::last_write_time(config_path);
                    current_write_time != last_write_time) {
                    last_write_time = current_write_time;

                    if (const auto result = load_config(config_path); result) {
                        if (m_change_callback) {
                            m_change_callback();
                        }
                    } else {
                        std::cerr << "Failed to reload configuration after file change" << std::endl;
                    }
                }
            } catch (const std::exception &e) {
                std::cerr << "Exception in config watch thread: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
    }

    ConfigManager &get_config_manager() {
        std::call_once(ConfigManagerHolder::initialized, [] {
            ConfigManagerHolder::instance = std::make_unique<ConfigManager>();
        });

        return *ConfigManagerHolder::instance;
    }
}
