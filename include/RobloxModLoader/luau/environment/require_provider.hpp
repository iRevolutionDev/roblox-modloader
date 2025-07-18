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
        inline std::unordered_map<std::string, int> g_module_cache;
        inline int g_original_require_ref = LUA_REFNIL;
    }
}
