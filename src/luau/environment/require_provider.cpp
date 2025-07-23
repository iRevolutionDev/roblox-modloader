#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/environment/require_provider.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "utils/directory_utils.hpp"
#include "pointers.hpp"

#include "lobject.h"
#include "lstate.h"
#include "ltable.h"


namespace rml::luau::environment {
    namespace require_impl {
        StateCache &get_cache_for_state(lua_State *L) {
            std::lock_guard lock(g_cache_mutex);
            return g_state_caches[L];
        }

        void cleanup_cache_for_state(lua_State *L) {
            std::lock_guard lock(g_cache_mutex);
            if (const auto it = g_state_caches.find(L); it != g_state_caches.end()) {
                for (const auto &ref: it->second.module_cache | std::views::values) {
                    if (ref != LUA_NOREF) {
                        lua_unref(L, ref);
                    }
                }
                if (it->second.original_require_ref != LUA_NOREF) {
                    lua_unref(L, it->second.original_require_ref);
                }
                g_state_caches.erase(it);
                LOG_DEBUG("Cleaned up require cache for lua_State: {}", static_cast<void*>(L));
            }
        }
    }

    bool RequireProvider::register_globals(lua_State *L) noexcept {
        try {
            auto &[module_cache, original_require_ref] = require_impl::get_cache_for_state(L);

            lua_getglobal(L, "require");
            if (lua_isfunction(L, -1)) {
                // THIS IS SO BAD, BUT I DON'T KNOW HOW TO DO IT BETTER
                // TODO: Find a better way to use lua_ref without crash
                if (const auto *g = L->global; g->registryfree != 0) {
                    auto *reg = hvalue(registry(L));
                    TValue *slot = luaH_setnum(L, reg, g->registryfree);
                    setnvalue(slot, 0);
                }
                original_require_ref = lua_ref(L, -1);

                if (original_require_ref == LUA_NOREF) {
                    LOG_WARN("Failed to store original require function reference for lua_State: {}",
                             static_cast<void*>(L));
                }

                lua_pop(L, 1);
            } else {
                lua_pop(L, 1);
                LOG_WARN("Original require function not found or not a function for lua_State: {}",
                         static_cast<void*>(L));
            }
            lua_pushcfunction(L, custom_require, "require");
            lua_setglobal(L, "require");

            LOG_DEBUG("RequireProvider registered for lua_State: {}", static_cast<void*>(L));
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to register RequireProvider globals: {}", e.what());
            return false;
        }
    }

    int RequireProvider::custom_require(lua_State *L) {
        try {
            auto &[module_cache, original_require_ref] = require_impl::get_cache_for_state(L);

            // Check if is the roblox instance
            if (lua_gettop(L) < 1 || lua_isuserdata(L, 1)) {
                return original_require(L);
            }

            if (lua_gettop(L) < 1 || !lua_isstring(L, 1)) {
                luaL_error(L, "require() expects a string argument");
                return 0;
            }

            const std::string module_name = lua_tostring(L, 1);

            if (const auto cache_it = module_cache.find(module_name);
                cache_it != module_cache.end()) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, cache_it->second);
                LOG_DEBUG("Found cached module '{}' for lua_State: {}", module_name, static_cast<void*>(L));
                return 1;
            }

            std::string resolved_path;

            if (module_name.starts_with("@rml/")) {
                resolved_path = resolve_rml_path(module_name.substr(5));
            } else if (module_name.starts_with("@self/")) {
                resolved_path = resolve_self_path(module_name.substr(6), L);
            } else {
                resolved_path = resolve_custom_path(module_name, L);

                if (resolved_path.empty()) {
                    return original_require(L);
                }
            }

            if (resolved_path.empty() || !file_exists(resolved_path)) {
                if (module_name.starts_with("@self/")) {
                    lua_getglobal(L, "_RML_MOD_CONTEXT");
                    std::string mod_path = "Unknown";
                    if (lua_istable(L, -1)) {
                        lua_getfield(L, -1, "path");
                        if (lua_isstring(L, -1)) {
                            mod_path = lua_tostring(L, -1);
                        }
                        lua_pop(L, 1);
                    }
                    lua_pop(L, 1);

                    const std::string expected_path = std::format("{}/scripts/{}.{lua,luau}",
                                                                  mod_path, module_name.substr(6), mod_path,
                                                                  module_name.substr(6));
                    luaL_error(L, "Module '%s' not found. Expected at: %s", module_name.c_str(), expected_path.c_str());
                    return 0;
                }
                return original_require(L);
            }

            const std::string source_code = read_file_contents(resolved_path);
            if (source_code.empty()) {
                luaL_error(L, "Failed to read module file: %s", resolved_path.c_str());
                return 0;
            }

            constexpr auto compilation_opts = Luau::CompileOptions{
                .optimizationLevel = 1,
                .debugLevel = 2,
            };

            ScriptContext::set_thread_identity(L, RBX::Security::Permissions::RobloxEngine,
                                               RBX::Security::FULL_CAPABILITIES);

            const auto bytecode = Luau::compile(source_code, compilation_opts);
            if (bytecode.empty()) {
                luaL_error(L, "Failed to compile module: %s", resolved_path.c_str());
                return 0;
            }

            const std::string chunk_name = std::format("={}", module_name);

            if (g_pointers->m_roblox_pointers.luau_load(L, chunk_name.c_str(), bytecode.data(), bytecode.size(), 0) !=
                0) {
                lua_error(L);
                return 0;
            }

            ScriptContext::elevate_closure(
                static_cast<const Closure *>(lua_topointer(L, -1)),
                RBX::Security::FULL_CAPABILITIES
            );

            if (lua_pcall(L, 0, 1, 0) != 0) {
                lua_error(L);
                return 0;
            }

            lua_pushvalue(L, -1);
            const int ref = lua_ref(L, -1);
            module_cache[module_name] = ref;

            LOG_DEBUG("Cached module '{}' with ref {} for lua_State: {}", module_name, ref, static_cast<void*>(L));

            return 1;
        } catch (const std::exception &e) {
            lua_pushstring(L, std::format("Error in custom require: {}", e.what()).c_str());
            lua_error(L);
            return 0;
        }
    }

    int RequireProvider::original_require(lua_State *L) {
        auto &cache = require_impl::get_cache_for_state(L);

        if (cache.original_require_ref == LUA_NOREF) {
            luaL_error(L, "Original require function not available");
            return 0;
        }
        lua_rawgeti(L, LUA_REGISTRYINDEX, cache.original_require_ref);

        lua_insert(L, 1);

        const int nargs = lua_gettop(L) - 1;
        if (lua_pcall(L, nargs, 1, 0) != 0) {
            lua_error(L);
            return 0;
        }

        return 1;
    }

    std::string RequireProvider::resolve_custom_path(const std::string &module_name, lua_State *L) {
        if (auto self_relative = resolve_self_path(module_name, L);
            !self_relative.empty() && file_exists(self_relative)) {
            return self_relative;
        }

        for (const std::vector<std::string> extensions = {".lua", ".luau", ""}; const auto &ext: extensions) {
            if (std::string full_path = module_name + ext; file_exists(full_path)) {
                return normalize_path(full_path);
            }
        }

        return "";
    }

    std::string RequireProvider::resolve_rml_path(const std::string &module_name) {
        try {
            const auto exe_dir = directory_utils::get_executable_directory();
            const auto rml_dir = exe_dir / "RobloxModLoader" / "libraries";

            for (const std::vector<std::string> extensions = {".lua", ".luau"}; const auto &ext: extensions) {
                if (std::filesystem::path full_path = rml_dir / (module_name + ext);
                    std::filesystem::exists(full_path)) {
                    return normalize_path(full_path.string());
                }
            }

            return "";
        } catch (const std::exception &e) {
            LOG_ERROR("Error resolving @rml/ path for '{}': {}", module_name, e.what());
            return "";
        }
    }

    std::string RequireProvider::resolve_self_path(const std::string &module_name, lua_State *L) {
        try {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                return "";
            }

            lua_getfield(L, -1, "path");
            if (!lua_isstring(L, -1)) {
                lua_pop(L, 2);
                LOG_WARN("Mod path not available in context for @self/ resolution");
                return "";
            }

            const auto mod_path = lua_tostring(L, -1);
            lua_pop(L, 2);

            const auto mod_dir = std::filesystem::path(mod_path);

            for (const std::vector<std::string> extensions = {".lua", ".luau"}; const auto &ext: extensions) {
                std::filesystem::path full_path = mod_dir / "scripts" / (module_name + ext);
                if (std::filesystem::exists(full_path)) {
                    return normalize_path(full_path.string());
                }
            }

            return "";
        } catch (const std::exception &e) {
            LOG_ERROR("Error resolving @self/ path for '{}': {}", module_name, e.what());
            return "";
        }
    }

    std::string RequireProvider::normalize_path(const std::string &path) {
        try {
            return std::filesystem::canonical(path).string();
        } catch (const std::exception &) {
            return path;
        }
    }

    bool RequireProvider::file_exists(const std::string &path) {
        try {
            return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
        } catch (const std::exception &) {
            return false;
        }
    }

    std::string RequireProvider::read_file_contents(const std::string &path) {
        try {
            const std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) {
                LOG_ERROR("Failed to open file: {}", path);
                return "";
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        } catch (const std::exception &e) {
            LOG_ERROR("Error reading file '{}': {}", path, e.what());
            return "";
        }
    }
}
