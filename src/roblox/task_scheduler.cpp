#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/job.hpp"
#include "RobloxModLoader/luau/script_engine.hpp"
#include "pointers.hpp"
#include "RobloxModLoader/memory/rtti_scanner.hpp"
#include "mod_manager.hpp"
#include <array>
#include <utility>
#include <thread>

#include "RobloxModLoader/roblox/script_context.hpp"

namespace RBX {
    TaskScheduler::TaskScheduler() {
        g_task_scheduler = this;

        initialize();
        initialize_vtable_mappings();
    }

    TaskScheduler::~TaskScheduler() {
        shutdown();

        g_task_scheduler = nullptr;
    }

    std::expected<TaskScheduler::JobId, std::string> TaskScheduler::register_job(JobPtr job) noexcept {
        if (!job) {
            return std::unexpected("Cannot register null job");
        }

        if (m_shutdown_requested.load(std::memory_order_acquire)) {
            return std::unexpected("TaskScheduler is shutting down");
        }

        const auto job_name = job->get_name();
        const auto job_id = generate_job_id();

        std::unique_lock lock(m_jobs_mutex);

        if (m_name_to_id.contains(std::string(job_name))) {
            return std::unexpected(std::format("Job with name '{}' already exists", job_name));
        }

        try {
            m_jobs.emplace(job_id, JobEntry(std::move(job)));
            m_name_to_id.emplace(job_name, job_id);

            LOG_DEBUG("[TaskScheduler] Registered job '{}' with ID {}", job_name, job_id);
            return job_id;
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Failed to register job '{}': {}", job_name, e.what()));
        }
    }

    bool TaskScheduler::unregister_job(const JobId job_id) noexcept {
        std::unique_lock lock(m_jobs_mutex);

        const auto it = m_jobs.find(job_id);
        if (it == m_jobs.end()) {
            return false;
        }

        const auto job_name = it->second.job->get_name();

        it->second.job->destroy();

        m_name_to_id.erase(std::string(job_name));
        m_jobs.erase(it);

        LOG_DEBUG("[TaskScheduler] Unregistered job '{}' (ID: {})", job_name, job_id);
        return true;
    }

    bool TaskScheduler::unregister_job(std::string_view job_name) noexcept {
        std::shared_lock shared_lock(m_jobs_mutex);

        const auto name_it = m_name_to_id.find(std::string(job_name));
        if (name_it == m_name_to_id.end()) {
            return false;
        }

        const auto job_id = name_it->second;
        shared_lock.unlock();

        return unregister_job(job_id);
    }

    void TaskScheduler::execute_jobs_for_kind(const rml::JobExecutionContext &context) noexcept {
        if (m_shutdown_requested.load(std::memory_order_acquire)) {
            return;
        }

        std::vector<std::pair<JobId, std::reference_wrapper<JobEntry> > > jobs_to_execute; {
            std::shared_lock lock(m_jobs_mutex);
            jobs_to_execute.reserve(m_jobs.size());

            for (auto &[job_id, entry]: m_jobs) {
                if (entry.job->should_execute(context)) {
                    jobs_to_execute.emplace_back(job_id, std::ref(entry));
                }
            }
        }

        std::ranges::sort(jobs_to_execute, [](const auto &a, const auto &b) {
            const auto priority_a = a.second.get().job->get_priority();
            const auto priority_b = b.second.get().job->get_priority();
            return static_cast<std::int32_t>(priority_a) < static_cast<std::int32_t>(priority_b);
        });

        for (auto &entry_ref: jobs_to_execute | std::views::values) {
            execute_job_with_stats(entry_ref.get(), context);
        }

        static std::atomic<std::uint64_t> cleanup_counter{0};
        const auto current_count = cleanup_counter.fetch_add(1);

        if (current_count % 100 == 0) {
            cleanup_finished_jobs();
        }

        if (current_count % 500 == 0) {
            cleanup_orphaned_script_engines();
        }
    }

    std::optional<std::reference_wrapper<rml::IJob> > TaskScheduler::get_job(const JobId job_id) const noexcept {
        std::shared_lock lock(m_jobs_mutex);

        if (const auto it = m_jobs.find(job_id); it != m_jobs.end()) {
            return std::ref(*it->second.job);
        }

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<rml::IJob> > TaskScheduler::get_job(
        const std::string_view job_name) const noexcept {
        std::shared_lock lock(m_jobs_mutex);

        const auto name_it = m_name_to_id.find(std::string(job_name));
        if (name_it == m_name_to_id.end()) {
            return std::nullopt;
        }

        if (const auto job_it = m_jobs.find(name_it->second); job_it != m_jobs.end()) {
            return std::ref(*job_it->second.job);
        }

        return std::nullopt;
    }

    std::vector<TaskScheduler::JobId> TaskScheduler::get_jobs_by_kind(rml::JobKind kind) const noexcept {
        std::vector<JobId> result;
        std::shared_lock lock(m_jobs_mutex);

        for (const auto &[job_id, entry]: m_jobs) {
            if (has_job_kind(entry.job->get_target_kind(), kind) || kind == rml::JobKind::Custom) {
                result.push_back(job_id);
            }
        }

        return result;
    }

    std::size_t TaskScheduler::get_job_count() const noexcept {
        std::shared_lock lock(m_jobs_mutex);
        return m_jobs.size();
    }

    std::optional<TaskScheduler::JobStats> TaskScheduler::get_job_stats(const JobId job_id) const noexcept {
        std::shared_lock lock(m_jobs_mutex);

        if (const auto it = m_jobs.find(job_id); it != m_jobs.end()) {
            return it->second.stats;
        }

        return std::nullopt;
    }

    void TaskScheduler::reset_stats() noexcept {
        std::unique_lock lock(m_jobs_mutex);

        for (auto &entry: m_jobs | std::views::values) {
            entry.stats = JobStats{};
        }
    }

    void TaskScheduler::shutdown() noexcept {
        LOG_INFO("[TaskScheduler] Shutting down TaskScheduler...");

        m_shutdown_requested.store(true, std::memory_order_release);

        shutdown_script_engines();

        std::unique_lock lock(m_jobs_mutex);

        for (const auto &entry: m_jobs | std::views::values) {
            entry.job->destroy();
        }

        m_jobs.clear();
        m_name_to_id.clear();

        LOG_INFO("[TaskScheduler] Shutdown completed");
    }

    bool TaskScheduler::is_shutdown() const noexcept {
        return m_shutdown_requested.load(std::memory_order_acquire);
    }

    std::optional<rml::JobKind> TaskScheduler::get_job_kind_from_vtable(void **vtable) const noexcept {
        if (const auto it = m_vtable_to_kind.find(vtable); it != m_vtable_to_kind.end()) {
            return it->second;
        }

        return std::nullopt;
    }

    std::optional<void **> TaskScheduler::get_vtable_for_job_kind(const rml::JobKind kind) const noexcept {
        if (const auto it = m_kind_to_vtable.find(kind); it != m_kind_to_vtable.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void TaskScheduler::set_data_model(const DataModelType type, const DataModel *data_model,
                                       ScriptContext *script_context) {
        const DataModel *old_data_model = nullptr; {
            std::shared_lock lock(m_data_model_mutex);
            if (const auto it = m_data_models.find(type); it != m_data_models.end()) {
                old_data_model = it->second;
            }
        }

        if (old_data_model && old_data_model != data_model) {
            LOG_INFO("[TaskScheduler] DataModel type {} changed, cleaning up old instance",
                     static_cast<int>(type));
            cleanup_script_engine(type);
        } {
            std::unique_lock lock(m_data_model_mutex);
            if (data_model) {
                m_data_models[type] = data_model;
            } else {
                m_data_models.erase(type);
            }
        }

        if (data_model) {
            create_or_get_script_engine(type, script_context);
        }
    }

    const DataModel *TaskScheduler::get_data_model_by_type(const DataModelType type) noexcept {
        std::shared_lock lock(m_data_model_mutex);

        const auto it = m_data_models.find(type);
        if (it == m_data_models.end()) return nullptr;

        return it->second;
    }

    std::shared_ptr<rml::luau::ScriptEngine> TaskScheduler::get_script_engine(DataModelType data_model_type) {
        std::shared_lock lock(m_script_engines_mutex);

        if (const auto it = m_script_engines.find(data_model_type); it != m_script_engines.end()) {
            return it->second;
        }

        return nullptr;
    }

    void TaskScheduler::initialize() noexcept {
        LOG_INFO("[TaskScheduler] Initializing TaskScheduler...");

        m_script_engines.clear();

        LOG_INFO("[TaskScheduler] TaskScheduler initialized successfully.");
    }

    std::shared_ptr<rml::luau::ScriptEngine> TaskScheduler::create_or_get_script_engine(
        DataModelType data_model_type, ScriptContext *script_context) {
        std::unique_lock lock(m_script_engines_mutex);

        if (const auto it = m_script_engines.find(data_model_type); it != m_script_engines.end()) {
            return it->second;
        }

        LOG_INFO("[TaskScheduler] Creating ScriptEngine for DataModel type: {}", static_cast<int>(data_model_type));

        if (!script_context) {
            return nullptr;
        }

        const auto global_state = script_context->get_global_state();
        const auto L = lua_newthread(global_state);

        rml::luau::ScriptContext::Context options{
            .L = L,
        };

        auto script_engine = std::make_shared<rml::luau::ScriptEngine>(options);

        if (!script_engine->initialize()) {
            LOG_ERROR("[TaskScheduler] Failed to initialize ScriptEngine for DataModel type: {}",
                      static_cast<int>(data_model_type));
            return nullptr;
        }

        m_script_engines[data_model_type] = script_engine;

        LOG_INFO("[TaskScheduler] ScriptEngine created successfully for DataModel type: {}",
                 static_cast<int>(data_model_type));

        return script_engine;
    }

    void TaskScheduler::shutdown_script_engines() noexcept {
        std::unique_lock lock(m_script_engines_mutex);

        LOG_INFO("[TaskScheduler] Shutting down all ScriptEngines...");

        for (auto &[data_model_type, engine]: m_script_engines) {
            if (engine) {
                LOG_INFO("[TaskScheduler] Shutting down ScriptEngine for DataModel type: {}",
                         static_cast<int>(data_model_type));
                engine->shutdown();
            }
        }

        m_script_engines.clear();

        LOG_INFO("[TaskScheduler] All ScriptEngines shut down successfully.");
    }

    void TaskScheduler::initialize_vtable_mappings() noexcept {
        static constexpr std::array<std::pair<std::string_view, rml::JobKind>, 4> known_job_classes{
            {
                {"RBX::HeartbeatTask", rml::JobKind::Heartbeat},
                {"RBX::PhysicsJob", rml::JobKind::Physics},
                {"RBX::ScriptContextFacets::WaitingHybridScriptsJob", rml::JobKind::WaitingHybridScripts},
                {"RBX::Studio::RenderJob", rml::JobKind::Render}
            }
        };

        m_vtable_to_kind.reserve(known_job_classes.size());
        m_kind_to_vtable.reserve(known_job_classes.size());

        for (const auto &[class_name, job_kind]: known_job_classes) {
            const auto rtti = memory::rtti::rtti_manager::get_class_rtti(class_name);
            if (!rtti) {
                LOG_WARN("[TaskScheduler] RTTI for '{}' not found, skipping vtable mapping", class_name);
                continue;
            }

            const auto vtable = rtti->get_virtual_function_table();
            if (!vtable) {
                LOG_WARN("[TaskScheduler] Failed to get vtable for '{}'", class_name);
                continue;
            }

            m_vtable_to_kind.emplace(vtable, job_kind);
            m_kind_to_vtable.emplace(job_kind, vtable);

            LOG_INFO("[TaskScheduler] Mapped vtable for '{}' (kind: {}) -> 0x{:X}",
                     class_name, std::to_underlying(job_kind),
                     reinterpret_cast<std::uintptr_t>(vtable));
        }

        LOG_INFO("[TaskScheduler] Initialized vtable mappings: {}/{} job types mapped",
                 m_vtable_to_kind.size(), known_job_classes.size());
    }

    void TaskScheduler::cleanup_finished_jobs() noexcept {
        std::unique_lock lock(m_jobs_mutex);

        auto it = m_jobs.begin();
        while (it != m_jobs.end()) {
            if (it->second.job->get_state() == rml::JobState::Destroyed) {
                const auto job_name = it->second.job->get_name();
                m_name_to_id.erase(std::string(job_name));
                it = m_jobs.erase(it);
                LOG_DEBUG("[TaskScheduler] Cleaned up destroyed job '{}'", job_name);
            } else {
                ++it;
            }
        }
    }

    TaskScheduler::JobId TaskScheduler::generate_job_id() noexcept {
        return m_next_job_id.fetch_add(1, std::memory_order_relaxed);
    }

    void TaskScheduler::execute_job_with_stats(JobEntry &entry, const rml::JobExecutionContext &context) noexcept {
        const auto start_time = std::chrono::steady_clock::now();

        try {
            entry.job->execute(context);
            entry.stats.executions++;
        } catch (const std::exception &e) {
            entry.stats.failures++;
            LOG_ERROR("[TaskScheduler] Job '{}' execution failed: {}",
                      entry.job->get_name(), e.what());
        } catch (...) {
            entry.stats.failures++;
            LOG_ERROR("[TaskScheduler] Job '{}' execution failed with unknown exception",
                      entry.job->get_name());
        }

        const auto end_time = std::chrono::steady_clock::now();
        const auto execution_time = end_time - start_time;

        entry.stats.total_execution_time += execution_time;
        entry.stats.average_execution_time = entry.stats.total_execution_time /
                                             std::max(entry.stats.executions, 1ULL);
        entry.last_execution = end_time;
    }

    void TaskScheduler::cleanup_data_model(DataModelType data_model_type) {
        LOG_INFO("[TaskScheduler] Cleaning up DataModel type: {}", static_cast<int>(data_model_type));
        cleanup_script_engine(data_model_type); {
            std::unique_lock lock(m_data_model_mutex);
            m_data_models.erase(data_model_type);
        }

        LOG_INFO("[TaskScheduler] DataModel type {} cleanup completed", static_cast<int>(data_model_type));
    }

    void TaskScheduler::cleanup_script_engine(DataModelType data_model_type) {
        LOG_INFO("[TaskScheduler] Cleaning up ScriptEngine for DataModel type: {}", static_cast<int>(data_model_type));

        std::shared_ptr<rml::luau::ScriptEngine> engine_to_cleanup; {
            std::unique_lock lock(m_script_engines_mutex);
            if (const auto it = m_script_engines.find(data_model_type); it != m_script_engines.end()) {
                engine_to_cleanup = it->second;
                m_script_engines.erase(it);
            }
        }

        if (!engine_to_cleanup) return;

        std::thread([engine = std::move(engine_to_cleanup), data_model_type]() {
            try {
                LOG_INFO("[TaskScheduler] Shutting down ScriptEngine for DataModel type: {}",
                         static_cast<int>(data_model_type));
                engine->shutdown();
                LOG_INFO("[TaskScheduler] ScriptEngine shutdown completed for DataModel type: {}",
                         static_cast<int>(data_model_type));
            } catch (const std::exception &e) {
                LOG_ERROR("[TaskScheduler] Error shutting down ScriptEngine for DataModel type {}: {}",
                          static_cast<int>(data_model_type), e.what());
            }
        }).detach();
    }

    void TaskScheduler::cleanup_orphaned_script_engines() {
        std::vector<DataModelType> orphaned_types; {
            std::shared_lock engines_lock(m_script_engines_mutex);
            std::shared_lock models_lock(m_data_model_mutex);

            for (const auto &data_model_type: m_script_engines | std::views::keys) {
                if (auto it = m_data_models.find(data_model_type); it == m_data_models.end() || it->second == nullptr) {
                    orphaned_types.push_back(data_model_type);
                }
            }
        }

        for (const auto &orphaned_type: orphaned_types) {
            LOG_WARN("[TaskScheduler] Found orphaned ScriptEngine for DataModel type: {}, cleaning up",
                     static_cast<int>(orphaned_type));
            cleanup_script_engine(orphaned_type);
        }
    }
}
