#pragma once

#include "RobloxModLoader/luau/script_host.hpp"
#include "RobloxModLoader/luau/script/script_catalog.hpp"
#include <unordered_map>

namespace rml::luau
{
	class Bridge;

	class RML_EXPORT ScriptRuntime final
	{
	public:
		ScriptRuntime();
		~ScriptRuntime();

		ScriptRuntime(const ScriptRuntime&) = delete;
		ScriptRuntime& operator=(const ScriptRuntime&) = delete;
		ScriptRuntime(ScriptRuntime&&) = delete;
		ScriptRuntime& operator=(ScriptRuntime&&) = delete;

		std::expected<void, std::string> initialize(const std::filesystem::path& mods_directory);
		void shutdown() noexcept;

		void bind_host(RBX::DataModelType type, lua_State* global_state);
		void unbind_host(RBX::DataModelType type);

		[[nodiscard]] ScriptHost* host(RBX::DataModelType type) noexcept;
		[[nodiscard]] ScriptHost* host_for(lua_State* thread) noexcept;
		[[nodiscard]] ScriptHost* host_for_ref(RefId id) noexcept;

		void activate(RBX::DataModelType type);

		std::expected<void, std::string> reload(std::string_view mod_name);

		[[nodiscard]] const ScriptCatalog& catalog() const noexcept { return m_catalog; }
		[[nodiscard]] Bridge& bridge() noexcept { return *m_bridge; }

		[[nodiscard]] RefId next_ref_id() noexcept;
		void map_ref(RefId id, ScriptHost* owner);
		void unmap_ref(RefId id) noexcept;
		void unmap_refs_of(const ScriptHost* owner) noexcept;

		[[nodiscard]] const ModEnvironment& loader_environment() const noexcept { return m_loader_env; }
		[[nodiscard]] ModEnvironment environment_for(const ModManifestPtr& mod) const;

	private:
		ScriptCatalog m_catalog;
		std::unique_ptr<Bridge> m_bridge;

		mutable std::shared_mutex m_hosts_mutex;
		std::unordered_map<RBX::DataModelType, std::unique_ptr<ScriptHost>> m_hosts;
		std::unordered_map<const void*, ScriptHost*> m_by_global_state;

		mutable std::shared_mutex m_refs_mutex;
		std::unordered_map<RefId, ScriptHost*> m_ref_owners;
		std::atomic<RefId> m_next_ref{1};

		ModEnvironment m_loader_env;
		std::filesystem::path m_mods_directory;
	};

	[[nodiscard]] RML_EXPORT ScriptRuntime* script_runtime() noexcept;
	RML_EXPORT void set_script_runtime(ScriptRuntime* runtime) noexcept;

	RML_EXPORT std::expected<void, vm::VmError> schedule(RBX::DataModelType context, std::string_view source,
	                                                     std::string_view chunk_name = "rml_native");

	[[nodiscard]] RML_EXPORT std::expected<std::future<WorkResult>, vm::VmError> evaluate(
	    RBX::DataModelType context, std::string_view source, std::string_view chunk_name = "rml_native");
}
