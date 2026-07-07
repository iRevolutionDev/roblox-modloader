#include "RobloxModLoader/script_subsystem.hpp"

#if RML_ENABLE_LUAU
#include "RobloxModLoader/roblox/job_manager.hpp"
#include "roblox/jobs/scripting/luau_waiting_script_job.hpp"
#endif

RML_LOG_SCOPE("ScriptSubsystem");

namespace rml {
	ScriptSubsystem::ScriptSubsystem() {
		g_script_subsystem = this;
	}

	ScriptSubsystem::~ScriptSubsystem() {
		shutdown();

		g_script_subsystem = nullptr;
	}

	void ScriptSubsystem::initialize() {
#if RML_ENABLE_LUAU
		LOG_INFO("Initializing Script Subsystem...");

		m_script_manager = std::make_unique<luau::ScriptManager>();

		if (jobs::has_job_manager()) {
			jobs::job_manager().register_job_and_ignore<jobs::LuauWaitingScriptJob>();
		}

		LOG_INFO("Script Subsystem initialized.");
#endif
	}

	void ScriptSubsystem::shutdown() {
#if RML_ENABLE_LUAU
		m_script_manager.reset();
#endif
	}
}
