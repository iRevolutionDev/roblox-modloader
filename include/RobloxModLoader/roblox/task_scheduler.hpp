#pragma once
#include "RobloxModLoader/roblox/job.hpp"
#include "RobloxModLoader/common.hpp"
#include <unordered_map>
#include <vector>
#include <shared_mutex>

namespace RBX {
    class TaskScheduler final {
    public:
        using JobPtr = std::unique_ptr<IJob>;
        using JobId = std::uint64_t;

        TaskScheduler();

        ~TaskScheduler();

        TaskScheduler(const TaskScheduler &) = delete;

        TaskScheduler &operator=(const TaskScheduler &) = delete;

        TaskScheduler(TaskScheduler &&) noexcept = delete;

        TaskScheduler &operator=(TaskScheduler &&) noexcept = delete;

        std::expected<JobId, std::string> register_job(JobPtr job) noexcept;

        bool unregister_job(JobId job_id) noexcept;

        bool unregister_job(std::string_view job_name) noexcept;

        void execute_jobs_for_kind(const JobExecutionContext &context) noexcept;

        std::optional<std::reference_wrapper<IJob> > get_job(JobId job_id) const noexcept;

        std::optional<std::reference_wrapper<IJob> > get_job(std::string_view job_name) const noexcept;

        std::vector<JobId> get_jobs_by_kind(JobKind kind) const noexcept;

        std::size_t get_job_count() const noexcept;

        struct JobStats {
            std::uint64_t executions = 0;
            std::uint64_t failures = 0;
            std::chrono::nanoseconds total_execution_time{0};
            std::chrono::nanoseconds average_execution_time{0};
        };

        std::optional<JobStats> get_job_stats(JobId job_id) const noexcept;

        void reset_stats() noexcept;

        std::uintptr_t get_roblox_scheduler() const noexcept;

        std::uintptr_t get_roblox_job_by_name(std::string_view name) const noexcept;

        void shutdown() noexcept;

        bool is_shutdown() const noexcept;

        std::optional<JobKind> get_job_kind_from_vtable(void **vtable) const noexcept;

        std::optional<void **> get_vtable_for_job_kind(JobKind kind) const noexcept;

    private:
        struct JobEntry {
            JobPtr job;
            JobStats stats;
            std::chrono::steady_clock::time_point last_execution;

            explicit JobEntry(JobPtr job_ptr) noexcept
                : job(std::move(job_ptr))
                  , last_execution(std::chrono::steady_clock::now()) {
            }
        };

        mutable std::shared_mutex m_jobs_mutex;
        std::unordered_map<JobId, JobEntry> m_jobs;
        std::unordered_map<std::string, JobId> m_name_to_id;
        std::atomic<JobId> m_next_job_id{1};

        std::uintptr_t m_roblox_scheduler;

        std::atomic<bool> m_shutdown_requested{false};

        std::unordered_map<void **, JobKind> m_vtable_to_kind;
        std::unordered_map<JobKind, void **> m_kind_to_vtable;

        void initialize() noexcept;

        void initialize_vtable_mappings() noexcept;

        void cleanup_finished_jobs() noexcept;

        JobId generate_job_id() noexcept;

        void execute_job_with_stats(JobEntry &entry, const JobExecutionContext &context) noexcept;
    };
}

inline RBX::TaskScheduler *g_task_scheduler{};
