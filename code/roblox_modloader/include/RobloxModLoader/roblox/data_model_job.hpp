#pragma once
#include "task_scheduler.hpp"
#include "task_scheduler.job.hpp"

namespace RBX {
    class DataModelJob : public TaskSchedulerJob {
    public:
        virtual TaskScheduler::StepResult step(const Stats &stats) = 0;
    };
}
