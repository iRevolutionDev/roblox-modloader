#pragma once

#include "RobloxModLoader/config/config_types.hpp"
#include "config_manager.hpp"

namespace rml::config {
    inline ConfigResult<void> initialize(const std::filesystem::path &config_path,
                                         const bool create_default = true) {
        auto &config_mgr = get_config_manager();

        if (!std::filesystem::exists(config_path) && create_default) {
            if (auto result = ConfigManager::create_default_config(config_path); !result) {
                return result;
            }
        }

        return config_mgr.load_config(config_path);
    }

    inline void shutdown() {
        auto &config_mgr = get_config_manager();
        config_mgr.stop_watching();
    }

    inline const CoreConfig &core() {
        return get_config_manager().get_core_config();
    }

    inline bool is_console_logging_enabled() {
        return core().logging.enable_console;
    }

    inline LogLevel get_log_level() {
        return core().logging.level;
    }

    inline bool is_debug_mode() {
        return core().developer.debug_mode;
    }
}
