#include "RobloxModLoader/common.hpp"
#include "data_model_watcher_job.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

namespace rml::jobs {
    DataModelWatcherJob::DataModelWatcherJob() noexcept
        : JobBase(JOB_NAME, JobPriority::High, JobKind::Heartbeat, true) {
    }

    bool DataModelWatcherJob::should_execute_impl(const JobExecutionContext &context) noexcept {
        return true;
    }

    void DataModelWatcherJob::execute_impl(const JobExecutionContext &context) {
    }

    void DataModelWatcherJob::destroy_impl() noexcept {
    }

    std::uintptr_t DataModelWatcherJob::get_current_data_model() const noexcept {
        return 0;
    }

    void DataModelWatcherJob::on_data_model_changed(std::uintptr_t old_dm, std::uintptr_t new_dm) noexcept {
    }
}
