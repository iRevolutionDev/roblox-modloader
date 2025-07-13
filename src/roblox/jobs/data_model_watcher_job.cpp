#include "RobloxModLoader/common.hpp"
#include "data_model_watcher_job.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

namespace rml::jobs {
    DataModelWatcherJob::DataModelWatcherJob() noexcept
        : JobBase(JOB_NAME, JobPriority::High, JobKind::Heartbeat, true) {
    }

    bool DataModelWatcherJob::should_execute_impl(const JobExecutionContext &context) noexcept {
        const auto data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
        if (!data_model) {
            return false;
        }

        const auto type = data_model->get_type();

        m_data_models[type] = data_model;
        m_data_model_last_time_stepped[type] = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - m_last_check
               ).count() > 16;
    }

    void DataModelWatcherJob::execute_impl(const JobExecutionContext &context) {
        m_last_check = std::chrono::high_resolution_clock::now();

        const auto data_model = g_task_scheduler->get_data_model();
        if (data_model != nullptr) {
            return;
        }

        const auto current_data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
        if (current_data_model == nullptr) {
            return;
        }

        on_data_model_changed(
            data_model,
            current_data_model
        );
    }

    void DataModelWatcherJob::destroy_impl() noexcept {
    }

    void DataModelWatcherJob::on_data_model_changed(const RBX::DataModel *old_data_model,
                                                    const RBX::DataModel *new_data_model) {
        if (g_task_scheduler == nullptr) {
            LOG_ERROR("[DataModelWatcherJob] TaskScheduler is null, cannot set new DataModel.");
            return;
        }

        if (old_data_model == new_data_model) {
            return;
        }

        LOG_INFO("[DataModelWatcherJob] DataModel changed from 0x{:X} to 0x{:X}",
                 old_data_model ? reinterpret_cast<uintptr_t>(old_data_model) : 0,
                 new_data_model ? reinterpret_cast<uintptr_t>(new_data_model) : 0);

        g_task_scheduler->set_data_model(new_data_model);
    }
}
