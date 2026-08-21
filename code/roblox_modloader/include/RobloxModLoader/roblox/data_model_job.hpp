#pragma once
#include "task_scheduler.job.hpp"

namespace RBX
{
	class DataModelJob : public TaskSchedulerJob
	{
	public:
		virtual StepResult step_data_model_job(const Stats& stats) = 0;
	};
}
