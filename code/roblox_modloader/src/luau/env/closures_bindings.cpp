#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/luau/env/binding.hpp"
#include "RobloxModLoader/luau/env/closure_registry.hpp"
#include "RobloxModLoader/luau/extensions/luau_extensions.hpp"
#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "RobloxModLoader/luau/script_host.hpp"
#include "RobloxModLoader/luau/vm/chunk.hpp"
#include "RobloxModLoader/luau/vm/stack_guard.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"
#include "pointers.hpp"

#include <Luau/Compiler.h>

RML_LOG_SCOPE("ClosuresBindings");

namespace rml::luau
{
	struct HeapWalk
	{
		const access::GlobalState* collector;
		bool include_tables;
		std::vector<std::pair<void*, std::uint8_t>> objects;
	};

	static bool collectable_on_the_stack(const std::uint8_t tag, const bool include_tables)
	{
		if (tag == LUA_TTABLE)
		{
			return include_tables;
		}

		return tag >= LUA_TSTRING && tag <= LUA_TBUFFER;
	}

	static LuaTable* new_object_holder(lua_State* L)
	{
		lua_createtable(L, 1, 0);
		return static_cast<LuaTable*>(const_cast<void*>(lua_topointer(L, -1)));
	}

	static void put_object(lua_State* L, LuaTable* holder, const int index, void* object, const std::uint8_t tag)
	{
		auto* slot = access::value(luaH_setnum(L, holder, index));
		slot->value = reinterpret_cast<std::uint64_t>(object);
		slot->tt = tag;

		if (access::needs_barrier(holder, object))
		{
			luaC_barriertable(L, holder, static_cast<GCObject*>(object));
		}
	}

	static void unwrap_holder(lua_State* L)
	{
		lua_rawgeti(L, -1, 1);
		lua_insert(L, -2);
		lua_pop(L, 1);
	}

	static bool take_heap_object(void* context, void*, void* object)
	{
		auto* walk = static_cast<HeapWalk*>(context);
		if (object == nullptr)
		{
			return false;
		}

		const auto tag = access::gc_header(object)->tt;
		if (!collectable_on_the_stack(tag, walk->include_tables) || access::swept_away(walk->collector, object))
		{
			return false;
		}

		walk->objects.emplace_back(object, tag);

		return false;
	}

	static int closures_getgc(lua_State* L)
	{
		const auto visit = g_pointers->m_roblox_pointers.luaM_visitgco;
		if (visit == nullptr)
		{
			luaL_error(L, "getgc: this Studio build did not give up its heap walk");
		}

		const auto include_tables = luaL_optboolean(L, 1, 0) != 0;
		auto* holder = new_object_holder(L);

		HeapWalk walk{access::state(L)->global, include_tables, {}};
		walk.objects.reserve(8192);

		visit(L, &walk, take_heap_object);

		int index = 0;
		for (const auto& [object, tag] : walk.objects)
		{
			put_object(L, holder, ++index, object, tag);
		}

		return 1;
	}

	static access::Closure* closure_argument(lua_State* L, const int index)
	{
		return access::closure(const_cast<void*>(lua_topointer(L, index)));
	}

	static void refuse_c_target(lua_State* L, const char* name)
	{
		if (lua_iscfunction(L, 1))
		{
			luaL_error(L,
			    "%s cannot take a C closure as its target on this Studio build: the fields such a "
			    "closure keeps live in a union the generated layout does not mirror yet",
			    name);
		}
	}

	static void push_luau_wrapper(lua_State* L, const int index)
	{
		lua_pushvalue(L, index);

		lua_newtable(L);
		lua_newtable(L);
		lua_pushvalue(L, LUA_GLOBALSINDEX);
		lua_setfield(L, -2, "__index");
		lua_setreadonly(L, -1, true);
		lua_setmetatable(L, -2);

		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "stub");

		static constexpr Luau::CompileOptions options{0, 0};
		const auto bytecode = Luau::compile("return stub(...)", options);

		if (const auto loaded = vm::load_chunk(L, "rml_closure_wrapper", std::as_bytes(std::span{bytecode}), RBX::Security::FULL_CAPABILITIES, -1); !loaded)
		{
			const auto message = loaded.error().describe();
			luaL_error(L, "newlclosure: the wrapper chunk could not be loaded (%s)", message.c_str());
		}

		lua_insert(L, -2);
		lua_pop(L, 1);
	}

	static int closures_newlclosure(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);
		push_luau_wrapper(L, 1);

		return 1;
	}

	static void copy_upvalues(const access::Closure* destination, const access::Closure* source)
	{
		auto* from = access::upvalues_of(source);
		auto* to = access::upvalues_of(destination);

		for (int i = 0; i < source->nupvalues; ++i)
		{
			access::copy_value(access::value_at(to, i), access::value_at(from, i));
		}
	}

	static access::Closure* clone_lua_closure(lua_State* L, const access::Closure* source)
	{
		auto* clone = access::closure(luaF_newLclosure(L,
		    source->nupvalues,
		    reinterpret_cast<LuaTable*>(source->env),
		    reinterpret_cast<Proto*>(source->p)));

		clone->stacksize = source->stacksize;
		clone->preload = source->preload;
		copy_upvalues(clone, source);

		return clone;
	}

	static int closures_iscclosure(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);
		lua_pushboolean(L, closure_argument(L, 1)->isC != 0);
		return 1;
	}

	static int closures_islclosure(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);
		lua_pushboolean(L, closure_argument(L, 1)->isC == 0);
		return 1;
	}

	static int closures_clonefunction(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);

		if (lua_iscfunction(L, 1))
		{
			luaL_error(L, "clonefunction only handles Luau closures on this Studio build");
		}

		auto* holder = new_object_holder(L);
		put_object(L, holder, 1, clone_lua_closure(L, closure_argument(L, 1)), LUA_TFUNCTION);
		unwrap_holder(L);

		return 1;
	}

	static int closures_hookfunction(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);
		luaL_checktype(L, 2, LUA_TFUNCTION);
		refuse_c_target(L, "hookfunction");

		auto& env = bound_env(L);
		auto& closures = env.closures();
		auto* target = closure_argument(L, 1);

		if (closures.is_protected(reinterpret_cast<Closure*>(target)))
		{
			luaL_error(L, "hookfunction: this function is protected from being hooked");
		}

		auto* replacement = closure_argument(L, 2);
		auto replacement_index = 2;

		if (lua_iscfunction(L, 2) || replacement->nupvalues > target->nupvalues)
		{
			push_luau_wrapper(L, 2);
			replacement = closure_argument(L, -1);
			replacement_index = lua_gettop(L);
		}

		auto* holder = new_object_holder(L);
		put_object(L, holder, 1, clone_lua_closure(L, target), LUA_TFUNCTION);
		unwrap_holder(L);

		if (!closures.is_hooked(reinterpret_cast<Closure*>(target)))
		{
			auto* const anchor = env.host().global_state();

			closures.register_hook(reinterpret_cast<Closure*>(target),
			    HookRecord{
			        .target = vm::Ref::take(L, 1, anchor),
			        .original = vm::Ref::take(L, -1, anchor),
			        .replacement = vm::Ref::take(L, replacement_index, anchor),
			        .original_kind = FunctionKind::LuauClosure,
			        .replacement_kind = FunctionKind::LuauClosure,
			        .owner = std::string{env.mod().mod_name()},
			    });
		}

		target->env = replacement->env;
		target->stacksize = replacement->stacksize;
		target->preload = replacement->preload;
		target->nupvalues = replacement->nupvalues;
		target->p = replacement->p;
		copy_upvalues(target, replacement);

		if (access::needs_barrier(target, replacement))
		{
			luaC_barrierf(L, reinterpret_cast<GCObject*>(target), reinterpret_cast<GCObject*>(replacement));
		}

		return 1;
	}

	bool restore_closure(lua_State* L, const HookRecord& record)
	{
		vm::StackGuard stack(L);

		if (!record.target.push(L) || lua_type(L, -1) != LUA_TFUNCTION)
		{
			return false;
		}

		auto* target = access::closure(const_cast<void*>(lua_topointer(L, -1)));

		if (!record.original.push(L) || lua_type(L, -1) != LUA_TFUNCTION)
		{
			return false;
		}

		const auto* source = access::closure(lua_topointer(L, -1));

		target->env = source->env;
		target->stacksize = source->stacksize;
		target->preload = source->preload;
		target->nupvalues = source->nupvalues;
		target->p = source->p;
		copy_upvalues(target, source);

		if (access::needs_barrier(target, source))
		{
			luaC_barrierf(L, reinterpret_cast<GCObject*>(target), reinterpret_cast<GCObject*>(const_cast<access::Closure*>(source)));
		}

		return true;
	}

	static int closures_restorefunction(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);

		auto& closures = bound_env(L).closures();
		auto* target = closure_argument(L, 1);
		const auto* record = closures.hook_for(reinterpret_cast<Closure*>(target));

		if (record == nullptr)
		{
			luaL_error(L, "restorefunction: this function was never hooked");
		}

		if (!restore_closure(L, *record))
		{
			luaL_error(L, "restorefunction: the original function is no longer referenced");
		}

		closures.unregister_hook(reinterpret_cast<Closure*>(target));

		return 0;
	}

	bool bind_closures(ScriptEnv& env, lua_State* L) noexcept
	{
		set_bound_global(env, L, "getgc", &closures_getgc);
		set_bound_global(env, L, "iscclosure", &closures_iscclosure);
		set_bound_global(env, L, "islclosure", &closures_islclosure);
		set_bound_global(env, L, "clonefunction", &closures_clonefunction);
		set_bound_global(env, L, "newlclosure", &closures_newlclosure);
		set_bound_global(env, L, "hookfunction", &closures_hookfunction);
		set_bound_global(env, L, "restorefunction", &closures_restorefunction);

		return true;
	}
}
