#include "RobloxModLoader/luau/environment/debug_provider.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/luau/extensions/luau_extensions.hpp"
#include "lapi.h"
#include "ldebug.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"

namespace rml::luau::environment
{
	namespace debug_impl
	{
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

				const auto* closure   = luau_to_closure(L, -1);
				const auto constCount = closure->l.p->sizek;
				const auto consts     = closure->l.p->k;

				lua_newtable(L);

				for (int i = 0; i < constCount; i++)
				{
					const TValue* tval = &consts[i];

					if (tval->tt == LUA_TFUNCTION)
					{
						lua_pushnil(L);
					}
					else
					{
						if (iscollectable(tval))
						{
							luaC_threadbarrier(L);
						}
						L->top->value = tval->value;
						L->top->tt    = tval->tt;
						L->top++;
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

				const auto* closure = luau_to_closure(L, -1);
				const auto constants = closure->l.p->k;

				if (constantIndex < 1)
				{
					luaL_argerror(L, 2, "constant index starts at 1");
				}

				if (constantIndex > closure->l.p->sizek)
				{
					luaL_argerror(L, 2, "constant index is out of range");
				}

				const auto* tValue = &constants[constantIndex - 1];

				if (tValue->tt == LUA_TFUNCTION)
				{
					lua_pushnil(L);
				}
				else
				{
					if (iscollectable(tValue))
					{
						luaC_threadbarrier(L);
					}
					L->top->tt    = tValue->tt;
					L->top->value = tValue->value;
					L->top++;
					checkliveness(L->global, tValue);
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

				const auto cl = luau_to_closure(L, -1);
				const auto* p = cl->l.p;
				auto* k       = p->k;

				if (index < 1)
				{
					luaL_argerror(L, 2, "constant index starts at 1");
				}

				if (index > p->sizek)
				{
					luaL_argerror(L, 2, "constant index out of range");
				}

				auto* constant = &k[index - 1];

				if (constant->tt == LUA_TFUNCTION)
				{
					return 0;
				}

				const TValue* newConstant = luaA_toobject(L, 3);

				if (newConstant->tt != constant->tt)
				{
					luaL_argerror(L, 3, "cannot replace constant when the element you want to replace it with is not of the same type.");
				}

				if (iscollectable(newConstant))
				{
					luaC_threadbarrier(L);
				}

				constant->tt    = newConstant->tt;
				constant->value = newConstant->value;

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

				lua_Debug lDebug{};

				if (!lua_getinfo(L, infoLevel, "fulasn", &lDebug))
				{
					luaL_argerror(L, 1, "invalid level");
				}

				lua_newtable(L);

				lua_pushstring(L, lDebug.source);
				lua_setfield(L, -2, "source");

				lua_pushstring(L, lDebug.short_src);
				lua_setfield(L, -2, "short_src");

				lua_pushvalue(L, 1);
				lua_setfield(L, -2, "func");

				lua_pushstring(L, lDebug.what);
				lua_setfield(L, -2, "what");

				lua_pushinteger(L, lDebug.currentline);
				lua_setfield(L, -2, "currentline");

				lua_pushstring(L, lDebug.name);
				lua_setfield(L, -2, "name");

				lua_pushinteger(L, lDebug.nupvals);
				lua_setfield(L, -2, "nups");

				lua_pushinteger(L, lDebug.nparams);
				lua_setfield(L, -2, "numparams");

				lua_pushinteger(L, lDebug.isvararg);
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

				const auto closure = clvalue(luaA_toobject(L, -1));
				const auto index   = static_cast<int>(lua_tointeger(L, 2));

				if (index < 1 || index > closure->l.p->sizep)
				{
					luaL_argerror(L, 2, "proto index out of range");
				}

				const auto proto = closure->l.p->p[index - 1];

				lua_newtable(L);
				setclvalue(L, L->top, luaF_newLclosure(L, proto->nups, closure->env, proto));
				L->top++;
				lua_rawseti(L, -2, 1);

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

				const auto* cl = luau_to_closure(L, -1);
				lua_newtable(L);

				const auto* mProto = cl->l.p;

				for (int i = 0; i < mProto->sizep; i++)
				{
					Proto* proto      = mProto->p[i];
					Closure* lclosure = luaF_newLclosure(L, proto->nups, cl->env, proto);

					setclvalue(L, L->top, lclosure);
					L->top++;
					lua_rawseti(L, -2, i + 1);
				}

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

				if (level >= L->ci - L->base_ci || level < 0)
				{
					luaL_argerror(L, 1, "level out of range");
				}

				const auto stackFrame = L->ci - level;
				const auto stackSize  = stackFrame->top - stackFrame->base;

				if (clvalue(stackFrame->func)->isC)
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				if (index < 1 || index > stackSize)
				{
					luaL_argerror(L, 2, "stack index out of range");
				}

				if (stackFrame->base[index - 1].tt != lua_type(L, 3))
				{
					luaL_argerror(L, 2, "type on the stack is different than that you are trying to set!");
				}

				if (iscollectable(luaA_toobject(L, 3)))
				{
					luaC_threadbarrier(L);
				}

				setobj2s(L, &stackFrame->base[index - 1], luaA_toobject(L, 3));
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

				if (level >= L->ci - L->base_ci || level < 0)
				{
					luaL_argerror(L, 1, "level out of range");
				}

				const auto frame          = L->ci - level;
				const auto stackFrameSize = static_cast<int>(frame->top - frame->base);

				if (clvalue(frame->func)->isC)
				{
					luaL_argerror(L, 1, "Lua function expected.");
				}

				if (index == 69420)
				{
					lua_newtable(L);

					for (int i = 0; i < stackFrameSize; i++)
					{
						setobj2s(L, L->top, &frame->base[i]);
						L->top++;
						lua_rawseti(L, -2, i + 1);
					}
				}
				else
				{
					if (index < 1 || index > stackFrameSize)
					{
						luaL_argerror(L, 2, "index out of range");
					}

					setobj2s(L, L->top, &frame->base[index - 1]);
					L->top++;
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

				auto* cl            = clvalue(luaA_toobject(L, -1));
				const TValue* value = luaA_toobject(L, 3);
				auto* upvalue_table = cl->l.uprefs;

				if (index < 1)
				{
					luaL_argerror(L, 2, "upvalue index starts at 1");
				}

				if (index > cl->nupvalues)
				{
					luaL_argerror(L, 2, "upvalue index out of range");
				}

				TValue* upvalue = &upvalue_table[index - 1];

				if (iscollectable(value))
				{
					luaC_threadbarrier(L);
				}

				upvalue->value = value->value;
				upvalue->tt    = value->tt;

				luaC_barrier(L, cl, value);

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
				const auto* cl = clvalue(luaA_toobject(L, -1));
				const TValue* upvalue_table = nullptr;

				if (!cl->isC)
				{
					upvalue_table = cl->l.uprefs;
				}
				else if (cl->isC)
				{
					upvalue_table = cl->c.upvals;
				}

				if (!index)
				{
					luaL_argerror(L, 2, "upvalue index starts at 1");
				}

				if (index > cl->nupvalues)
				{
					luaL_argerror(L, 2, "upvalue index is out of range");
				}

				const auto* upval = &upvalue_table[index - 1];
				auto* top = L->top;

				if (iscollectable(upval))
				{
					luaC_threadbarrier(L);
				}

				if (upval->tt == LUA_TTABLE)
				{
					lua_pushnil(L);
					return 1;
				}

				top->value = upval->value;
				top->tt    = upval->tt;
				L->top++;

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

				const auto* cl = clvalue(luaA_toobject(L, -1));
				const TValue* upvalueTable = nullptr;

				lua_newtable(L);

				if (!cl->isC)
				{
					upvalueTable = cl->l.uprefs;
				}
				else if (cl->isC)
				{
					upvalueTable = cl->c.upvals;
				}

				for (int i = 0; i < cl->nupvalues; i++)
				{
					const auto* upval = &upvalueTable[i];
					auto* top         = L->top;

					if (iscollectable(upval))
					{
						luaC_threadbarrier(L);
					}

					if (upval->tt == LUA_TFUNCTION || upval->tt == LUA_TTABLE)
					{
						lua_pushnil(L);
					}
					else
					{
						top->value = upval->value;
						top->tt    = upval->tt;
						L->top++;
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
