#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/environment/closures_provider.hpp"
#include "RobloxModLoader/luau/environment/environment_context.hpp"
#include "RobloxModLoader/luau/extensions/luau_extensions.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "lmem.h"

#include <luau/Compiler.h>
#include <array>

namespace rml::luau::environment {
    namespace {
        constexpr std::array<const char *, 19> VALID_METAMETHODS = {
            "__index", "__newindex", "__namecall", "__call", "__add", "__sub", "__mul",
            "__div", "__mod", "__pow", "__unm", "__eq", "__lt", "__le",
            "__concat", "__len", "__tostring", "__metatable", "__mode"
        };

        constexpr Luau::CompileOptions LOADSTRING_COMPILE_OPTS{1, 2};
        constexpr Luau::CompileOptions CLOSURE_COMPILE_OPTS{0, 0};
    }

    ClosuresProvider::ClosuresProvider() = default;

    ClosuresProvider::~ClosuresProvider() = default;

    bool ClosuresProvider::register_globals(lua_State *L) noexcept {
        try {
            LuaFunctionRegistry::register_function_safe(L, "iscclosure", check_c_closure);
            LuaFunctionRegistry::register_function_safe(L, "islclosure", check_luau_closure);
            LuaFunctionRegistry::register_function_safe(L, "clonefunction", duplicate_function);
            LuaFunctionRegistry::register_function_safe(L, "newcclosure", create_c_closure_wrapper);
            LuaFunctionRegistry::register_function_safe(L, "newlclosure", create_luau_closure_wrapper);
            LuaFunctionRegistry::register_function_safe(L, "isexecutorclosure", check_executor_closure);
            LuaFunctionRegistry::register_function_safe(L, "loadstring", compile_and_load_code);
            LuaFunctionRegistry::register_function_safe(L, "hookfunction", hook_target_function);
            LuaFunctionRegistry::register_function_safe(L, "ishooked", check_hook_status);
            LuaFunctionRegistry::register_function_safe(L, "restorefunction", restore_original_function);
            LuaFunctionRegistry::register_function_safe(L, "hookmetamethod", hook_metamethod_function);
            LuaFunctionRegistry::register_function_safe(L, "is_protected", check_is_protected_status);
            LuaFunctionRegistry::register_function_safe(L, "protect_function", set_protected_function);
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to register standard closures provider: {}", e.what());
            return false;
        }
    }

    bool ClosuresProvider::register_roblox_globals(lua_State *L) noexcept {
        try {
            if (!register_globals(L)) {
                return false;
            }

            LuaFunctionRegistry::register_function_safe(L, "getgc", collect_garbage_objects);
            LuaFunctionRegistry::register_function_safe(L, "getnamecallmethod", get_namecall_method);

            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to register Roblox closures provider: {}", e.what());
            return false;
        }
    }

    std::shared_ptr<EnvironmentContext> ClosuresProvider::get_environment_context_for_state(lua_State *L) {
        return get_environment_context(L);
    }

    bool ClosuresProvider::validate_metamethod_name(const char *method_name) {
        for (const auto &metamethod: VALID_METAMETHODS) {
            if (strcmp(metamethod, method_name) == 0) {
                return true;
            }
        }
        return false;
    }

    bool ClosuresProvider::ensure_context_available(lua_State *L, const char *function_name) {
        if (get_environment_context_for_state(L)) {
            return true;
        }

        luaL_error(L, "%s: environment context not available", function_name);
        return false;
    }

    void ClosuresProvider::handle_hook_error(lua_State *L, const char *function_name, const char *error) {
        LOG_ERROR("{}: {}", function_name, error);
        luaL_error(L, "%s: %s", function_name, error);
    }

    int ClosuresProvider::check_c_closure(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_pushboolean(L, lua_iscfunction(L, 1));
        return 1;
    }

    int ClosuresProvider::check_luau_closure(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_pushboolean(L, !lua_iscfunction(L, 1));
        return 1;
    }

    int ClosuresProvider::check_executor_closure(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);

        const auto closure = luau_to_mutable_closure(L, 1);
        const auto context = get_environment_context_for_state(L);

        bool is_executor_closure = false;
        if (context) {
            is_executor_closure = context->is_self_closure(closure) || context->is_wrapped_closure(closure);
        }

        lua_pushboolean(L, is_executor_closure);
        return 1;
    }

    Closure *ClosuresProvider::clone_c_closure(lua_State *L, Closure *original) {
        Closure *clone = luaF_newCclosure(L, original->nupvalues, original->env);

        if (original->c.debugname != nullptr) {
            clone->c.debugname = original->c.debugname;
        }

        for (int i = 0; i < original->nupvalues; i++) {
            setobj2n(L, &clone->c.upvals[i], &original->c.upvals[i]);
        }

        clone->c.f = original->c.f;
        clone->c.cont = original->c.cont;

        return clone;
    }

    void ClosuresProvider::copy_closure_data(lua_State *L, Closure *dest, Closure *source) {
        dest->env = source->env;
        dest->stacksize = source->stacksize;
        dest->preload = source->preload;
        dest->nupvalues = source->nupvalues;

        if (lua_iscfunction(L, -1)) {
            dest->c.f = source->c.f;
            dest->c.cont = source->c.cont;
            for (int i = 0; i < source->nupvalues; i++) {
                setobj2n(L, &dest->c.upvals[i], &source->c.upvals[i]);
            }
        } else {
            dest->l.p = source->l.p;
            for (int i = 0; i < source->nupvalues; i++) {
                setobj2n(L, &dest->l.uprefs[i], &source->l.uprefs[i]);
            }
        }
    }

    int ClosuresProvider::duplicate_function(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        if (!lua_iscfunction(L, 1)) {
            lua_clonefunction(L, 1);
            return 1;
        }
        const auto original_closure = luau_to_mutable_closure(L, 1);
        Closure *cloned_closure = clone_c_closure(L, original_closure);

        setclvalue(L, L->top, cloned_closure);
        L->top++;
        if (const auto context = get_environment_context_for_state(L)) {
            if (const auto original_wrapped = context->get_original(original_closure); original_wrapped.has_value()) {
                context->register_new_c_closure(cloned_closure, original_wrapped.value());
            }
        }

        return 1;
    }

    int ClosuresProvider::cclosure_stub_handler(lua_State *L) {
        auto context = get_environment_context_for_state(L);
        if (!context) {
            handle_hook_error(L, "newcclosure", "environment context not available");
        }

        const auto current_closure = clvalue(L->ci->func);
        const auto referenced_object = context->get_original(current_closure);

        if (!referenced_object.has_value()) {
            handle_hook_error(L, "newcclosure", "closure mapping not found internally");
        }

        const auto original_closure = referenced_object.value().get_referenced_object(L);
        if (!original_closure.has_value()) {
            handle_hook_error(L, "newcclosure", "invalid closure reference");
        }

        const auto argument_count = lua_gettop(L);

        // Setup call to original function
        setclvalue(L, L->top, original_closure.value());
        L->top++;
        lua_insert(L, 1);

        L->ci->flags |= LUA_CALLINFO_HANDLE;
        L->baseCcalls++;

        const auto call_status = static_cast<lua_Status>(lua_pcall(L, argument_count, LUA_MULTRET, 0));
        L->baseCcalls--;
        if (call_status == LUA_OK && (L->status == LUA_YIELD || L->status == LUA_BREAK)) {
            return lua_yield(L, 0);
        }
        if (call_status != LUA_OK && call_status != LUA_YIELD) {
            const auto error_message = lua_tostring(L, -1);
            if (error_message && strcmp(error_message, "attempt to yield across metamethod/C-call boundary") == 0) {
                return lua_yield(L, 0);
            }
        }

        if (call_status != LUA_OK) {
            lua_error(L);
        }

        return lua_gettop(L);
    }

    int ClosuresProvider::cclosure_continuation_handler(lua_State *L, int status) {
        if (status == LUA_OK) {
            return lua_gettop(L);
        }
        lua_error(L);
        return 0;
    }

    int ClosuresProvider::create_c_closure_wrapper(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        const auto debug_name = luaL_optstring(L, 2, nullptr);

        lua_pushcclosurek(L, cclosure_stub_handler, debug_name, 0, cclosure_continuation_handler);
        if (const auto context = get_environment_context_for_state(L)) {
            const auto reference_id = lua_ref(L, 1);
            context->register_new_c_closure(
                luau_to_mutable_closure(L, -1),
                ReferencedLuauObject<Closure *, LUA_TFUNCTION>{reference_id}
            );
        }

        return 1;
    }

    int ClosuresProvider::create_luau_closure_wrapper(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        if (!lua_iscfunction(L, 1)) {
            lua_ref(L, -1);
        }
        lua_newtable(L);
        lua_newtable(L);
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfield(L, -2, "__index");
        lua_setreadonly(L, -1, true);
        lua_setmetatable(L, -2);
        lua_pushvalue(L, -2);
        lua_setfield(L, -2, "stub");
        constexpr auto wrapper_code = "return stub(...)";
        const auto bytecode = Luau::compile(wrapper_code, CLOSURE_COMPILE_OPTS);

        if (luau_load(L, "=rml_closure_wrapper", bytecode.c_str(), bytecode.size(), -1) != LUA_OK) {
            handle_hook_error(L, "newlclosure", "failed to compile wrapper function");
        }

        lua_remove(L, lua_gettop(L) - 1);
        return 1;
    }

    int ClosuresProvider::compile_and_load_code(lua_State *L) {
        const auto source_code = luaL_checkstring(L, 1);
        const auto chunk_name = luaL_optstring(L, 2, "=loadstring");

        const auto bytecode = Luau::compile(source_code, LOADSTRING_COMPILE_OPTS);

        if (luau_load(L, chunk_name, bytecode.c_str(), bytecode.size(), 0) != LUA_OK) {
            lua_pushnil(L);
            lua_pushvalue(L, -2);
            return 2;
        }
        if (const auto context = get_environment_context_for_state(L)) {
            context->register_self_closure(luau_to_mutable_closure(L, -1));
        }

        lua_setsafeenv(L, LUA_GLOBALSINDEX, false);
        return 1;
    }

    int ClosuresProvider::check_is_protected_status(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        const auto closure = luau_to_mutable_closure(L, 1);

        bool is_protected = false;
        if (const auto context = get_environment_context_for_state(L)) {
            is_protected = context->is_protected(closure);
        }

        lua_pushboolean(L, is_protected);
        return 1;
    }

    int ClosuresProvider::set_protected_function(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        const auto closure = luau_to_mutable_closure(L, 1);

        if (auto context = get_environment_context_for_state(L)) {
            if (!context->is_protected(closure)) {
                context->protect_function(closure);
            }
        }

        return 0;
    }

    int ClosuresProvider::check_hook_status(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        const auto closure = luau_to_mutable_closure(L, 1);

        bool is_hooked = false;
        if (const auto context = get_environment_context_for_state(L)) {
            is_hooked = context->is_hooked(closure);
        }

        lua_pushboolean(L, is_hooked);
        return 1;
    }

    int ClosuresProvider::hook_target_function(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        luaL_checktype(L, 2, LUA_TFUNCTION);
        luau_limit_stack(L, 2);

        auto context = get_environment_context_for_state(L);
        if (!context) {
            handle_hook_error(L, "hookfunction", "failed to get environment context");
        }

        const auto target_closure = luau_to_mutable_closure(L, 1);
        if (context->is_protected(target_closure)) {
            handle_hook_error(L, "hookfunction", "this function cannot be hooked from Luau");
        }
        luau_prepare_gc_push(L, 2);
        lua_pushcclosure(L, duplicate_function, nullptr, 0);
        lua_pushvalue(L, 1);
        lua_pcall(L, 1, 1, 0);
        const auto original_backup = luau_to_mutable_closure(L, -1);

        luau_prepare_gc_push(L, 2);
        lua_pushcclosure(L, duplicate_function, nullptr, 0);
        lua_pushvalue(L, 1);
        lua_pcall(L, 1, 1, 0);
        const auto hook_replacement_ref = lua_ref(L, 2);
        if (!context->is_hooked(target_closure)) {
            HookInformation hook_info;
            hook_info.original = ReferencedLuauObject<Closure *, LUA_TFUNCTION>{lua_ref(L, -1)};
            hook_info.hooked_with = ReferencedLuauObject<Closure *, LUA_TFUNCTION>{hook_replacement_ref};
            context->register_hook(target_closure, hook_info);
        }

        lua_pop(L, 2);
        if (lua_iscfunction(L, 1)) {
            const auto replacement_closure = luau_to_mutable_closure(L, 2);
            if (!lua_iscfunction(L, 2) || !context->is_wrapped_closure(replacement_closure)) {
                luau_prepare_gc_push(L, 2);
                lua_pushcclosure(L, create_c_closure_wrapper, nullptr, 0);
                lua_pushvalue(L, 2);
                lua_call(L, 1, 1);
            } else {
                luau_check_stack(L, 1);
                lua_pushvalue(L, 2);
            }
            if (context->is_wrapped_closure(target_closure)) {
                const auto final_replacement = luau_to_mutable_closure(L, -1);

                if (context->is_wrapped_closure(final_replacement)) {
                    const auto target_wrapped = context->get_original(target_closure);
                    const auto replacement_wrapped = context->get_original(final_replacement);

                    if (target_wrapped.has_value() && replacement_wrapped.has_value()) {
                        auto hook_info = context->get_hook(target_closure);
                        if (hook_info.has_value()) {
                            hook_info->hooked_with = replacement_wrapped.value();
                            hook_info->hooked_type = FunctionKind::NewCClosure;
                            hook_info->hook_with_type = FunctionKind::NewCClosure;
                            context->register_hook(target_closure, hook_info.value());
                            context->register_new_c_closure(target_closure, replacement_wrapped.value());
                        }
                    }

                    luau_prepare_gc_push(L, 1);
                    L->top->value.p = original_backup;
                    L->top->tt = LUA_TFUNCTION;
                    L->top++;

                    if (const auto target_wrapped = context->get_original(target_closure); target_wrapped.has_value()) {
                        context->register_new_c_closure(luau_to_mutable_closure(L, -1), target_wrapped.value());
                    }
                    return 1;
                }
            } else {
                const auto final_replacement = luau_to_mutable_closure(L, -1);
                const auto replacement_wrapped = context->get_original(final_replacement);

                if (replacement_wrapped.has_value()) {
                    auto hook_info = context->get_hook(target_closure);
                    if (hook_info.has_value()) {
                        hook_info->hooked_with = replacement_wrapped.value();
                        hook_info->hooked_type = FunctionKind::CClosure;
                        hook_info->hook_with_type = FunctionKind::NewCClosure;
                        context->register_hook(target_closure, hook_info.value());
                    }
                    copy_closure_data(L, target_closure, final_replacement);
                    context->register_new_c_closure(target_closure, replacement_wrapped.value());

                    luau_prepare_gc_push(L, 1);
                    L->top->value.p = original_backup;
                    L->top->tt = LUA_TFUNCTION;
                    L->top++;
                    return 1;
                }
            }
        }
        if (!lua_iscfunction(L, 1)) {
            const auto replacement_closure = luau_to_mutable_closure(L, 2);
            bool wrapped_replacement = false;

            if (lua_iscfunction(L, 2) || (replacement_closure->nupvalues > target_closure->nupvalues)) {
                luau_prepare_gc_push(L, 2);
                lua_pushcclosure(L, create_luau_closure_wrapper, nullptr, 0);
                lua_pushvalue(L, 2);
                lua_call(L, 1, 1);
                wrapped_replacement = true;
            }

            const auto final_replacement = luau_to_mutable_closure(L, wrapped_replacement ? -1 : 2);
            copy_closure_data(L, target_closure, final_replacement);

            auto hook_info = context->get_hook(target_closure);
            if (hook_info.has_value()) {
                hook_info->hooked_with = ReferencedLuauObject<Closure *, LUA_TFUNCTION>(
                    lua_ref(L, wrapped_replacement ? -1 : 2));
                hook_info->hooked_type = FunctionKind::LuauClosure;
                hook_info->hook_with_type = FunctionKind::LuauClosure;
                context->register_hook(target_closure, hook_info.value());
            }

            luau_prepare_gc_push(L, 1);
            L->top->value.p = original_backup;
            L->top->tt = LUA_TFUNCTION;
            L->top++;
            return 1;
        }

        handle_hook_error(L, "hookfunction", "operation not supported for this function type");
        return 0;
    }

    int ClosuresProvider::restore_original_function(lua_State *L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        const auto target_closure = luau_to_mutable_closure(L, 1);

        auto context = get_environment_context_for_state(L);
        if (!context || !context->is_hooked(target_closure)) {
            return 0;
        }

        auto hook_info = context->get_hook(target_closure);
        if (!hook_info.has_value()) {
            return 0;
        }

        const auto original_closure = hook_info->original.get_referenced_object(L);
        if (!original_closure.has_value()) {
            handle_hook_error(L, "restorefunction", "cannot obtain original function to restore");
        }

        switch (hook_info->hook_with_type) {
            case FunctionKind::NewCClosure: {
                context->register_new_c_closure(target_closure, hook_info->original);
                hook_info->hooked_with.unreference_object(L);
                break;
            }
            case FunctionKind::CClosure: {
                copy_closure_data(L, target_closure, original_closure.value());
                context->unregister_new_c_closure(target_closure);
                break;
            }
            case FunctionKind::LuauClosure: {
                copy_closure_data(L, target_closure, original_closure.value());
                break;
            }
        }
        hook_info->original.unreference_object(L);
        hook_info->hooked_with.unreference_object(L);
        context->unregister_hook(target_closure);

        return 0;
    }

    int ClosuresProvider::hook_metamethod_function(lua_State *L) {
        luaL_checkany(L, 1);
        const auto metamethod_name = luaL_checkstring(L, 2);
        luaL_checktype(L, 3, LUA_TFUNCTION);
        if (!validate_metamethod_name(metamethod_name)) {
            luaL_argerror(L, 2, std::format("invalid metafield '{}'", metamethod_name).c_str());
        }
        if (!lua_getmetatable(L, 1)) {
            luaL_argerror(L, 1, "object has no metatable");
        }

        lua_setreadonly(L, -1, false);
        if (!luaL_getmetafield(L, 1, metamethod_name)) {
            lua_setreadonly(L, -2, true);
            luaL_argerror(L, 1, "metafield does not exist");
        }

        if (!lua_isfunction(L, -1)) {
            lua_setreadonly(L, -3, true);
            luaL_argerror(L, 2, "metafield is not a function");
        }
        lua_pushcfunction(L, hook_target_function, "hookfunction");
        lua_pushvalue(L, -2);
        lua_pushvalue(L, 3);
        lua_call(L, 2, 1);

        lua_setreadonly(L, -3, true);
        return 1;
    }

    int ClosuresProvider::collect_garbage_objects(lua_State *L) {
        const bool include_tables = luaL_optboolean(L, 1, false);
        luau_limit_stack(L, 1);
        luau_prepare_gc_push(L, 1);
        lua_newtable(L);

        struct GCCollectionContext {
            lua_State *lua_state;
            bool access_tables;
            int items_found;
        };

        auto gc_context = GCCollectionContext{L, include_tables, 0};

        const auto old_threshold = L->global->GCthreshold;
        L->global->GCthreshold = SIZE_MAX;

        luaM_visitgco(L, &gc_context, [](void *ctx, lua_Page *page, GCObject *gc_obj) -> bool {
            const auto collection_ctx = static_cast<GCCollectionContext *>(ctx);
            const auto ctx_lua = collection_ctx->lua_state;

            if (isdead(ctx_lua->global, gc_obj)) {
                return false;
            }

            const auto obj_type = gc_obj->gch.tt;
            const bool should_include = (obj_type < LUA_TPROTO && obj_type >= LUA_TSTRING && obj_type != LUA_TTABLE) ||
                                        (obj_type == LUA_TTABLE && collection_ctx->access_tables);

            if (should_include) {
                luau_prepare_gc_push(ctx_lua, 1);
                ctx_lua->top->value.gc = gc_obj;
                ctx_lua->top->tt = obj_type;
                ctx_lua->top++;

                const auto index = collection_ctx->items_found++;
                lua_rawseti(ctx_lua, -2, index + 1);
            }

            return false;
        });

        L->global->GCthreshold = old_threshold;
        return 1;
    }

    int ClosuresProvider::get_namecall_method(lua_State *L) {
        const auto namecall_method = lua_namecallatom(L, nullptr);

        if (!namecall_method) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, namecall_method);
        }

        return 1;
    }
}
