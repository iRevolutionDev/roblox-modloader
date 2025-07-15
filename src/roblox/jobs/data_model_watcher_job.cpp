#include "RobloxModLoader/common.hpp"
#include "data_model_watcher_job.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/script_context.hpp"
#include "RobloxModLoader/luau/script_manager.hpp"
#include "RobloxModLoader/roblox/waiting_hybrid_scripts_job.hpp"

namespace rml::jobs {
    DataModelWatcherJob::DataModelWatcherJob() noexcept
        : JobBase(JOB_NAME, JobPriority::High, JobKind::WaitingHybridScripts, true) {
    }

    bool DataModelWatcherJob::should_execute_impl(const JobExecutionContext &context) noexcept {
        const auto data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
        if (!data_model) {
            return false;
        }

        const auto type = data_model->get_type();

        m_data_models[type] = data_model;
        m_data_model_last_time_stepped[type] = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::high_resolution_clock::now() - m_last_check
               ).count() > 16;
    }

    void DataModelWatcherJob::execute_impl(const JobExecutionContext &context) {
        m_last_check = std::chrono::high_resolution_clock::now();

        const auto job = context.job_as<RBX::ScriptContextFacets::WaitingHybridScriptsJob>();
        const auto new_data_model = RBX::DataModel::from_job(job);
        if (!new_data_model) {
            return;
        }

        const auto old_data_model = g_task_scheduler->get_data_model_by_type(new_data_model->get_type());
        if (old_data_model == new_data_model) {
            return;
        }

        check_and_cleanup_stale_data_models();

        on_data_model_changed(
            old_data_model,
            new_data_model,
            job->script_context
        );
    }

    void DataModelWatcherJob::destroy_impl() noexcept {
    }

    void DataModelWatcherJob::on_data_model_changed(const RBX::DataModel *old_data_model,
                                                    const RBX::DataModel *new_data_model,
                                                    RBX::ScriptContext *script_context) {
        if (g_task_scheduler == nullptr) {
            LOG_ERROR("TaskScheduler is null, cannot set new DataModel.");
            return;
        }

        if (old_data_model == new_data_model) {
            return;
        }

        LOG_INFO("DataModel changed from 0x{:X} to 0x{:X}",
                 old_data_model ? reinterpret_cast<uintptr_t>(old_data_model) : 0,
                 new_data_model ? reinterpret_cast<uintptr_t>(new_data_model) : 0);

        g_task_scheduler->set_data_model(new_data_model->get_type(), new_data_model, script_context);

        const auto data_model_type = new_data_model->get_type();

        LOG_INFO("New DataModel type: {}, executing context scripts",
                 static_cast<int>(data_model_type));

        try {
            luau::g_script_manager->execute_scripts_for_context(data_model_type);
            LOG_INFO("Successfully triggered mod scripts for DataModel type: {}",
                     static_cast<int>(data_model_type));
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to execute mod scripts for DataModel type {}: {}",
                      static_cast<int>(data_model_type), e.what());
        }
    }

    void DataModelWatcherJob::check_and_cleanup_stale_data_models() {
        const auto now = std::chrono::high_resolution_clock::now();
        constexpr auto stale_threshold = std::chrono::seconds{5};

        std::vector<RBX::DataModelType> stale_types;
        for (auto it = m_data_model_last_time_stepped.begin();
             it != m_data_model_last_time_stepped.end();) {
            const auto &[data_model_type, last_time] = *it;

            if (const auto time_since_last_step = now - last_time; time_since_last_step <= stale_threshold) {
                ++it;
                continue;
            }

            const auto current_data_model = g_task_scheduler->get_data_model_by_type(data_model_type);
            const auto tracked_data_model = m_data_models.find(data_model_type);

            bool should_cleanup = false;

            if (current_data_model) {
                if (tracked_data_model != m_data_models.end() &&
                    tracked_data_model->second != current_data_model) {
                    should_cleanup = true;
                }
            } else {
                should_cleanup = true;
            }

            if (!should_cleanup) {
                ++it;
                continue;
            }

            LOG_INFO("Detected stale DataModel type: {}, cleaning up",
                     static_cast<int>(data_model_type));

            stale_types.push_back(data_model_type);
            m_data_models.erase(data_model_type);
            it = m_data_model_last_time_stepped.erase(it);
        }

        for (const auto &stale_type: stale_types) {
            g_task_scheduler->cleanup_data_model(stale_type);
        }
    }
}
