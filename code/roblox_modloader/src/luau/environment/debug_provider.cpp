#include "RobloxModLoader/luau/environment/debug_provider.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/luau/extensions/luau_extensions.hpp"
#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "lapi.h"
#include "ldebug.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"

#include <array>

namespace rml::luau::environment
{
	namespace debug_impl
	{
		static void barrier_into(lua_State* L, void* owner, const access::TValue* value)
		{
			if (value->tt < LUA_TSTRING)
			{
				return;
			}

			auto* object = reinterpret_cast<void*>(value->value);

			if (access::needs_barrier(owner, object))
			{
				luaC_barrierf(L, static_cast<GCObject*>(owner), static_cast<GCObject*>(object));
			}
		}

		static void normalize_stack(lua_State* L, const int count)
		{
			if (const int top = lua_gettop(L); top < count)
			{
				for (int i = top; i < count; ++i)
				{
					lua_pushnil(L);
				}
			}
			else if (top > count)
			{
				lua_settop(L, count);
			}
		}

		int getconstants(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 1);
				normalize_stack(L, 1);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "Expected function or number for argument #1");
				}

				if (lua_isnumber(L, 1))
				{
					lua_Debug dbgInfo{};
					const int level = lua_tointeger(L, 1);

					if (!lua_getinfo(L, level, "f", &dbgInfo))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				namespace access = rml::luau::access;

				const auto* closure   = access::closure(luau_to_closure(L, -1));
				const auto* proto     = access::proto(closure->p);
				const auto constCount = proto->sizek;
				const auto* consts    = access::value(proto->k);
				auto* state           = access::state(L);

				lua_newtable(L);

				for (int i = 0; i < constCount; i++)
				{
					const auto* tval = access::value_at(consts, i);

					if (tval->tt == LUA_TFUNCTION)
					{
						lua_pushnil(L);
					}
					else
					{
						barrier_into(L, L, tval);
						access::copy_value(state->top, tval);
						access::advance_top(state);
					}

					lua_rawseti(L, -2, i + 1);
				}

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getconstants: {}", e.what()).c_str());
				lua_error(L);
			}
		}

		int getconstant(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 2);
				normalize_stack(L, 2);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "Expected function or number for argument #1");
				}

				const int constantIndex = luaL_checkinteger(L, 2);

				if (lua_isnumber(L, 1))
				{
					lua_Debug dbgInfo{};
					if (!lua_getinfo(L, lua_tointeger(L, 1), "f", &dbgInfo))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				namespace access = rml::luau::access;

				const auto* closure   = access::closure(luau_to_closure(L, -1));
				const auto* proto     = access::proto(closure->p);
				const auto* constants = access::value(proto->k);

				if (constantIndex < 1)
				{
					luaL_argerror(L, 2, "constant index starts at 1");
				}

				if (constantIndex > proto->sizek)
				{
					luaL_argerror(L, 2, "constant index is out of range");
				}

				const auto* tValue = access::value_at(constants, constantIndex - 1);

				if (tValue->tt == LUA_TFUNCTION)
				{
					lua_pushnil(L);
				}
				else
				{
					barrier_into(L, L, tValue);

					auto* state = access::state(L);
					access::copy_value(state->top, tValue);
					access::advance_top(state);
				}

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getconstant: {}", e.what()).c_str());
				lua_error(L);
				return 0;
			}
		}

		int setconstant(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 1);
				luaL_checknumber(L, 2);
				luaL_checkany(L, 3);

				normalize_stack(L, 3);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "function or level expected");
				}

				const int index = luaL_checkinteger(L, 2);

				if (lua_isnumber(L, 1))
				{
					lua_Debug ar;
					if (!lua_getinfo(L, lua_tointeger(L, 1), "f", &ar))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				namespace access = rml::luau::access;

				const auto* cl = access::closure(luau_to_closure(L, -1));
				const auto* p  = access::proto(cl->p);
				auto* k        = access::value(p->k);

				if (index < 1)
				{
					luaL_argerror(L, 2, "constant index starts at 1");
				}

				if (index > p->sizek)
				{
					luaL_argerror(L, 2, "constant index out of range");
				}

				auto* constant = access::value_at(k, index - 1);

				if (constant->tt == LUA_TFUNCTION)
				{
					return 0;
				}

				const auto* newConstant = access::value(luaA_toobject(L, 3));

				if (newConstant->tt != constant->tt)
				{
					luaL_argerror(L, 3, "cannot replace constant when the element you want to replace it with is not of the same type.");
				}

				barrier_into(L, const_cast<access::Proto*>(p), newConstant);

				access::copy_value(constant, newConstant);

				return 0;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.setconstant: {}", e.what()).c_str());
				lua_error(L);
				return 0;
			}
		}

		int getinfo(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 1);
				normalize_stack(L, 1);
				auto infoLevel = 0;

				if (lua_isnumber(L, 1))
				{
					infoLevel = static_cast<int>(lua_tointeger(L, 1));
					luaL_argcheck(L, infoLevel >= 0, 1, "level cannot be negative");
				}
				else if (lua_isfunction(L, 1))
				{
					infoLevel = -lua_gettop(L);
				}
				else
				{
					luaL_argerror(L, 1, "function or level expected");
				}

				alignas(16) std::array<std::byte, 1024> storage{};
				const auto* record = rml::luau::access::debug_record(storage.data());

				if (!lua_getinfo(L, infoLevel, "fulasn", reinterpret_cast<lua_Debug*>(storage.data())))
				{
					luaL_argerror(L, 1, "invalid level");
				}

				const auto text = [](const char* value) { return value != nullptr ? value : ""; };

				lua_newtable(L);

				lua_pushstring(L, text(record->source));
				lua_setfield(L, -2, "source");

				lua_pushstring(L, text(record->short_src));
				lua_setfield(L, -2, "short_src");

				lua_pushvalue(L, 1);
				lua_setfield(L, -2, "func");

				lua_pushstring(L, text(record->what));
				lua_setfield(L, -2, "what");

				lua_pushinteger(L, record->currentline);
				lua_setfield(L, -2, "currentline");

				lua_pushinteger(L, record->linedefined);
				lua_setfield(L, -2, "linedefined");

				lua_pushstring(L, text(record->name));
				lua_setfield(L, -2, "name");

				lua_pushinteger(L, record->nupvals);
				lua_setfield(L, -2, "nups");

				lua_pushinteger(L, record->nparams);
				lua_setfield(L, -2, "numparams");

				lua_pushinteger(L, record->isvararg);
				lua_setfield(L, -2, "is_vararg");

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getinfo: {}", e.what()).c_str());
				lua_error(L);
				return 0;
			}
		}

		int getproto(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 1);
				luaL_checktype(L, 2, LUA_TNUMBER);
				const bool active = luaL_optboolean(L, 3, true);
				normalize_stack(L, 3);

				if (!active)
				{
					luaL_argerror(L, 3, "prototypes cannot be inactive (not implemented)");
				}

				if (!lua_isnumber(L, 1) && !lua_isfunction(L, 1))
				{
					luaL_argerror(L, 1, "function or level expected");
				}

				if (lua_isnumber(L, 1))
				{
					const int level = static_cast<int>(lua_tointeger(L, 1));
					lua_Debug ar;
					if (!lua_getinfo(L, level, "f", &ar))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					luaL_checktype(L, 1, LUA_TFUNCTION);
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerrorL(L, 1, "Lua function expected.");
				}

				luaL_error(L, "debug.getproto needs Proto.nups, which the dumper has not recovered yet");

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getproto: {}", e.what()).c_str());
				lua_error(L);
				return 0;
			}
		}

		int getprotos(lua_State* L)
		{
			try
			{
				luaL_checkany(L, 1);
				normalize_stack(L, 1);

				if (!lua_isnumber(L, 1) && !lua_isfunction(L, 1))
				{
					luaL_argerror(L, 1, "function or level expected");
				}

				if (lua_isnumber(L, 1))
				{
					const int level = static_cast<int>(lua_tointeger(L, 1));
					lua_Debug ar;
					if (!lua_getinfo(L, level, "f", &ar))
					{
						luaL_error(L, "level out of range");
					}
				}
				else
				{
					luaL_checktype(L, 1, LUA_TFUNCTION);
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerrorL(L, 1, "Lua function expected.");
				}

				luaL_error(L, "debug.getprotos needs Proto.nups, which the dumper has not recovered yet");

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getprotos: {}", e.what()).c_str());
				lua_error(L);
			}
		}

		int setstack(lua_State* L)
		{
			try
			{
				luaL_checktype(L, 1, LUA_TNUMBER);
				luaL_checktype(L, 2, LUA_TNUMBER);
				luaL_checkany(L, 3);
				normalize_stack(L, 3);

				const auto level = lua_tointeger(L, 1);
				const auto index = lua_tointeger(L, 2);

				namespace access = rml::luau::access;

				auto* state = access::state(L);

				if (level >= access::frames_between(state->ci, state->base_ci) || level < 0)
				{
					luaL_argerror(L, 1, "level out of range");
				}

				auto* stackFrame = access::frame_at(state->ci, -static_cast<int>(level));
				const auto stackSize = (reinterpret_cast<std::byte*>(stackFrame->top) -
				                        reinterpret_cast<std::byte*>(stackFrame->base)) /
				                       static_cast<std::ptrdiff_t>(rml::luau::access::tvalue_size);

				if (access::closure_in(stackFrame->func)->isC != 0)
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				if (index < 1 || index > stackSize)
				{
					luaL_argerror(L, 2, "stack index out of range");
				}

				auto* slot = access::value_at(stackFrame->base, static_cast<int>(index) - 1);

				if (slot->tt != lua_type(L, 3))
				{
					luaL_argerror(L, 2, "type on the stack is different than that you are trying to set!");
				}

				const auto* replacement = access::value(luaA_toobject(L, 3));

				barrier_into(L, L, replacement);

				access::copy_value(slot, replacement);
				return 0;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.setstack: {}", e.what()).c_str());
				lua_error(L);
				return 0;
			}
		}

		int getstack(lua_State* L)
		{
			try
			{
				luaL_checktype(L, 1, LUA_TNUMBER);

				const auto level = lua_tointeger(L, 1);
				const auto index = luaL_optinteger(L, 2, 69420);
				normalize_stack(L, 2);

				namespace access = rml::luau::access;

				auto* state = access::state(L);

				if (level >= access::frames_between(state->ci, state->base_ci) || level < 0)
				{
					luaL_argerror(L, 1, "level out of range");
				}

				auto* frame = access::frame_at(state->ci, -static_cast<int>(level));
				const auto stackFrameSize =
				    static_cast<int>((reinterpret_cast<std::byte*>(frame->top) -
				                      reinterpret_cast<std::byte*>(frame->base)) /
				                     static_cast<std::ptrdiff_t>(rml::luau::access::tvalue_size));

				if (access::closure_in(frame->func)->isC != 0)
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				if (index == 69420)
				{
					lua_newtable(L);

					for (int i = 0; i < stackFrameSize; i++)
					{
						access::copy_value(state->top, access::value_at(frame->base, i));
						access::advance_top(state);
						lua_rawseti(L, -2, i + 1);
					}
				}
				else
				{
					if (index < 1 || index > stackFrameSize)
					{
						luaL_argerror(L, 2, "index out of range");
					}

					access::copy_value(state->top, access::value_at(frame->base, static_cast<int>(index) - 1));
					access::advance_top(state);
				}

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getstack: {}", e.what()).c_str());
				lua_error(L);
			}
		}

		int debug_setupvalue(lua_State* L)
		{
			try
			{
				const int index = luaL_checkinteger(L, 2);
				luaL_checkany(L, 3);
				normalize_stack(L, 3);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "function or level expected");
				}

				if (lua_isnumber(L, 1))
				{
					lua_Debug ar;
					if (!lua_getinfo(L, lua_tointeger(L, 1), "f", &ar))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				if (lua_iscfunction(L, -1))
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				namespace access = rml::luau::access;

				auto* raw           = clvalue(luaA_toobject(L, -1));
				auto* cl            = access::closure(raw);
				const auto* value   = access::value(luaA_toobject(L, 3));
				auto* upvalue_table = access::upvalues_of(cl);

				if (index < 1)
				{
					luaL_argerror(L, 2, "upvalue index starts at 1");
				}

				if (index > cl->nupvalues)
				{
					luaL_argerror(L, 2, "upvalue index out of range");
				}

				auto* upvalue = access::value_at(upvalue_table, index - 1);

				barrier_into(L, raw, value);

				access::copy_value(upvalue, value);

				lua_pushboolean(L, true);
				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.setupvalue: {}", e.what()).c_str());
				lua_error(L);
			}
		}

		int getupvalue(lua_State* L)
		{
			try
			{
				luaL_checktype(L, 2, LUA_TNUMBER);
				normalize_stack(L, 2);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "function or level expected");
				}

				if (lua_isnumber(L, 1))
				{
					lua_Debug ar;
					if (!lua_getinfo(L, lua_tointeger(L, 1), "f", &ar))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				const int index = luaL_checkinteger(L, 2);
				namespace access = rml::luau::access;

				const auto* cl              = access::closure(clvalue(luaA_toobject(L, -1)));
				const auto* upvalue_table   = access::upvalues_of(cl);

				if (!index)
				{
					luaL_argerror(L, 2, "upvalue index starts at 1");
				}

				if (index > cl->nupvalues)
				{
					luaL_argerror(L, 2, "upvalue index is out of range");
				}

				const auto* upval = access::value_at(upvalue_table, index - 1);

				barrier_into(L, L, upval);

				if (upval->tt == LUA_TTABLE)
				{
					lua_pushnil(L);
					return 1;
				}

				auto* state = access::state(L);
				access::copy_value(state->top, upval);
				access::advance_top(state);

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getupvalue: {}", e.what()).c_str());
				lua_error(L);
			}
		}

		int getupvalues(lua_State* L)
		{
			try
			{
				normalize_stack(L, 1);

				if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1))
				{
					luaL_typeerror(L, 1, "function or level expected");
				}

				if (lua_isnumber(L, 1))
				{
					lua_Debug ar;
					if (!lua_getinfo(L, lua_tointeger(L, 1), "f", &ar))
					{
						luaL_argerror(L, 1, "level out of range");
					}
				}
				else
				{
					lua_pushvalue(L, 1);
				}

				namespace access = rml::luau::access;

				const auto* cl           = access::closure(clvalue(luaA_toobject(L, -1)));
				const auto* upvalueTable = access::upvalues_of(cl);
				auto* state              = access::state(L);

				lua_newtable(L);

				for (int i = 0; i < cl->nupvalues; i++)
				{
					const auto* upval = access::value_at(upvalueTable, i);

					barrier_into(L, L, upval);

					if (upval->tt == LUA_TFUNCTION || upval->tt == LUA_TTABLE)
					{
						lua_pushnil(L);
					}
					else
					{
						access::copy_value(state->top, upval);
						access::advance_top(state);
					}

					lua_rawseti(L, -2, (i + 1));
				}

				return 1;
			}
			catch (const std::exception& e)
			{
				lua_pushstring(L, std::format("Error in debug.getupvalues: {}", e.what()).c_str());
				lua_error(L);
			}
		}
	}

	bool DebugProvider::register_globals(lua_State* L) noexcept
	{
		try
		{
			register_debug_table(L);
			return true;
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Failed to register debug provider: {}", e.what());
			return false;
		}
	}

	void DebugProvider::register_debug_table(lua_State* L) noexcept
	{
		try
		{
			lua_getglobal(L, "debug");
			lua_setreadonly(L, -1, false);

			lua_pushcfunction(L, debug_impl::getconstants, "getconstants");
			lua_setfield(L, -2, "getconstants");
			lua_pushcfunction(L, debug_impl::getconstant, "getconstant");
			lua_setfield(L, -2, "getconstant");
			lua_pushcfunction(L, debug_impl::setconstant, "setconstant");
			lua_setfield(L, -2, "setconstant");
			lua_pushcfunction(L, debug_impl::getinfo, "getinfo");
			lua_setfield(L, -2, "getinfo");
			lua_pushcfunction(L, debug_impl::getproto, "getproto");
			lua_setfield(L, -2, "getproto");
			lua_pushcfunction(L, debug_impl::getprotos, "getprotos");
			lua_setfield(L, -2, "getprotos");
			lua_pushcfunction(L, debug_impl::setstack, "setstack");
			lua_setfield(L, -2, "setstack");
			lua_pushcfunction(L, debug_impl::getstack, "getstack");
			lua_setfield(L, -2, "getstack");
			lua_pushcfunction(L, debug_impl::debug_setupvalue, "setupvalue");
			lua_setfield(L, -2, "setupvalue");
			lua_pushcfunction(L, debug_impl::getupvalue, "getupvalue");
			lua_setfield(L, -2, "getupvalue");
			lua_pushcfunction(L, debug_impl::getupvalues, "getupvalues");
			lua_setfield(L, -2, "getupvalues");
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Failed to register debug table: {}", e.what());
		}
	}
}
