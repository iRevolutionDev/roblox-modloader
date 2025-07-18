#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/environment/rml_provider.hpp"

namespace rml::luau::environment {
    namespace rml_logger_impl {
        int info(lua_State *L) {
            try {
                if (lua_gettop(L) < 1 || !lua_isstring(L, 2)) {
                    lua_pushstring(L, "Expected string argument (message)");
                    lua_error(L);
                    return 0;
                }

                const auto message = lua_tostring(L, 2);
                lua_getglobal(L, "rml");
                lua_getfield(L, -1, "mod");
                lua_getfield(L, -1, "name");
                std::string mod_name = "Unknown";
                if (lua_isstring(L, -1)) {
                    mod_name = lua_tostring(L, -1);
                }
                lua_pop(L, 3);

                LOG_INFO("[{}] {}", mod_name, message);
                return 0;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error in logger.info: {}", e.what()).c_str());
                lua_error(L);
                return 0;
            }
        }

        int warn(lua_State *L) {
            try {
                if (lua_gettop(L) < 1 || !lua_isstring(L, 2)) {
                    lua_pushstring(L, "Expected string argument (message)");
                    lua_error(L);
                }

                const auto message = lua_tostring(L, 2);

                lua_getglobal(L, "rml");
                lua_getfield(L, -1, "mod");
                lua_getfield(L, -1, "name");

                std::string mod_name = "Unknown";
                if (lua_isstring(L, -1)) {
                    mod_name = lua_tostring(L, -1);
                }

                lua_pop(L, 3);

                LOG_WARN("[{}] {}", mod_name, message);
                return 0;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error in logger.warn: {}", e.what()).c_str());
                lua_error(L);
            }
        }

        int error(lua_State *L) {
            try {
                if (lua_gettop(L) < 1 || !lua_isstring(L, 2)) {
                    lua_pushstring(L, "Expected string argument (message)");
                    lua_error(L);
                }

                const auto message = lua_tostring(L, 2);

                lua_getglobal(L, "rml");
                lua_getfield(L, -1, "mod");
                lua_getfield(L, -1, "name");

                std::string mod_name = "Unknown";
                if (lua_isstring(L, -1)) {
                    mod_name = lua_tostring(L, -1);
                }

                lua_pop(L, 3);

                LOG_ERROR("[{}] {}", mod_name, message);
                return 0;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error in logger.error: {}", e.what()).c_str());
                lua_error(L);
            }
        }

        int debug(lua_State *L) {
            try {
                if (lua_gettop(L) < 1 || !lua_isstring(L, 2)) {
                    lua_pushstring(L, "Expected string argument (message)");
                    lua_error(L);
                }

                const auto message = lua_tostring(L, 2);

                lua_getglobal(L, "rml");
                lua_getfield(L, -1, "mod");
                lua_getfield(L, -1, "name");

                std::string mod_name = "Unknown";
                if (lua_isstring(L, -1)) {
                    mod_name = lua_tostring(L, -1);
                }

                lua_pop(L, 3);

                LOG_DEBUG("[{}] {}", mod_name, message);
                return 0;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error in logger.debug: {}", e.what()).c_str());
                lua_error(L);
            }
        }
    }

    namespace rml_mod_impl {
        int get_name(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "name");
            return 1;
        }

        int get_version(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "version");
            return 1;
        }

        int get_description(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "description");
            return 1;
        }

        int get_author(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "author");
            return 1;
        }

        int get_path(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "path");
            return 1;
        }

        int get_dependencies(lua_State *L) {
            lua_getglobal(L, "_RML_MOD_CONTEXT");
            lua_getfield(L, -1, "dependencies");
            return 1;
        }
    }

    namespace rml_utils_impl {
        int format_string(lua_State *L) {
            try {
                const int argc = lua_gettop(L);
                if (argc < 1 || !lua_isstring(L, 1)) {
                    lua_pushstring(L, "Expected format string as first argument");
                    lua_error(L);
                }

                const auto format_str = lua_tostring(L, 1);

                std::string result = format_str;
                size_t pos = 0;

                for (int i = 2; i <= argc; ++i) {
                    pos = result.find("{}", pos);
                    if (pos == std::string::npos) break;

                    std::string replacement;
                    if (lua_isstring(L, i)) {
                        replacement = lua_tostring(L, i);
                    } else if (lua_isnumber(L, i)) {
                        replacement = std::to_string(lua_tonumber(L, i));
                    } else if (lua_isboolean(L, i)) {
                        replacement = lua_toboolean(L, i) ? "true" : "false";
                    } else if (lua_isnil(L, i)) {
                        replacement = "nil";
                    } else {
                        replacement = "[object]";
                    }

                    result.replace(pos, 2, replacement);
                    pos += replacement.length();
                }

                lua_pushstring(L, result.c_str());
                return 1;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error in format_string: {}", e.what()).c_str());
                lua_error(L);
            }
        }

        int get_timestamp(lua_State *L) {
            try {
                const auto now = std::chrono::system_clock::now();
                const auto time_t = std::chrono::system_clock::to_time_t(now);

                std::stringstream ss;
                ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

                lua_pushstring(L, ss.str().c_str());
                return 1;
            } catch (const std::exception &e) {
                lua_pushstring(L, std::format("Error getting timestamp: {}", e.what()).c_str());
                lua_error(L);
            }
        }
    }

    bool RMLProvider::register_globals(lua_State *L) noexcept {
        try {
            register_core_namespace(L);
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to register RML globals: {}", e.what());
            return false;
        }
    }

    void RMLProvider::register_core_namespace(lua_State *L) noexcept {
        lua_newtable(L);

        register_logger_table(L);

        register_mod_table(L);

        register_utilities_table(L);

        lua_setglobal(L, NAME.data());
    }

    void RMLProvider::register_logger_table(lua_State *L) noexcept {
        lua_newtable(L);

        constexpr luaL_Reg logger_funcs[] = {
            {"info", rml_logger_impl::info},
            {"warn", rml_logger_impl::warn},
            {"error", rml_logger_impl::error},
            {"debug", rml_logger_impl::debug},
            {nullptr, nullptr}
        };

        luaL_register(L, nullptr, logger_funcs);
        lua_setfield(L, -2, "logger");
    }

    void RMLProvider::register_mod_table(lua_State *L) noexcept {
        lua_newtable(L);
        lua_pushstring(L, "Unknown");
        lua_setfield(L, -2, "name");

        lua_pushstring(L, "0.0.0");
        lua_setfield(L, -2, "version");

        lua_pushstring(L, "");
        lua_setfield(L, -2, "description");

        lua_pushstring(L, "Unknown");
        lua_setfield(L, -2, "author");

        lua_pushstring(L, "");
        lua_setfield(L, -2, "path");

        // lua_newtable(L);
        // lua_setfield(L, -2, "dependencies");

        lua_setfield(L, -2, "mod");
    }

    void RMLProvider::register_utilities_table(lua_State *L) noexcept {
        lua_newtable(L);

        constexpr luaL_Reg utils_funcs[] = {
            {"format", rml_utils_impl::format_string},
            {"timestamp", rml_utils_impl::get_timestamp},
            {nullptr, nullptr}
        };

        luaL_register(L, nullptr, utils_funcs);
        lua_setfield(L, -2, "utils");
    }

    void RMLProvider::set_mod_context(lua_State *L, const ModContext &context) noexcept {
        try {
            lua_newtable(L);

            lua_pushstring(L, context.mod_name.c_str());
            lua_setfield(L, -2, "name");

            lua_pushstring(L, context.mod_version.c_str());
            lua_setfield(L, -2, "version");

            lua_pushstring(L, context.mod_description.c_str());
            lua_setfield(L, -2, "description");

            lua_pushstring(L, context.mod_author.c_str());
            lua_setfield(L, -2, "author");

            lua_pushstring(L, context.mod_path.string().c_str());
            lua_setfield(L, -2, "path");

            // lua_newtable(L);
            // for (size_t i = 0; i < context.mod_dependencies.size(); ++i) {
            //     lua_pushstring(L, context.mod_dependencies[i].c_str());
            //     lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
            // }
            // lua_setfield(L, -2, "dependencies");

            lua_setglobal(L, "_RML_MOD_CONTEXT");

            lua_getglobal(L, "rml");
            if (lua_istable(L, -1)) {
                lua_getfield(L, -1, "mod");
                if (lua_istable(L, -1)) {
                    lua_pushstring(L, context.mod_name.c_str());
                    lua_setfield(L, -2, "name");
                    lua_pushstring(L, context.mod_version.c_str());
                    lua_setfield(L, -2, "version");
                    lua_pushstring(L, context.mod_description.c_str());
                    lua_setfield(L, -2, "description");
                    lua_pushstring(L, context.mod_author.c_str());
                    lua_setfield(L, -2, "author");

                    lua_pushstring(L, context.mod_path.string().c_str());
                    lua_setfield(L, -2, "path");

                    // TODO: Future support for dependencies
                    // lua_newtable(L);
                    // for (size_t i = 0; i < context.mod_dependencies.size(); ++i) {
                    //     lua_pushstring(L, context.mod_dependencies[i].c_str());
                    //     lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
                    // }
                    // lua_setfield(L, -2, "dependencies");
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to set mod context: {}", e.what());
        }
    }
}
