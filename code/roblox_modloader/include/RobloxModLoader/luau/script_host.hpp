#pragma once

#include "RobloxModLoader/luau/dispatch/dispatcher.hpp"
#include "RobloxModLoader/luau/env/binding.hpp"
#include "RobloxModLoader/luau/env/closure_registry.hpp"
#include "RobloxModLoader/luau/modules/module_registry.hpp"
#include "RobloxModLoader/luau/script/script_asset.hpp"
#include <unordered_map>

namespace RBX
{
	enum class DataModelType : std::int32_t;
}

namespace rml::luau
{
	class ScriptRuntime;

	class RML_EXPORT ScriptHost final
	{
	public:
		ScriptHost(ScriptRuntime& runtime, RBX::DataModelType type, lua_State* global_state);
		~ScriptHost();

		ScriptHost(const ScriptHost&) = delete;
		ScriptHost& operator=(const ScriptHost&) = delete;
		ScriptHost(ScriptHost&&) = delete;
		ScriptHost& operator=(ScriptHost&&) = delete;

		[[nodiscard]] RBX::DataModelType type() const noexcept { return m_type; }
		[[nodiscard]] lua_State* global_state() const noexcept { return m_global; }
		[[nodiscard]] ScriptRuntime& runtime() const noexcept { return *m_runtime; }

		[[nodiscard]] Dispatcher& dispatcher() noexcept { return m_dispatcher; }
		[[nodiscard]] ModuleRegistry& modules() noexcept { return m_modules; }
		[[nodiscard]] ClosureRegistry& closures() noexcept { return m_closures; }

		void pump(const Budget& budget) noexcept;

		[[nodiscard]] bool has_pending() const noexcept { return m_dispatcher.has_pending(); }

		std::expected<void, vm::VmError> post_script(const ModManifestPtr& mod, const ScriptAsset& asset);

		std::expected<ScriptEnv*, vm::VmError> env_for(const ModManifestPtr& mod);

		std::expected<ScriptEnv*, vm::VmError> loader_env() { return env_for(nullptr); }

		[[nodiscard]] RefId retain(vm::Ref ref);
		[[nodiscard]] vm::Ref* lookup(RefId id) noexcept;
		void release(RefId id) noexcept;

		void shutdown() noexcept;

	private:
		[[nodiscard]] bool on_owner_thread() const noexcept;

		void execute(Work& work, std::move_only_function<void(WorkResult)> settle) noexcept;

		ScriptRuntime* m_runtime;
		RBX::DataModelType m_type;
		lua_State* m_global;

		Dispatcher m_dispatcher;
		ModuleRegistry m_modules;
		ClosureRegistry m_closures;

		std::unordered_map<std::string, std::unique_ptr<ScriptEnv>> m_mod_envs;
		std::unordered_map<RefId, vm::Ref> m_refs;

		std::atomic<std::thread::id> m_owner{};
		bool m_shutdown{false};

		friend class Dispatcher;
	};
}
