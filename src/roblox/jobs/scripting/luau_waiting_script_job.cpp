#include "RobloxModLoader/common.hpp"
#include "luau_waiting_script_job.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/waiting_hybrid_scripts_job.hpp"

namespace rml::jobs {
    LuauWaitingScriptJob::LuauWaitingScriptJob() noexcept
        : JobBase(JOB_NAME, JobPriority::High, JobKind::WaitingHybridScripts, true) {
    }

    bool LuauWaitingScriptJob::should_execute_impl(const JobExecutionContext &context) noexcept {
        const auto data_model = g_task_scheduler->get_data_model();
        return data_model != nullptr;
    }

    void LuauWaitingScriptJob::execute_impl(const JobExecutionContext &context) {
        const auto data_model = g_task_scheduler->get_data_model();
        const auto job = context.job_as<RBX::ScriptContextFacets::WaitingHybridScriptsJob>();
        const auto script_context = job->script_context;
        if (!data_model || script_context == nullptr) {
            LOG_ERROR("[LuauWaitingScriptJob] DataModel or ScriptContext is null, cannot execute job.");
            return;
        }
    }

    void LuauWaitingScriptJob::destroy_impl() noexcept {
    }
}
