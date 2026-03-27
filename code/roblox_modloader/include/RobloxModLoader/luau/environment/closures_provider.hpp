#pragma once

#include "RobloxModLoader/common.hpp"
#include "environment_context.hpp"
#include <memory>

struct Closure;
struct Proto;

namespace rml::luau::environment {
    class ClosuresProvider {
    public:
        static constexpr std::string_view NAME = "closures";

        ClosuresProvider();

        ~ClosuresProvider();

        static bool register_globals(lua_State *L) noexcept;

        static bool register_roblox_globals(lua_State *L) noexcept;

    private:
        static int check_c_closure(lua_State *L);

        static int check_luau_closure(lua_State *L);

        static int check_executor_closure(lua_State *L);

        static int duplicate_function(lua_State *L);

        static int create_c_closure_wrapper(lua_State *L);

        static int create_luau_closure_wrapper(lua_State *L);

        static int compile_and_load_code(lua_State *L);

        static int hook_target_function(lua_State *L);

        static int check_hook_status(lua_State *L);

        static int restore_original_function(lua_State *L);

        static int hook_metamethod_function(lua_State *L);

        static int check_is_protected_status(lua_State *L);

        static int set_protected_function(lua_State *L);

        static int collect_garbage_objects(lua_State *L);

        static int get_namecall_method(lua_State *L);

        static int cclosure_stub_handler(lua_State *L);

        static int cclosure_continuation_handler(lua_State *L, int status);

        static std::shared_ptr<EnvironmentContext> get_environment_context_for_state(lua_State *L);

        static bool validate_metamethod_name(const char *method_name);

        static Closure *clone_c_closure(lua_State *L, Closure *original);

        static void copy_closure_data(lua_State *L, Closure *dest, Closure *source);

        static void handle_hook_error(lua_State *L, const char *function_name, const char *error);

        static bool ensure_context_available(lua_State *L, const char *function_name);
    };
}
