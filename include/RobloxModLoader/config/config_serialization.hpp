#pragma once

#include "config_types.hpp"
#include <toml++/toml.hpp>

namespace rml::config::serialization {
    [[nodiscard]] std::string log_level_to_string(LogLevel level) noexcept;

    [[nodiscard]] std::optional<LogLevel> string_to_log_level(std::string_view str) noexcept;

    [[nodiscard]] toml::table logging_to_toml(const CoreConfig::Logging &logging);

    [[nodiscard]] ConfigResult<CoreConfig::Logging> logging_from_toml(const toml::table &table);

    [[nodiscard]] toml::table performance_to_toml(const CoreConfig::Performance &performance);

    [[nodiscard]] ConfigResult<CoreConfig::Performance> performance_from_toml(const toml::table &table);

    [[nodiscard]] toml::table security_to_toml(const CoreConfig::Security &security);

    [[nodiscard]] ConfigResult<CoreConfig::Security> security_from_toml(const toml::table &table);

    [[nodiscard]] toml::table developer_to_toml(const CoreConfig::Developer &developer);

    [[nodiscard]] ConfigResult<CoreConfig::Developer> developer_from_toml(const toml::table &table);

    [[nodiscard]] toml::table mod_config_to_toml(const ModConfig &mod_config);

    [[nodiscard]] ConfigResult<ModConfig> mod_config_from_toml(const toml::table &table);

    void insert_config_value(toml::table &table, const std::string &key, const ConfigValue &value);

    [[nodiscard]] ConfigResult<ConfigValue> deserialize_config_value(const toml::node &node);

    [[nodiscard]] ValidationResult validate_core_config(const CoreConfig &config);

    [[nodiscard]] ValidationResult validate_mod_config(const ModConfig &config);
}
