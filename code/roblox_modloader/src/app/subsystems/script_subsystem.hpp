#pragma once

#include <memory>

#if RML_ENABLE_LUAU
	#include "RobloxModLoader/luau/script_manager.hpp"
#endif

namespace rml
{
	class ScriptSubsystem final
	{
	public:
		ScriptSubsystem();
		~ScriptSubsystem();
		ScriptSubsystem(const ScriptSubsystem&) = delete;
		ScriptSubsystem& operator=(const ScriptSubsystem&) = delete;
		ScriptSubsystem(ScriptSubsystem&&) = delete;
		ScriptSubsystem& operator=(ScriptSubsystem&&) = delete;

		void initialize();
		void shutdown();

	private:
#if RML_ENABLE_LUAU
		std::unique_ptr<luau::ScriptManager> m_script_manager;
#endif
	};

	inline ScriptSubsystem* g_script_subsystem{};
}
