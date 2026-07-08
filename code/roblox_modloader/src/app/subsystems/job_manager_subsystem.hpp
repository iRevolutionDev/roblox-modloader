#pragma once

#include "../isubsystem.hpp"
#include "RobloxModLoader/roblox/job_manager.hpp"
#include "task_scheduler_subsystem.hpp"

namespace rml
{
	class JobManagerSubsystem final : public ISubsystem
	{
	public:
		explicit JobManagerSubsystem(TaskSchedulerSubsystem& task_scheduler_subsystem) :
		    m_task_scheduler_subsystem(task_scheduler_subsystem)
		{
		}

		std::expected<void, SubsystemError> initialize() override
		{
			m_instance = std::make_unique<jobs::JobManager>(m_task_scheduler_subsystem.task_scheduler());
			return {};
		}

		void shutdown() override
		{
			m_instance.reset();
		}

		[[nodiscard]] std::string_view name() const noexcept override
		{
			return "JobManager";
		}

	private:
		TaskSchedulerSubsystem& m_task_scheduler_subsystem;
		std::unique_ptr<jobs::JobManager> m_instance;
	};
}
