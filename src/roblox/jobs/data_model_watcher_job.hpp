#pragma once
#include "RobloxModLoader/roblox/job_base.hpp"
#include "RobloxModLoader/common.hpp"

namespace rml::jobs {
    class DataModelWatcherJob final : public JobBase {
    public:
        DataModelWatcherJob() noexcept;

        ~DataModelWatcherJob() override = default;

        DataModelWatcherJob(const DataModelWatcherJob &) = delete;

        DataModelWatcherJob &operator=(const DataModelWatcherJob &) = delete;

        DataModelWatcherJob(DataModelWatcherJob &&) noexcept = delete;

        DataModelWatcherJob &operator=(DataModelWatcherJob &&) noexcept = delete;

    private:
        bool should_execute_impl(const JobExecutionContext &context) noexcept override;

        void execute_impl(const JobExecutionContext &context) override;

        void destroy_impl() noexcept override;

        static constexpr std::string_view JOB_NAME = "DataModelWatcher";

        std::uintptr_t get_current_data_model() const noexcept;

        void on_data_model_changed(std::uintptr_t old_dm, std::uintptr_t new_dm) noexcept;
    };
}
