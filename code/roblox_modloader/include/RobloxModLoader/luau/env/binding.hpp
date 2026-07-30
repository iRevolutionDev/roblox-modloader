#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/vm/lua_thread.hpp"

namespace rml::luau
{
	class ScriptHost;
	class ModuleRegistry;
	class ClosureRegistry;

	class ScriptEnv final
	{
	public:
		ScriptEnv(ScriptHost& host, vm::Thread thread, ModEnvironment mod) noexcept;

		ScriptEnv(const ScriptEnv&) = delete;
		ScriptEnv& operator=(const ScriptEnv&) = delete;
		ScriptEnv(ScriptEnv&&) = delete;
		ScriptEnv& operator=(ScriptEnv&&) = delete;

		[[nodiscard]] lua_State* thread() const noexcept { return m_thread.get(); }
		[[nodiscard]] ScriptHost& host() const noexcept { return *m_host; }
		[[nodiscard]] const ModEnvironment& mod() const noexcept { return m_mod; }

		[[nodiscard]] ModuleRegistry& modules() const noexcept;
		[[nodiscard]] ClosureRegistry& closures() const noexcept;

		void adopt_original_require(vm::Ref require) noexcept { m_original_require = std::move(require); }
		[[nodiscard]] const vm::Ref& original_require() const noexcept { return m_original_require; }

	private:
		ScriptHost* m_host;
		vm::Thread m_thread;
		ModEnvironment m_mod;
		vm::Ref m_original_require;
	};

	struct GlobalBinder
	{
		std::string_view name;
		bool (*bind)(ScriptEnv&, lua_State*) noexcept;
	};

	bool bind_require(ScriptEnv&, lua_State*) noexcept;
	bool bind_rml(ScriptEnv&, lua_State*) noexcept;
	bool bind_bridge(ScriptEnv&, lua_State*) noexcept;
	bool bind_debug(ScriptEnv&, lua_State*) noexcept;
	bool bind_closures(ScriptEnv&, lua_State*) noexcept;

	inline constexpr std::array kBinders{
	    GlobalBinder{"require", &bind_require},
	    GlobalBinder{"rml", &bind_rml},
	    GlobalBinder{"bridge", &bind_bridge},
	    GlobalBinder{"debug", &bind_debug},
	    GlobalBinder{"closures", &bind_closures},
	};

	bool bind_globals(ScriptEnv& env, lua_State* L) noexcept;

	void set_bound_global(ScriptEnv& env, lua_State* L, const char* name, lua_CFunction fn);

	void push_bound_function(ScriptEnv& env, lua_State* L, const char* debug_name, lua_CFunction fn);

	[[nodiscard]] ScriptEnv& bound_env(lua_State* L) noexcept;
}
