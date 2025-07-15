#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/script_manager.hpp"
#include "RobloxModLoader/config/config.hpp"
#include "RobloxModLoader/config/config_helpers.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "pointers.hpp"

namespace rml::luau {
    ScriptManager::ScriptManager() {
        g_script_manager = this;

        initialize();
    }

    ScriptManager::~ScriptManager() {
        g_script_manager = nullptr;

        shutdown();
    }

    void ScriptManager::initialize() {
        LOG_INFO("Initializing mod script manager...");
        if (config::is_debug_mode()) {
            m_hot_reload_enabled = true;
            LOG_INFO("Hot reload enabled in debug mode");
        }

        LOG_INFO("Mod script manager initialized successfully");
    }

    void ScriptManager::shutdown() {
        LOG_INFO("Shutting down mod script manager...");

        std::unique_lock lock(m_scripts_mutex);
        m_loaded_mods.clear();

        LOG_INFO("Mod script manager shutdown complete");
    }

    void ScriptManager::load_mod_scripts(const std::filesystem::path &mods_directory) {
        LOG_INFO("Loading mod scripts from: {}", mods_directory.string());

        if (!std::filesystem::exists(mods_directory)) {
            LOG_WARN("Mods directory does not exist: {}", mods_directory.string());
            return;
        }

        std::unique_lock lock(m_scripts_mutex);
        m_loaded_mods.clear();

        for (const auto &entry: std::filesystem::directory_iterator(mods_directory)) {
            if (!entry.is_directory()) {
                continue;
            }

            const auto &mod_directory = entry.path();
            const auto mod_name = mod_directory.filename().string();
            const auto config_path = mod_directory / "mod.toml";

            if (!std::filesystem::exists(config_path)) {
                LOG_WARN("No config.toml found for mod: {}", mod_name);
                continue;
            }

            // Load mod configuration
            if (const auto config_result = config::helpers::load_mod_config_from_file(mod_name, config_path); !
                config_result) {
                LOG_ERROR("Failed to load config for mod: {}", mod_name);
                continue;
            }

            const auto mod_config = config::mod(mod_name);
            if (!mod_config) {
                LOG_ERROR("Failed to get loaded config for mod: {}", mod_name);
                continue;
            }

            load_mod_scripts_for_context(mod_directory, *mod_config);
        }

        LOG_INFO("Loaded scripts for {} mods", m_loaded_mods.size());
    }

    void ScriptManager::load_mod_scripts_for_context(const std::filesystem::path &mod_directory,
                                                     const config::ModConfig &mod_config) {
        LOG_INFO("Loading scripts for mod: {}", mod_config.name);

        const auto scripts_directory = mod_directory / "scripts";
        if (!std::filesystem::exists(scripts_directory)) {
            LOG_WARN("Scripts directory not found for mod: {}", mod_config.name);
            return;
        }

        ModScriptContext mod_context;
        mod_context.mod_name = mod_config.name;
        mod_context.mod_path = mod_directory;

        // Load scripts for each DataModel context
        if (!mod_config.datamodel_context.standalone.empty()) {
            auto scripts = resolve_script_patterns(scripts_directory, mod_config.datamodel_context.standalone);
            mod_context.scripts_by_context[RBX::DataModelType::Standalone] = std::move(scripts);
            LOG_INFO("Loaded {} standalone scripts for mod: {}",
                     mod_context.scripts_by_context[RBX::DataModelType::Standalone].size(), mod_config.name);
        }

        if (!mod_config.datamodel_context.edit.empty()) {
            auto scripts = resolve_script_patterns(scripts_directory, mod_config.datamodel_context.edit);
            mod_context.scripts_by_context[RBX::DataModelType::Edit] = std::move(scripts);
            LOG_INFO("Loaded {} edit scripts for mod: {}",
                     mod_context.scripts_by_context[RBX::DataModelType::Edit].size(), mod_config.name);
        }

        if (!mod_config.datamodel_context.client.empty()) {
            auto scripts = resolve_script_patterns(scripts_directory, mod_config.datamodel_context.client);
            mod_context.scripts_by_context[RBX::DataModelType::Client] = std::move(scripts);
            LOG_INFO("Loaded {} client scripts for mod: {}",
                     mod_context.scripts_by_context[RBX::DataModelType::Client].size(), mod_config.name);
        }

        if (!mod_config.datamodel_context.server.empty()) {
            auto scripts = resolve_script_patterns(scripts_directory, mod_config.datamodel_context.server);
            mod_context.scripts_by_context[RBX::DataModelType::Server] = std::move(scripts);
            LOG_INFO("Loaded {} server scripts for mod: {}",
                     mod_context.scripts_by_context[RBX::DataModelType::Server].size(), mod_config.name);
        }

        mod_context.loaded = true;
        m_loaded_mods.push_back(std::move(mod_context));
    }

    void ScriptManager::execute_scripts_for_context(RBX::DataModelType data_model_type) {
        std::shared_lock lock(m_scripts_mutex);

        LOG_INFO("Executing scripts for DataModel type: {}", static_cast<int>(data_model_type));

        for (auto &mod_context: m_loaded_mods) {
            if (!mod_context.loaded) {
                continue;
            }

            const auto it = mod_context.scripts_by_context.find(data_model_type);
            if (it == mod_context.scripts_by_context.end()) {
                continue;
            }

            LOG_INFO("Scheduling {} scripts for mod: {} (DataModel type: {})",
                     it->second.size(), mod_context.mod_name, static_cast<int>(data_model_type));

            for (auto &script_info: it->second) {
                try {
                    schedule_script(data_model_type, script_info);
                } catch (const std::exception &e) {
                    LOG_ERROR("Failed to schedule script '{}' for mod '{}': {}",
                              script_info.pattern, mod_context.mod_name, e.what());
                }
            }
        }
    }

    void ScriptManager::reload_mod_scripts(const std::string &mod_name) {
        LOG_INFO("Reloading scripts for mod: {}", mod_name);

        std::unique_lock lock(m_scripts_mutex);

        const auto it = std::ranges::find_if(m_loaded_mods,
                                             [&mod_name](const ModScriptContext &context) {
                                                 return context.mod_name == mod_name;
                                             });

        if (it == m_loaded_mods.end()) {
            LOG_WARN("Mod '{}' not found for reload", mod_name);
            return;
        }

        const auto mod_config = config::mod(mod_name);
        if (!mod_config) {
            LOG_ERROR("Failed to get config for mod: {}", mod_name);
            return;
        }

        const auto mod_directory = it->mod_path;
        m_loaded_mods.erase(it);

        lock.unlock();
        load_mod_scripts_for_context(mod_directory, *mod_config);

        LOG_INFO("Successfully reloaded scripts for mod: {}", mod_name);
    }

    std::vector<ScriptInfo> ScriptManager::resolve_script_patterns(
        const std::filesystem::path &scripts_directory,
        const std::vector<std::string> &patterns) {
        std::vector<ScriptInfo> scripts;

        for (const auto &pattern: patterns) {
            for (const auto matching_files = find_matching_files(scripts_directory, {pattern}); const auto &file_path:
                 matching_files) {
                ScriptInfo script_info;
                script_info.pattern = pattern;
                script_info.full_path = file_path;
                script_info.content = load_script_content(file_path);

                if (!script_info.content.empty()) {
                    scripts.push_back(std::move(script_info));
                }
            }
        }

        return scripts;
    }

    std::string ScriptManager::load_script_content(const std::filesystem::path &script_path) {
        try {
            std::ifstream file(script_path, std::ios::binary);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open script file: {}", script_path.string());
                return {};
            }

            std::string content;
            file.seekg(0, std::ios::end);
            content.reserve(file.tellg());
            file.seekg(0, std::ios::beg);

            content.assign(std::istreambuf_iterator(file),
                           std::istreambuf_iterator<char>());

            LOG_DEBUG("Loaded script: {} ({} bytes)",
                      script_path.string(), content.size());

            return content;
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to load script '{}': {}", script_path.string(), e.what());
            return {};
        }
    }

    bool ScriptManager::matches_pattern(const std::filesystem::path &file_path, const std::string &pattern) {
        const auto normalized_path = file_path.generic_string();

        std::string regex_pattern;
        bool in_double_star = false;

        for (size_t i = 0; i < pattern.length(); ++i) {
            switch (const auto c = pattern[i]) {
                case '.':
                case '^':
                case '$':
                case '+':
                case '{':
                case '}':
                case '[':
                case ']':
                case '(':
                case ')':
                case '\\':
                case '|':
                    regex_pattern += '\\';
                    regex_pattern += c;
                    break;

                case '*':
                    if (i + 1 < pattern.length() && pattern[i + 1] == '*') {
                        regex_pattern += ".*";
                        in_double_star = true;
                        ++i;
                    } else {
                        regex_pattern += "[^/]*";
                    }
                    break;

                case '?':
                    regex_pattern += "[^/]";
                    break;

                case '/':
                    if (in_double_star) {
                        in_double_star = false;
                    }
                    regex_pattern += '/';
                    break;

                default:
                    regex_pattern += c;
                    break;
            }
        }

        try {
            const std::regex pattern_regex(regex_pattern, std::regex_constants::icase);
            return std::regex_match(normalized_path, pattern_regex);
        } catch (const std::exception &e) {
            LOG_WARN("Invalid pattern '{}': {}", pattern, e.what());
            return false;
        }
    }

    std::vector<std::filesystem::path> ScriptManager::find_matching_files(
        const std::filesystem::path &directory,
        const std::vector<std::string> &patterns) {
        std::vector<std::filesystem::path> matching_files;

        if (!std::filesystem::exists(directory)) {
            LOG_WARN("Directory does not exist: {}", directory.string());
            return matching_files;
        }

        try {
            for (const auto &entry: std::filesystem::recursive_directory_iterator(directory)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                const auto &file_path = entry.path();
                const auto relative_path = std::filesystem::relative(file_path, directory);

                if (const auto extension = file_path.extension().string();
                    extension != ".lua" && extension != ".luau") {
                    continue;
                }

                for (const auto &pattern: patterns) {
                    if (matches_pattern(relative_path, pattern)) {
                        matching_files.push_back(file_path);
                        break;
                    }
                }
            }
        } catch (const std::exception &e) {
            LOG_ERROR("Error scanning directory '{}': {}", directory.string(), e.what());
        }

        return matching_files;
    }

    void ScriptManager::schedule_script(const RBX::DataModelType data_model_type,
                                        const ScriptInfo &script_info) {
        if (!g_task_scheduler) {
            LOG_ERROR("TaskScheduler not available, cannot schedule script: {}",
                      script_info.full_path.string());
            return;
        }

        const auto engine = g_task_scheduler->get_script_engine(data_model_type);
        if (!engine) {
            return;
        }

        const auto chunk_name = script_info.full_path.filename().string();

        LOG_DEBUG("Scheduling script via ScriptEngine: {}", script_info.full_path.string());

        execute_script_async(engine, script_info, chunk_name);
    }

    void ScriptManager::execute_script_async(const std::shared_ptr<ScriptEngine> &engine,
                                             const ScriptInfo &script_info,
                                             const std::string &chunk_name) noexcept {
        if (!engine) [[unlikely]] {
            log_script_error(script_info, "Script engine is null");
            return;
        }

        try {
            auto future = engine->execute_script(
                script_info.content,
                chunk_name
            );

            std::thread execution_thread([future = std::move(future), script_info]() mutable {
                try {
                    const auto result = future.get();
                    handle_script_result(script_info, result);
                } catch (const std::exception &e) {
                    log_script_error(script_info, std::format("Exception during execution: {}", e.what()));
                } catch (...) {
                    log_script_error(script_info, "Unknown exception during execution");
                }
            });

            execution_thread.detach();
        } catch (const std::exception &e) {
            log_script_error(script_info, std::format("Failed to schedule script: {}", e.what()));
        } catch (...) {
            log_script_error(script_info, "Unknown error during script scheduling");
        }
    }

    void ScriptManager::log_script_success(const ScriptInfo &script_info) noexcept {
        try {
            LOG_DEBUG("Script '{}' executed successfully",
                      script_info.full_path.string());
        } catch (...) {
            std::fprintf(stderr, "Script executed successfully: %s\n",
                         script_info.full_path.string().c_str());
        }
    }

    void ScriptManager::log_script_error(const ScriptInfo &script_info,
                                         std::string_view error_message) noexcept {
        try {
            LOG_ERROR("Script '{}': {}",
                      script_info.full_path.string(), error_message);
        } catch (...) {
            std::fprintf(stderr, "Critical logging error for script: %s\n",
                         script_info.full_path.string().c_str());
        }
    }
}
