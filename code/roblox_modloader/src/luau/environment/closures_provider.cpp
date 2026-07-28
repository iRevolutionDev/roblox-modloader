#include "RobloxModLoader/luau/environment/closures_provider.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/luau/environment/environment_context.hpp"
#include "RobloxModLoader/luau/extensions/luau_extensions.hpp"
#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "pointers.hpp"

#include <Luau/Compiler.h>

#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"

RML_LOG_SCOPE("ClosuresProvider");

namespace rml::luau::environment
{
	namespace closures_impl
	{
		struct HeapWalk
		{
			lua_State* state;
			const access::GlobalState* collector;
			LuaTable* into;
			bool include_tables;
			int found;
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

		static void put_object(lua_State* L, LuaTable* holder, const int index, void* object,
		                       const std::uint8_t tag)
		{
			auto* slot = access::value(luaH_setnum(L, holder, index));
			slot->value = reinterpret_cast<std::uint64_t>(object);
			slot->tt = tag;

			luaC_barriertable(L, holder, static_cast<GCObject*>(object));
		}

		static void unwrap_holder(lua_State* L)
		{
			lua_rawgeti(L, -1, 1);
			lua_insert(L, -2);
			lua_pop(L, 1);
		}

		static void* heap_object_behind(void* reported, const std::uint8_t tag)
		{
			auto* object = tag == LUA_TUSERDATA ? access::object_behind_payload(reported) : reported;

			return access::gc_header(object)->tt == tag ? object : nullptr;
		}

		static void take_heap_node(void* context, void* reported, const std::uint8_t tag, std::uint8_t,
		                           std::size_t, const char*)
		{
			auto* walk = static_cast<HeapWalk*>(context);
			if (reported == nullptr || !collectable_on_the_stack(tag, walk->include_tables))
			{
				return;
			}

			auto* object = heap_object_behind(reported, tag);
			if (object == nullptr || access::swept_away(walk->collector, object))
			{
				return;
			}

			put_object(walk->state, walk->into, ++walk->found, object, tag);
		}

		static void skip_heap_edge(void*, void*, void*, const char*)
		{
		}

		int getgc(lua_State* L)
		{
			const auto enumerate = g_pointers->m_roblox_pointers.luaC_enumheap;
			if (enumerate == nullptr)
			{
				luaL_error(L, "getgc: this Studio build did not give up its heap walk");
			}

			const auto include_tables = luaL_optboolean(L, 1, 0) != 0;

			HeapWalk walk{L, access::state(L)->global, new_object_holder(L), include_tables, 0};
			enumerate(L, &walk, take_heap_node, skip_heap_edge);

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

			const auto load = g_pointers->m_roblox_pointers.luau_load;
			if (load == nullptr ||
			    load(L, "=rml_closure_wrapper", bytecode.data(), bytecode.size(), -1) != LUA_OK)
			{
				luaL_error(L, "newlclosure: the wrapper chunk could not be loaded");
			}

			lua_insert(L, -2);
			lua_pop(L, 1);
		}

		int newlclosure(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);
			push_luau_wrapper(L, 1);

			return 1;
		}

		static void copy_upvalues(access::Closure* destination, const access::Closure* source)
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
			auto* clone = access::closure(luaF_newLclosure(L, source->nupvalues,
			                                               reinterpret_cast<LuaTable*>(source->env),
			                                               reinterpret_cast<Proto*>(source->p)));

			clone->stacksize = source->stacksize;
			clone->preload = source->preload;
			copy_upvalues(clone, source);

			return clone;
		}

		int iscclosure(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);
			lua_pushboolean(L, closure_argument(L, 1)->isC != 0);
			return 1;
		}

		int islclosure(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);
			lua_pushboolean(L, closure_argument(L, 1)->isC == 0);
			return 1;
		}

		int clonefunction(lua_State* L)
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

		int hookfunction(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);
			luaL_checktype(L, 2, LUA_TFUNCTION);
			refuse_c_target(L, "hookfunction");

			const auto context = get_environment_context(L);
			if (!context)
			{
				luaL_error(L, "hookfunction: the environment context is not available");
			}

			auto* target = closure_argument(L, 1);

			if (context->is_protected(reinterpret_cast<Closure*>(target)))
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

			if (!context->is_hooked(reinterpret_cast<Closure*>(target)))
			{
				HookInformation hook;
				hook.original = ReferencedLuauObject<Closure*, LUA_TFUNCTION>{lua_ref(L, -1)};
				hook.hooked_with = ReferencedLuauObject<Closure*, LUA_TFUNCTION>{lua_ref(L, replacement_index)};
				hook.hooked_type = FunctionKind::LuauClosure;
				hook.hook_with_type = FunctionKind::LuauClosure;
				context->register_hook(reinterpret_cast<Closure*>(target), hook);
			}

			target->env = replacement->env;
			target->stacksize = replacement->stacksize;
			target->preload = replacement->preload;
			target->nupvalues = replacement->nupvalues;
			target->p = replacement->p;
			copy_upvalues(target, replacement);

			luaC_barrierf(L, reinterpret_cast<GCObject*>(target), reinterpret_cast<GCObject*>(replacement));

			return 1;
		}

		int restorefunction(lua_State* L)
		{
			luaL_checktype(L, 1, LUA_TFUNCTION);

			const auto context = get_environment_context(L);
			if (!context)
			{
				luaL_error(L, "restorefunction: the environment context is not available");
			}

			auto* target = closure_argument(L, 1);
			const auto hook = context->get_hook(reinterpret_cast<Closure*>(target));

			if (!hook.has_value())
			{
				luaL_error(L, "restorefunction: this function was never hooked");
			}

			const auto original = hook->original.get_referenced_object(L);
			if (!original.has_value())
			{
				luaL_error(L, "restorefunction: the original function is no longer referenced");
			}

			const auto* source = access::closure(original.value());

			target->env = source->env;
			target->stacksize = source->stacksize;
			target->preload = source->preload;
			target->nupvalues = source->nupvalues;
			target->p = source->p;
			copy_upvalues(target, source);

			luaC_barrierf(L, reinterpret_cast<GCObject*>(target),
			              reinterpret_cast<GCObject*>(const_cast<access::Closure*>(source)));

			context->unregister_hook(reinterpret_cast<Closure*>(target));

			return 0;
		}
	}

	bool ClosuresProvider::register_globals(lua_State* L) noexcept
	{
		try
		{
			register_function(L, "getgc", closures_impl::getgc);
			register_function(L, "iscclosure", closures_impl::iscclosure);
			register_function(L, "islclosure", closures_impl::islclosure);
			register_function(L, "clonefunction", closures_impl::clonefunction);
			register_function(L, "newlclosure", closures_impl::newlclosure);
			register_function(L, "hookfunction", closures_impl::hookfunction);
			register_function(L, "restorefunction", closures_impl::restorefunction);
			return true;
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Failed to register the closures provider: {}", e.what());
			return false;
		}
	}
}
