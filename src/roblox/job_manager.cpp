#include "RobloxModLoader/roblox/job_manager.hpp"

#include "jobs/data_model_watcher_job.hpp"
#include "jobs/scripting/luau_waiting_script_job.hpp"

namespace rml::jobs {
    class JobManager::LambdaJob final : public JobBase {
    public:
        LambdaJob(const std::string_view name, const JobPriority priority, const JobKind target_kind,
                  std::function<bool(const JobExecutionContext &)> should_execute_func,
                  std::function<void(const JobExecutionContext &)> execute_func) noexcept
            : JobBase(name, priority, target_kind, true)
              , m_should_execute_func(std::move(should_execute_func))
              , m_execute_func(std::move(execute_func)) {
        }

    protected:
        bool should_execute_impl(const JobExecutionContext &context) noexcept override {
            try {
                return m_should_execute_func && m_should_execute_func(context);
            } catch (...) {
                return false;
            }
        }

        void execute_impl(const JobExecutionContext &context) override {
            if (m_execute_func) {
                m_execute_func(context);
            }
        }

        void destroy_impl() noexcept override {
            m_should_execute_func = nullptr;
            m_execute_func = nullptr;
        }

    private:
        std::function<bool(const JobExecutionContext &)> m_should_execute_func;
        std::function<void(const JobExecutionContext &)> m_execute_func;
    };

    // This will be used later by the mods i guess
    class JobManager::PeriodicJob final : public JobBase {
    public:
        PeriodicJob(const std::string_view name, const std::chrono::milliseconds interval,
                    const JobPriority priority, const JobKind target_kind,
                    std::function<void(const JobExecutionContext &)> execute_func) noexcept
            : JobBase(name, priority, target_kind, true)
              , m_interval(interval)
              , m_execute_func(std::move(execute_func))
              , m_last_execution(std::chrono::steady_clock::now()) {
        }

    protected:
        bool should_execute_impl(const JobExecutionContext &context) noexcept override {
            const auto now = std::chrono::steady_clock::now();
            const auto last = m_last_execution.load(std::memory_order_acquire);
            return now - last >= m_interval;
        }

        void execute_impl(const JobExecutionContext &context) override {
            m_last_execution.store(std::chrono::steady_clock::now(), std::memory_order_release);

            if (m_execute_func) {
                m_execute_func(context);
            }
        }

        void destroy_impl() noexcept override {
            m_execute_func = nullptr;
        }

    private:
        const std::chrono::milliseconds m_interval;
        std::function<void(const JobExecutionContext &)> m_execute_func;
        std::atomic<std::chrono::steady_clock::time_point> m_last_execution;
    };

    JobManager::JobManager() {
        g_job_manager = this;

        register_job_and_ignore<DataModelWatcherJob>();
        register_job_and_ignore<LuauWaitingScriptJob>();
    }

    JobManager::~JobManager() {
        g_job_manager = nullptr;
    }

    std::expected<RBX::TaskScheduler::JobId, std::string> JobManager::register_lambda_job(
        std::string_view name,
        JobPriority priority,
        JobKind target_kind,
        std::function<bool(const JobExecutionContext &)> should_execute_func,
        std::function<void(const JobExecutionContext &)> execute_func
    ) noexcept {
        if (!g_task_scheduler) {
            return std::unexpected("TaskScheduler not initialized");
        }

        try {
            auto job = std::make_unique<LambdaJob>(
                name, priority, target_kind,
                std::move(should_execute_func),
                std::move(execute_func)
            );

            return g_task_scheduler->register_job(std::move(job));
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Failed to create lambda job: {}", e.what()));
        }
    }

    std::expected<RBX::TaskScheduler::JobId, std::string> JobManager::register_periodic_job(
        std::string_view name,
        std::chrono::milliseconds interval,
        JobPriority priority,
        JobKind target_kind,
        std::function<void(const JobExecutionContext &)> execute_func
    ) noexcept {
        if (!g_task_scheduler) {
            return std::unexpected("TaskScheduler not initialized");
        }

        try {
            auto job = std::make_unique<PeriodicJob>(
                name, interval, priority, target_kind,
                std::move(execute_func)
            );

            return g_task_scheduler->register_job(std::move(job));
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Failed to create periodic job: {}", e.what()));
        }
    }

    bool JobManager::unregister_job(const RBX::TaskScheduler::JobId job_id) noexcept {
        return g_task_scheduler && g_task_scheduler->unregister_job(job_id);
    }

    bool JobManager::unregister_job(const std::string_view job_name) noexcept {
        return g_task_scheduler && g_task_scheduler->unregister_job(job_name);
    }

    std::optional<RBX::TaskScheduler::JobStats> JobManager::get_job_stats(
        const RBX::TaskScheduler::JobId job_id) noexcept {
        return g_task_scheduler ? g_task_scheduler->get_job_stats(job_id) : std::nullopt;
    }

    std::vector<RBX::TaskScheduler::JobId> JobManager::get_jobs_by_kind(const JobKind kind) noexcept {
        return g_task_scheduler ? g_task_scheduler->get_jobs_by_kind(kind) : std::vector<RBX::TaskScheduler::JobId>{};
    }

    std::size_t JobManager::get_job_count() noexcept {
        return g_task_scheduler ? g_task_scheduler->get_job_count() : 0;
    }
}
