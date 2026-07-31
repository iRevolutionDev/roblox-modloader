#pragma once

#include "filesystem/directory_watcher.hpp"

namespace rml::luau
{
	class ScriptRuntime;

	class ScriptWatcher final
	{
	public:
		explicit ScriptWatcher(ScriptRuntime& runtime) noexcept;
		~ScriptWatcher();

		ScriptWatcher(const ScriptWatcher&) = delete;
		ScriptWatcher& operator=(const ScriptWatcher&) = delete;
		ScriptWatcher(ScriptWatcher&&) = delete;
		ScriptWatcher& operator=(ScriptWatcher&&) = delete;

		void start();
		void stop();

		[[nodiscard]] bool running() const noexcept { return m_watcher.running(); }

	private:
		ScriptRuntime* m_runtime;
		filesystem::DirectoryWatcher m_watcher;
	};
}
