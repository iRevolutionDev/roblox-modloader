#pragma once

#include "globals_registry.hpp"
#include <unordered_map>
#include <string>

namespace rml::luau::environment {
    class RequireProvider final : public GlobalProvider<RequireProvider> {
    public:
        static constexpr std::string_view NAME = "require_provider";

        bool register_globals(lua_State *L) noexcept override;

    private:
        static int custom_require(lua_State *L);

        static int original_require(lua_State *L);

        static std::string resolve_custom_path(const std::string &module_name, lua_State *L);

        static std::string resolve_rml_path(const std::string &module_name);

        static std::string resolve_self_path(const std::string &module_name, lua_State *L);

        static std::string normalize_path(const std::string &path);

        static bool file_exists(const std::string &path);

        static std::string read_file_contents(const std::string &path);
    };

    namespace require_impl {
        struct StateCache {
            std::unordered_map<std::string, int> module_cache;
            int original_require_ref = LUA_NOREF;
        };

        inline std::unordered_map<lua_State *, StateCache> g_state_caches;
        inline std::mutex g_cache_mutex;

        StateCache &get_cache_for_state(lua_State *L);

        void cleanup_cache_for_state(lua_State *L);
    }
}
