#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

struct Closure;
struct lua_State;

namespace rml::luau {
    class ScriptEngine;
}

namespace rml::luau::environment {
    enum class FunctionKind : std::uint8_t {
        CClosure = 0,
        LuauClosure = 1,
        NewCClosure = 2
    };

    template<typename T, lua_Type Type>
    class ReferencedLuauObject {
    public:
        ReferencedLuauObject() : ref_id(LUA_NOREF) {
        }

        explicit ReferencedLuauObject(int ref) : ref_id(ref) {
        }

        ~ReferencedLuauObject() = default;

        std::optional<T> get_referenced_object(lua_State *L) const {
            if (ref_id == LUA_NOREF) return std::nullopt;

            lua_getref(L, ref_id);
            if (lua_type(L, -1) != Type) {
                lua_pop(L, 1);
                return std::nullopt;
            }

            T result = reinterpret_cast<T>(const_cast<void *>(lua_topointer(L, -1)));
            lua_pop(L, 1);
            return result;
        }

        void unreference_object(lua_State *L) {
            if (ref_id != LUA_NOREF) {
                lua_unref(L, ref_id);
                ref_id = LUA_NOREF;
            }
        }

        bool is_valid() const { return ref_id != LUA_NOREF; }

    private:
        int ref_id;
    };

    struct HookInformation {
        ReferencedLuauObject<Closure *, LUA_TFUNCTION> original;
        ReferencedLuauObject<Closure *, LUA_TFUNCTION> hooked_with;
        FunctionKind hooked_type;
        FunctionKind hook_with_type;
    };

    class EnvironmentContext final {
    public:
        explicit EnvironmentContext(std::shared_ptr<ScriptEngine> parentEngine);

        ~EnvironmentContext();

        EnvironmentContext(const EnvironmentContext &) = delete;

        EnvironmentContext &operator=(const EnvironmentContext &) = delete;

        EnvironmentContext(EnvironmentContext &&) = delete;

        EnvironmentContext &operator=(EnvironmentContext &&) = delete;

        void register_hook(Closure *target, const HookInformation &info);

        void unregister_hook(Closure *target);

        bool is_hooked(Closure *target) const;

        std::optional<HookInformation> get_hook(Closure *target) const;

        void register_new_c_closure(Closure *wrapper, const ReferencedLuauObject<Closure *, LUA_TFUNCTION> &original);

        void unregister_new_c_closure(Closure *wrapper);

        bool is_wrapped_closure(Closure *wrapper) const;

        std::optional<ReferencedLuauObject<Closure *, LUA_TFUNCTION> > get_original(Closure *wrapper) const;

        void protect_function(Closure *closure);

        bool is_protected(Closure *closure) const;

        void register_self_closure(Closure *closure);

        bool is_self_closure(Closure *closure) const;

        void destroy_context();

        bool is_destroyed() const;

        std::shared_ptr<ScriptEngine> get_engine() const;

    private:
        std::shared_ptr<ScriptEngine> m_parent_engine_;

        mutable std::shared_mutex m_mutex;
        std::atomic<bool> m_is_destroyed_{false};

    public:
        std::unordered_map<Closure *, HookInformation> m_function_hooks;
        std::unordered_map<Closure *, ReferencedLuauObject<Closure *, LUA_TFUNCTION> > m_newcclosures;
        std::unordered_set<Closure *> m_protected_closures;
        std::unordered_set<Closure *> m_our_closures;
    };

    std::shared_ptr<EnvironmentContext> get_environment_context(lua_State *L);
}
