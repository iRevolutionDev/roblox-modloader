#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/modules/bytecode_cache.hpp"
#include "RobloxModLoader/luau/modules/module_resolver.hpp"
#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include <unordered_map>

namespace rml::luau
{
	class ModuleRegistry final
	{
	public:
		explicit ModuleRegistry(BytecodeCache& bytecode) noexcept
			: m_bytecode(&bytecode)
		{
		}

		ModuleRegistry(const ModuleRegistry&) = delete;
		ModuleRegistry& operator=(const ModuleRegistry&) = delete;
		ModuleRegistry(ModuleRegistry&&) = delete;
		ModuleRegistry& operator=(ModuleRegistry&&) = delete;

		[[nodiscard]] std::expected<void, vm::VmError> require(
		    lua_State* L, const ResolvedModule& module, const ModEnvironment& env);

		void invalidate(const ModuleId& id);
		std::size_t invalidate_under(const std::filesystem::path& root);
		void clear() noexcept;
		void release() noexcept;

		[[nodiscard]] std::size_t loaded_count() const noexcept { return m_loaded.size(); }

	private:
		[[nodiscard]] std::expected<void, vm::VmError> load(
		    lua_State* L, const ResolvedModule& module, const ModEnvironment& env);

		[[nodiscard]] std::string describe_cycle(const ResolvedModule& repeated) const;

		BytecodeCache* m_bytecode;
		std::unordered_map<ModuleId, vm::Ref, ModuleIdHash> m_loaded;
		std::vector<ResolvedModule> m_loading;
	};
}
