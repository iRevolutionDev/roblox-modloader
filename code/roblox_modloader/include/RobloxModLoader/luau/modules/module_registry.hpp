#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/modules/bytecode_cache.hpp"
#include "RobloxModLoader/luau/modules/module_resolver.hpp"
#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include <unordered_map>

namespace rml::luau
{
	class ScriptEnv;

	struct LoadedModule
	{
		vm::Ref value;
		bool returned_nil{false};
	};

	class ModuleRegistry final
	{
	public:
		ModuleRegistry(BytecodeCache& bytecode, lua_State* anchor) noexcept
			: m_bytecode(&bytecode), m_anchor(anchor)
		{
		}

		ModuleRegistry(const ModuleRegistry&) = delete;
		ModuleRegistry& operator=(const ModuleRegistry&) = delete;
		ModuleRegistry(ModuleRegistry&&) = delete;
		ModuleRegistry& operator=(ModuleRegistry&&) = delete;

		[[nodiscard]] std::expected<void, vm::VmError> require(
		    lua_State* L, const ResolvedModule& module, ScriptEnv& env);

		void invalidate(const ModuleId& id);
		std::size_t invalidate_under(const std::filesystem::path& root);
		void clear() noexcept;
		void release() noexcept;

		[[nodiscard]] std::size_t loaded_count() const noexcept { return m_loaded.size(); }

	private:
		[[nodiscard]] std::expected<void, vm::VmError> load(
		    lua_State* L, const ResolvedModule& module, ScriptEnv& env);

		[[nodiscard]] std::string describe_cycle(const ResolvedModule& repeated) const;

		BytecodeCache* m_bytecode;
		lua_State* m_anchor;
		std::unordered_map<ModuleId, LoadedModule, ModuleIdHash> m_loaded;
		std::vector<ResolvedModule> m_loading;
	};
}
