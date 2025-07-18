#pragma once
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/job_base.hpp"
#include "RobloxModLoader/common.hpp"

namespace rml::jobs {
    class JobManager final {
    public:
        JobManager();

        ~JobManager();

        JobManager(const JobManager &) = delete;

        JobManager &operator=(const JobManager &) = delete;

        JobManager(JobManager &&) noexcept = default;

        JobManager &operator=(JobManager &&) noexcept = default;

        /**
         * @brief Register a job with the task scheduler
         * @tparam JobType The job type to register (must implement JobImplementation concept)
         * @tparam Args Constructor arguments for the job
         * @param args Arguments to forward to the job constructor
         * @return Expected containing job ID on success, error message on failure
         */
        template<JobImplementation JobType, typename... Args>
        std::expected<RBX::TaskScheduler::JobId, std::string> register_job(Args &&... args) noexcept {
            if (!g_task_scheduler) {
                return std::unexpected("TaskScheduler not initialized");
            }

            try {
                auto job = std::make_unique<JobType>(std::forward<Args>(args)...);
                return g_task_scheduler->register_job(std::move(job));
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
    };

    inline JobManager *g_job_manager{};
}
