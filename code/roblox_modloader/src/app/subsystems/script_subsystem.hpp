#pragma once

#include <expected>
#include <memory>
#include <string>

#if RML_ENABLE_LUAU
	#include "RobloxModLoader/luau/script_runtime.hpp"
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

		std::expected<void, std::string> initialize();
		void shutdown();

	private:
#if RML_ENABLE_LUAU
		std::unique_ptr<luau::ScriptRuntime> m_runtime;
#endif
	};
}
