#include "RobloxModLoader/common.hpp"
#include "luau_waiting_script_job.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/luau/script_engine.hpp"
#include "RobloxModLoader/luau/script_manager.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/waiting_hybrid_scripts_job.hpp"

namespace rml::jobs {
    LuauWaitingScriptJob::LuauWaitingScriptJob() noexcept
        : JobBase(JOB_NAME, JobPriority::High, JobKind::WaitingHybridScripts, true) {
    }

    bool LuauWaitingScriptJob::should_execute_impl(const JobExecutionContext &context) noexcept {
        const auto data_model = RBX::DataModel::from_job(context.job_as<RBX::DataModelJob>());
        if (!data_model) {
            return false;
        }

        const auto data_model_type = data_model->get_type();

        if (!g_task_scheduler) {
            return false;
        }

        const auto engine = g_task_scheduler->get_script_engine(data_model_type);
        return engine && engine->get_scheduler().get_total_queue_size();
    }

    void LuauWaitingScriptJob::execute_impl(const JobExecutionContext &context) {
        const auto job = context.job_as<RBX::ScriptContextFacets::WaitingHybridScriptsJob>();
        const auto data_model = RBX::DataModel::from_job(job);
        if (!data_model) {
            return;
        }

        const auto data_model_type = data_model->get_type();

        if (const auto engine = g_task_scheduler->get_script_engine(data_model_type)) {
            if (auto &scheduler = const_cast<luau::ScriptScheduler &>(engine->get_scheduler()); scheduler.step()) {
                LOG_DEBUG("[LuauWaitingScriptJob] Processed script from queue for DataModel type: {}",
                          static_cast<int>(data_model_type));
            }
        }
    }

    void LuauWaitingScriptJob::destroy_impl() noexcept {
    }
}
