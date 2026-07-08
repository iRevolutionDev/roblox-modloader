#pragma once

#include "isubsystem.hpp"

namespace rml
{
	class Application
	{
	public:
		Application();
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		[[nodiscard]] std::expected<void, SubsystemError> initialize();
		void run();
		void shutdown();

	private:
		std::vector<std::unique_ptr<ISubsystem>> m_subsystems;
		bool m_shutdown_complete{true};
	};
}
