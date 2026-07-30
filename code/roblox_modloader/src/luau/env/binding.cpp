#include "RobloxModLoader/luau/env/binding.hpp"

#include "RobloxModLoader/luau/script_host.hpp"

#include <cstring>

RML_LOG_SCOPE("LuauBinding");

namespace rml::luau
{
	static void push_env_upvalue(ScriptEnv& env, lua_State* L)
	{
		auto* address = &env;
		lua_pushlstring(L, reinterpret_cast<const char*>(&address), sizeof(address));
	}

	ScriptEnv::ScriptEnv(ScriptHost& host, vm::Thread thread, ModEnvironment mod) noexcept
		: m_host(&host), m_thread(std::move(thread)), m_mod(std::move(mod))
	{
	}

	ModuleRegistry& ScriptEnv::modules() const noexcept { return m_host->modules(); }

	ClosureRegistry& ScriptEnv::closures() const noexcept { return m_host->closures(); }

	void push_bound_function(ScriptEnv& env, lua_State* L, const char* debug_name, const lua_CFunction fn)
	{
		push_env_upvalue(env, L);
		lua_pushcclosure(L, fn, debug_name, 1);
	}

	void set_bound_global(ScriptEnv& env, lua_State* L, const char* name, const lua_CFunction fn)
	{
		push_bound_function(env, L, name, fn);
		lua_setglobal(L, name);
	}

	ScriptEnv& bound_env(lua_State* L) noexcept
	{
		std::size_t length = 0;
		const auto* bytes = lua_tolstring(L, lua_upvalueindex(1), &length);

		ScriptEnv* env = nullptr;
		std::memcpy(&env, bytes, sizeof(env));

		return *env;
	}

	bool bind_globals(ScriptEnv& env, lua_State* L) noexcept
	{
		bool all_bound = true;

		for (const auto& [name, bind] : kBinders)
		{
			const auto top = lua_gettop(L);

			if (!bind(env, L))
			{
				RML_ERROR("Failed to bind the '{}' global for mod '{}'", name, env.mod().mod_name());
				all_bound = false;
			}

			lua_settop(L, top);
		}

		return all_bound;
	}
}
