#pragma once
#include "RobloxModLoader/roblox/i_task_scheduler.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/job_base.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/rml_export.hpp"

#include <chrono>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace rml::jobs {
    class JobManager final {
    public:
        explicit JobManager(ITaskScheduler &task_scheduler);

        ~JobManager();

        JobManager(const JobManager &) = delete;

        JobManager &operator=(const JobManager &) = delete;

        JobManager(JobManager &&) noexcept = delete;

        JobManager &operator=(JobManager &&) noexcept = delete;

        /**
         * @brief Register a job with the task scheduler
         * @tparam JobType The job type to register (must implement JobImplementation concept)
         * @tparam Args Constructor arguments for the job
         * @param args Arguments to forward to the job constructor
         * @return Expected containing job ID on success, error message on failure
         */
        template<JobImplementation JobType, typename... Args>
        std::expected<RBX::TaskScheduler::JobId, std::string> register_job(Args &&... args) noexcept {
            try {
                auto job = std::make_unique<JobType>(std::forward<Args>(args)...);
                return m_task_scheduler.register_job(std::move(job));
            } catch (const std::exception &e) {
                return std::unexpected(std::format("Failed to create job: {}", e.what()));
            }
        }

        template<JobImplementation JobType, typename... Args>
        void register_job_and_ignore(Args &&... args) noexcept {
            auto result = register_job<JobType>(std::forward<Args>(args)...);
            if (!result) {
                LOG_ERROR("Failed to register job: {}", result.error());
            }
        }

        /**
         * @brief Register a lambda-based job
         * @param name Job name
         * @param priority Job priority
         * @param target_kind Target job kind
         * @param should_execute_func Function to determine if job should execute
         * @param execute_func Function to execute when job runs
         * @return Expected containing job ID on success, error message on failure
         */
        static std::expected<RBX::TaskScheduler::JobId, std::string> register_lambda_job(
            std::string_view name,
            JobPriority priority,
            JobKind target_kind,
            std::function<bool(const JobExecutionContext &)> should_execute_func,
            std::function<void(const JobExecutionContext &)> execute_func
        ) noexcept;

        /**
         * @brief Register a periodic job that executes at fixed intervals
         * @param name Job name
         * @param interval Execution interval
         * @param priority Job priority
         * @param target_kind Target job kind
         * @param execute_func Function to execute
         * @return Expected containing job ID on success, error message on failure
         */
        static std::expected<RBX::TaskScheduler::JobId, std::string> register_periodic_job(
            std::string_view name,
            std::chrono::milliseconds interval,
            JobPriority priority,
            JobKind target_kind,
            std::function<void(const JobExecutionContext &)> execute_func
        ) noexcept;

        /**
         * @brief Unregister a job by ID
         */
        static bool unregister_job(RBX::TaskScheduler::JobId job_id) noexcept;

        /**
         * @brief Unregister a job by name
         */
        static bool unregister_job(std::string_view job_name) noexcept;

        /**
         * @brief Get job statistics
         */
        static std::optional<RBX::TaskScheduler::JobStats> get_job_stats(RBX::TaskScheduler::JobId job_id) noexcept;

        /**
         * @brief Get all jobs of a specific kind
         */
        static std::vector<RBX::TaskScheduler::JobId> get_jobs_by_kind(JobKind kind) noexcept;

        /**
         * @brief Get total number of registered jobs
         */
        static std::size_t get_job_count() noexcept;

    private:
        class LambdaJob;
        class PeriodicJob;

        ITaskScheduler &m_task_scheduler;
    };

    [[nodiscard]] RML_EXPORT JobManager &job_manager();

    [[nodiscard]] RML_EXPORT bool has_job_manager() noexcept;
}
