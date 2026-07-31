#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/modules/bytecode_cache.hpp"
#include "RobloxModLoader/luau/modules/module_id.hpp"
#include "RobloxModLoader/luau/vm/lua_ref.hpp"
#include <unordered_map>

namespace rml::luau
{
	class ModuleRegistry final
	{
	public:
		ModuleRegistry() = default;

		ModuleRegistry(const ModuleRegistry&) = delete;
		ModuleRegistry& operator=(const ModuleRegistry&) = delete;
		ModuleRegistry(ModuleRegistry&&) = delete;
		ModuleRegistry& operator=(ModuleRegistry&&) = delete;

		[[nodiscard]] std::expected<void, vm::VmError> require(
		    lua_State* L, const ModuleId& id, const ModEnvironment& env);

		void invalidate(const ModuleId& id);
		std::size_t invalidate_under(const std::filesystem::path& root);
		void clear() noexcept;
		void release() noexcept;

		[[nodiscard]] std::size_t loaded_count() const noexcept { return m_loaded.size(); }

	private:
		[[nodiscard]] std::expected<void, vm::VmError> load(
		    lua_State* L, const ModuleId& id, const ModEnvironment& env);

		[[nodiscard]] std::string describe_cycle(const ModuleId& repeated) const;

		std::unordered_map<ModuleId, vm::Ref, ModuleIdHash> m_loaded;
		std::vector<ModuleId> m_loading;
		BytecodeCache m_bytecode;
	};
}
