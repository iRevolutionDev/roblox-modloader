#pragma once

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/config/config_types.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"

#include <format>
#include <string_view>

#include "pointers.hpp"
#include "RobloxModLoader/roblox/util/standard_out.hpp"

namespace RBX {
    class ScriptContext;
}

namespace rml::luau {
    struct ScriptInfo {
        std::string pattern;
        std::filesystem::path full_path;
        std::string content;
    };

    struct ModScriptContext {
        std::string mod_name;
        std::filesystem::path mod_path;
        std::unordered_map<RBX::DataModelType, std::vector<ScriptInfo> > scripts_by_context;
        bool loaded{false};
    };

    class ScriptManager final {
    public:
        ScriptManager();

        ~ScriptManager();

        ScriptManager(const ScriptManager &) = delete;

        ScriptManager &operator=(const ScriptManager &) = delete;

        ScriptManager(ScriptManager &&) = delete;

        ScriptManager &operator=(ScriptManager &&) = delete;

        void initialize();

        void shutdown();

        void load_mod_scripts(const std::filesystem::path &mods_directory);

        void load_mod_scripts_for_context(const std::filesystem::path &mod_directory,
                                          const config::ModConfig &mod_config);

        void execute_scripts_for_context(RBX::DataModelType data_model_type);

        [[nodiscard]] const std::vector<ModScriptContext> &get_loaded_mods() const noexcept {
            return m_loaded_mods;
        }

        void reload_mod_scripts(const std::string &mod_name);

        [[nodiscard]] bool is_hot_reload_enabled() const noexcept {
            return m_hot_reload_enabled;
        }

        void set_hot_reload_enabled(const bool enabled) noexcept {
            m_hot_reload_enabled = enabled;
        }

    private:
        mutable std::shared_mutex m_scripts_mutex;
        std::vector<ModScriptContext> m_loaded_mods;
        std::atomic<bool> m_hot_reload_enabled{false};

        [[nodiscard]] static std::vector<ScriptInfo> resolve_script_patterns(
            const std::filesystem::path &scripts_directory,
            const std::vector<std::string> &patterns);

        [[nodiscard]] static std::string load_script_content(const std::filesystem::path &script_path);

        [[nodiscard]] static bool matches_pattern(const std::filesystem::path &file_path,
                                                  const std::string &pattern);

        [[nodiscard]] static std::vector<std::filesystem::path> find_matching_files(
            const std::filesystem::path &directory,
            const std::vector<std::string> &patterns);

        static void schedule_script(RBX::DataModelType data_model_type, const ScriptInfo &script_info);

        static void execute_script_async(const std::shared_ptr<class ScriptEngine> &engine,
                                         const ScriptInfo &script_info,
                                         const std::string &chunk_name) noexcept;

        template<typename ResultType>
        static void handle_script_result(const ScriptInfo &script_info,
                                         const ResultType &result) noexcept {
            try {
                if (!result.success) [[unlikely]] {
                    const auto error_msg = std::format("Script execution failed: {}", result.error_message);
                    log_script_error(script_info, error_msg);

                    if (const auto roblox_error = std::format("Script '{}' execution failed: {}",
                                                              script_info.full_path.filename().string(),
                                                              result.error_message);
                        roblox_error.size() < 1024) {
                        g_pointers->m_roblox_pointers.print(
                            RBX::MESSAGE_ERROR,
                            "%s\n",
                            roblox_error.c_str()
                        );
                    }
                } else {
                    log_script_success(script_info);
                }
            } catch (...) {
                log_script_error(script_info, "Critical error in result handling");
            }
        }

        static void log_script_error(const ScriptInfo &script_info,
                                     std::string_view error_message) noexcept;

        static void log_script_success(const ScriptInfo &script_info) noexcept;
    };

    inline ScriptManager *g_script_manager{};
}
