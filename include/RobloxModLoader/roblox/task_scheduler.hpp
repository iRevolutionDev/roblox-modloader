#pragma once
#include "job.hpp"
#include "RobloxModLoader/luau/script_engine.hpp"

namespace rml {
    enum class JobKind : std::uint8_t;
    struct JobExecutionContext;
}

namespace RBX {
    class ScriptContext;
    enum class DataModelType;
    class DataModel;

    class TaskScheduler final {
    public:
        typedef enum {
            Done,
            Stepped,
        } StepResult;

        using JobPtr = std::unique_ptr<rml::IJob>;
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

        void execute_jobs_for_kind(const rml::JobExecutionContext &context) noexcept;

        std::optional<std::reference_wrapper<rml::IJob> > get_job(JobId job_id) const noexcept;

        std::optional<std::reference_wrapper<rml::IJob> > get_job(std::string_view job_name) const noexcept;

        std::vector<JobId> get_jobs_by_kind(rml::JobKind kind) const noexcept;

        std::size_t get_job_count() const noexcept;

        struct JobStats {
            std::uint64_t executions = 0;
            std::uint64_t failures = 0;
            std::chrono::nanoseconds total_execution_time{0};
            std::chrono::nanoseconds average_execution_time{0};
        };

        std::optional<JobStats> get_job_stats(JobId job_id) const noexcept;

        void reset_stats() noexcept;

        void shutdown() noexcept;

        bool is_shutdown() const noexcept;

        std::optional<rml::JobKind> get_job_kind_from_vtable(void **vtable) const noexcept;

        std::optional<void **> get_vtable_for_job_kind(rml::JobKind kind) const noexcept;

        void set_data_model(DataModelType type, const DataModel *data_model, ScriptContext *script_context);

        const DataModel *get_data_model_by_type(DataModelType type) noexcept;

        std::shared_ptr<rml::luau::ScriptEngine> get_script_engine(DataModelType data_model_type);

        std::shared_ptr<rml::luau::ScriptEngine> create_or_get_script_engine(
            DataModelType data_model_type, ScriptContext *script_context);

        void cleanup_data_model(DataModelType data_model_type);

        void cleanup_script_engine(DataModelType data_model_type);

        void cleanup_orphaned_script_engines();

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
        std::shared_mutex m_data_model_mutex;
        mutable std::shared_mutex m_script_engines_mutex;

        std::unordered_map<DataModelType, const DataModel *> m_data_models;
        std::unordered_map<DataModelType, std::shared_ptr<rml::luau::ScriptEngine> > m_script_engines;

        std::unordered_map<JobId, JobEntry> m_jobs;
        std::unordered_map<std::string, JobId> m_name_to_id;
        std::atomic<JobId> m_next_job_id{1};

        std::atomic<bool> m_shutdown_requested{false};

        std::unordered_map<void **, rml::JobKind> m_vtable_to_kind;
        std::unordered_map<rml::JobKind, void **> m_kind_to_vtable;

        void initialize() noexcept;

        void initialize_vtable_mappings() noexcept;

        void cleanup_finished_jobs() noexcept;

        JobId generate_job_id() noexcept;

        static void execute_job_with_stats(JobEntry &entry, const rml::JobExecutionContext &context) noexcept;

        void setup_script_engine_environment(std::shared_ptr<rml::luau::ScriptEngine> engine,
                                             DataModelType data_model_type);

        void shutdown_script_engines() noexcept;
    };
}

inline RBX::TaskScheduler *g_task_scheduler{};
