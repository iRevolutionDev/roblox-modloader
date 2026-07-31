#include "RobloxModLoader/luau/env/binding.hpp"

#include "RobloxModLoader/luau/script_host.hpp"
#include "RobloxModLoader/luau/vm/stack_guard.hpp"

#include <cstring>

RML_LOG_SCOPE("LuauBinding");

namespace rml::luau
{
	static void push_env_upvalue(const ScriptEnv& env, lua_State* L)
	{
		auto* token = env.token().get();
		lua_pushlstring(L, reinterpret_cast<const char*>(&token), sizeof(token));
	}

	ScriptEnv::ScriptEnv(ScriptHost& host, vm::Thread thread, ModEnvironment mod)
		: m_host(&host), m_thread(std::move(thread)), m_mod(std::move(mod)), m_modules(host.bytecode()),
		  m_token(std::make_shared<EnvToken>())
	{
		m_token->env = this;
	}

	ScriptEnv::~ScriptEnv()
	{
		m_token->env = nullptr;
	}

	ClosureRegistry& ScriptEnv::closures() const noexcept { return m_host->closures(); }

	void ScriptEnv::add_unload_handler(vm::Ref handler)
	{
		if (handler.valid())
		{
			m_unload_handlers.push_back(std::move(handler));
		}
	}

	std::vector<vm::Ref> ScriptEnv::take_unload_handlers() noexcept
	{
		return std::exchange(m_unload_handlers, {});
	}

	void ScriptEnv::release() noexcept
	{
		for (auto& handler : m_unload_handlers)
		{
			handler.release();
		}

		m_unload_handlers.clear();
		m_modules.release();
		m_original_require.release();
		m_thread.release();
	}

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

	ScriptEnv* try_bound_env(lua_State* L) noexcept
	{
		std::size_t length = 0;
		const auto* bytes = lua_tolstring(L, lua_upvalueindex(1), &length);

		if (bytes == nullptr || length != sizeof(EnvToken*))
		{
			return nullptr;
		}

		EnvToken* token = nullptr;
		std::memcpy(&token, bytes, sizeof(token));

		return token == nullptr ? nullptr : token->env;
	}

	ScriptEnv& bound_env(lua_State* L)
	{
		if (auto* env = try_bound_env(L))
		{
			return *env;
		}

		luaL_error(L, "this function belongs to a script environment that was unloaded");
	}

	vm::Ref anchor_in_env(const ScriptEnv& env, lua_State* L, const int index)
	{
		auto* target = env.thread();

		if (target == nullptr || target == L)
		{
			return vm::Ref::take(L, index);
		}

		const auto staged = vm::Ref::take(L, index);
		if (!staged.valid())
		{
			return {};
		}

		vm::StackGuard guard(target);
		if (!staged.push(target))
		{
			return {};
		}

		return vm::Ref::take(target, -1);
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
