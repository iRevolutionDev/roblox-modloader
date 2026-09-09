#include "script_subsystem.hpp"

#if RML_ENABLE_LUAU
	#include "RobloxModLoader/roblox/job_manager.hpp"
	#include "config/config_manager.hpp"
	#include "filesystem/directory.hpp"
	#include "roblox/jobs/scripting/luau_waiting_script_job.hpp"
#endif

RML_LOG_SCOPE("ScriptSubsystem");

namespace rml
{
	ScriptSubsystem::ScriptSubsystem() = default;

	ScriptSubsystem::~ScriptSubsystem()
	{
		shutdown();
	}

	std::expected<void, std::string> ScriptSubsystem::initialize()
	{
#if RML_ENABLE_LUAU
		RML_INFO("Initializing the script runtime...");

		auto runtime = std::make_unique<luau::ScriptRuntime>();

		if (auto started = runtime->initialize(filesystem::directory::get_mod_loader_directory() / "mods"); !started)
		{
			return std::unexpected(std::move(started.error()));
		}

		luau::set_script_runtime(runtime.get());
		m_runtime = std::move(runtime);

		if (config::get_config_manager().get_core_config().developer.enable_hot_reload)
		{
			m_runtime->set_hot_reload(true);
		}

		if (jobs::has_job_manager())
		{
			jobs::job_manager().register_job_and_ignore<jobs::LuauWaitingScriptJob>();
			RML_INFO("Registered LuauWaitingScriptJob");
		}
		else
		{
			RML_ERROR("No job manager at script runtime init, nothing will pump the VM");
		}

		RML_INFO("Script runtime ready.");
#endif
		return {};
	}

	void ScriptSubsystem::shutdown()
	{
#if RML_ENABLE_LUAU
		if (!m_runtime)
		{
			return;
		}

		luau::set_script_runtime(nullptr);
		m_runtime->shutdown();
		m_runtime.reset();
#endif
	}
}
