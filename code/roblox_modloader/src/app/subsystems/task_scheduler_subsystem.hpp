#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

namespace rml
{
	class TaskSchedulerSubsystem final : public ISubsystem
	{
	public:
		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<RBX::TaskScheduler>();
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "TaskScheduler";
		}

		[[nodiscard]] ITaskScheduler& task_scheduler() const
		{
			return *m_instance;
		}

	private:
		std::unique_ptr<RBX::TaskScheduler> m_instance;
	};
}
