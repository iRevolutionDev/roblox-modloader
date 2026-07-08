#include "RobloxModLoader/roblox/task_scheduler.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "data_model_registry.hpp"
#include "job_registry.hpp"

#include <cassert>

RML_LOG_SCOPE("TaskScheduler");

static RBX::TaskScheduler* s_active_task_scheduler{};

namespace RBX
{
	TaskScheduler::TaskScheduler() :
	    m_job_registry(std::make_unique<rml::JobRegistry>()),
	    m_data_model_registry(std::make_unique<rml::DataModelRegistry>())
#if RML_ENABLE_LUAU
	    , m_script_engine_registry(std::make_unique<rml::luau::ScriptEngineRegistry>())
#endif
	{
		s_active_task_scheduler = this;

		RML_INFO("TaskScheduler initialized successfully.");
	}

	TaskScheduler::~TaskScheduler()
	{
		shutdown();

		s_active_task_scheduler = nullptr;
	}

	std::expected<TaskScheduler::JobId, std::string> TaskScheduler::register_job(JobPtr job) noexcept
	{
		return m_job_registry->register_job(std::move(job));
	}

	bool TaskScheduler::unregister_job(const JobId job_id) noexcept
	{
		return m_job_registry->unregister_job(job_id);
	}

	bool TaskScheduler::unregister_job(const std::string_view job_name) noexcept
	{
		return m_job_registry->unregister_job(job_name);
	}

	void TaskScheduler::execute_jobs_for_kind(const rml::JobExecutionContext& context) noexcept
	{
		m_job_registry->execute_jobs_for_kind(context);

#if RML_ENABLE_LUAU
		m_script_engine_registry->maybe_cleanup_orphaned_script_engines([this](const DataModelType data_model_type) {
			return m_data_model_registry->get_data_model_by_type(data_model_type);
		});
#endif
	}

	std::optional<std::reference_wrapper<rml::IJob> > TaskScheduler::get_job(const JobId job_id) const noexcept
	{
		return m_job_registry->get_job(job_id);
	}

	std::optional<std::reference_wrapper<rml::IJob> > TaskScheduler::get_job(const std::string_view job_name) const noexcept
	{
		return m_job_registry->get_job(job_name);
	}

	std::vector<TaskScheduler::JobId> TaskScheduler::get_jobs_by_kind(const rml::JobKind kind) const noexcept
	{
		return m_job_registry->get_jobs_by_kind(kind);
	}

	std::size_t TaskScheduler::get_job_count() const noexcept
	{
		return m_job_registry->get_job_count();
	}

	std::optional<TaskScheduler::JobStats> TaskScheduler::get_job_stats(const JobId job_id) const noexcept
	{
		return m_job_registry->get_job_stats(job_id);
	}

	void TaskScheduler::reset_stats() noexcept
	{
		m_job_registry->reset_stats();
	}

	void TaskScheduler::shutdown() noexcept
	{
		RML_INFO("Shutting down TaskScheduler...");

#if RML_ENABLE_LUAU
		m_script_engine_registry->shutdown();
#endif

		m_job_registry->shutdown();

		RML_INFO("Shutdown completed");
	}

	bool TaskScheduler::is_shutdown() const noexcept
	{
		return m_job_registry->is_shutdown();
	}

	std::optional<rml::JobKind> TaskScheduler::get_job_kind_from_vtable(void** vtable) const noexcept
	{
		return m_job_registry->get_job_kind_from_vtable(vtable);
	}

	std::optional<void**> TaskScheduler::get_vtable_for_job_kind(const rml::JobKind kind) const noexcept
	{
		return m_job_registry->get_vtable_for_job_kind(kind);
	}

	void TaskScheduler::set_data_model(const DataModelType type, DataModel* data_model, ScriptContext* script_context)
	{
		m_data_model_registry->set_data_model(type, data_model, script_context);
	}

	const DataModel* TaskScheduler::get_data_model_by_type(const DataModelType type) noexcept
	{
		return m_data_model_registry->get_data_model_by_type(type);
	}

	void TaskScheduler::cleanup_data_model(const DataModelType data_model_type)
	{
		m_data_model_registry->cleanup_data_model(data_model_type);
	}

#if RML_ENABLE_LUAU
	std::shared_ptr<rml::luau::ScriptEngine> TaskScheduler::get_script_engine(const DataModelType data_model_type)
	{
		return m_script_engine_registry->get_script_engine(data_model_type);
	}

	std::shared_ptr<rml::luau::ScriptEngine> TaskScheduler::get_script_engine(lua_State* L)
	{
		return m_script_engine_registry->get_script_engine(L);
	}

	void TaskScheduler::cleanup_script_engine(const DataModelType data_model_type)
	{
		m_script_engine_registry->cleanup_script_engine(data_model_type);
	}

	void TaskScheduler::cleanup_orphaned_script_engines()
	{
		m_script_engine_registry->cleanup_orphaned_script_engines([this](const DataModelType data_model_type) {
			return m_data_model_registry->get_data_model_by_type(data_model_type);
		});
	}
#endif
}

namespace rml
{
	RBX::TaskScheduler& task_scheduler()
	{
		assert(s_active_task_scheduler && "TaskScheduler accessed before Application initialized it");
		return *s_active_task_scheduler;
	}

	bool has_task_scheduler() noexcept
	{
		return s_active_task_scheduler != nullptr;
	}
}
