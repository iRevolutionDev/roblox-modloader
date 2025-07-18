#pragma once

#include "RobloxModLoader/common.hpp"
#include "globals_registry.hpp"

namespace rml::luau::environment {
    class RMLProvider final : public GlobalProvider<RMLProvider> {
    public:
        static constexpr std::string_view NAME = "rml";

        struct ModContext {
            std::string mod_name;
            std::string mod_version;
            std::string mod_description;
            std::string mod_author;
            std::filesystem::path mod_path;
            std::vector<std::string> mod_dependencies;
        };

        bool register_globals(lua_State *L) noexcept override;

        static void set_mod_context(lua_State *L, const ModContext &context) noexcept;

        static void register_logger_table(lua_State *L) noexcept;

        static void register_mod_table(lua_State *L) noexcept;

        static void register_utilities_table(lua_State *L) noexcept;

    private:
        static void register_core_namespace(lua_State *L) noexcept;
    };

    namespace rml_logger_impl {
        int info(lua_State *L);

        int warn(lua_State *L);

        int error(lua_State *L);

        int debug(lua_State *L);
    }

    namespace rml_mod_impl {
        int get_name(lua_State *L);

        int get_version(lua_State *L);

        int get_description(lua_State *L);

        int get_author(lua_State *L);

        int get_path(lua_State *L);

        int get_dependencies(lua_State *L);
    }

    namespace rml_utils_impl {
        int format_string(lua_State *L);

        int get_timestamp(lua_State *L);
    }
}
