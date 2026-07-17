#pragma once
#include "job.hpp"
#include "i_task_scheduler.hpp"

#if RML_ENABLE_LUAU
#include "RobloxModLoader/luau/script_engine_registry.hpp"
#endif

namespace rml {
    enum class JobKind : std::uint8_t;
    struct JobExecutionContext;
    class JobRegistry;
    class DataModelRegistry;
}

namespace RBX {
    class ScriptContext;
    enum class DataModelType;
    class DataModel;

    class TaskScheduler final : public rml::ITaskScheduler {
    public:
        typedef enum {
            Done,
            Stepped,
        } StepResult;

        using JobPtr = rml::ITaskScheduler::JobPtr;
        using JobId = rml::ITaskScheduler::JobId;
        using JobStats = rml::ITaskScheduler::JobStats;

        TaskScheduler();

        ~TaskScheduler() override;

        TaskScheduler(const TaskScheduler &) = delete;

        TaskScheduler &operator=(const TaskScheduler &) = delete;

        TaskScheduler(TaskScheduler &&) noexcept = delete;

        TaskScheduler &operator=(TaskScheduler &&) noexcept = delete;

        std::expected<JobId, std::string> register_job(JobPtr job) noexcept override;

        bool unregister_job(JobId job_id) noexcept override;

        bool unregister_job(std::string_view job_name) noexcept override;

        void execute_jobs_for_kind(const rml::JobExecutionContext &context) noexcept override;

        std::optional<std::reference_wrapper<rml::IJob> > get_job(JobId job_id) const noexcept override;

        std::optional<std::reference_wrapper<rml::IJob> > get_job(std::string_view job_name) const noexcept override;

        std::vector<JobId> get_jobs_by_kind(rml::JobKind kind) const noexcept override;

        std::size_t get_job_count() const noexcept override;

        std::optional<JobStats> get_job_stats(JobId job_id) const noexcept override;

        void reset_stats() noexcept override;

        void shutdown() noexcept override;

        bool is_shutdown() const noexcept override;

        std::optional<rml::JobKind> get_job_kind_from_vtable(void **vtable) const noexcept override;

        std::optional<void **> get_vtable_for_job_kind(rml::JobKind kind) const noexcept override;

        void set_data_model(DataModelType type, DataModel *data_model, ScriptContext *script_context) override;

        const DataModel *get_data_model_by_type(DataModelType type) noexcept override;

        void cleanup_data_model(DataModelType data_model_type) override;

#if RML_ENABLE_LUAU
        std::shared_ptr<rml::luau::ScriptEngine> get_script_engine(DataModelType data_model_type);

        std::shared_ptr<rml::luau::ScriptEngine> get_script_engine(lua_State *L);

        void cleanup_script_engine(DataModelType data_model_type);

        void cleanup_orphaned_script_engines();
#endif

    private:
        std::unique_ptr<rml::JobRegistry> m_job_registry;
        std::unique_ptr<rml::DataModelRegistry> m_data_model_registry;
#if RML_ENABLE_LUAU
        std::unique_ptr<rml::luau::ScriptEngineRegistry> m_script_engine_registry;
#endif
    };
}

namespace rml {
    [[nodiscard]] RML_EXPORT RBX::TaskScheduler &task_scheduler();

    [[nodiscard]] RML_EXPORT bool has_task_scheduler() noexcept;
}
