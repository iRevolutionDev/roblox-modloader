#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/task_scheduler.job.hpp"
#include <unordered_map>
#include <utility>

RBX::TaskScheduler::StepResult hooks::on_job_step(void **this_ptr, const RBX::Stats &time_metrics) {
    if (!this_ptr || !*this_ptr) {
        return RBX::TaskScheduler::StepResult::Stepped; // No job to step, return early.
    }

    const auto vtable = static_cast<void **>(*this_ptr);
    const auto detected_kind = [&]() -> rml::JobKind {
        if (!g_task_scheduler) {
            LOG_ERROR("[hooks::on_job_step] Task scheduler is not initialized, cannot determine job kind.");
            return rml::JobKind::Heartbeat;
        }

        const auto kind = g_task_scheduler->get_job_kind_from_vtable(vtable);
        if (!kind.has_value()) return rml::JobKind::Heartbeat;

        return *kind;
    }();

    if (g_task_scheduler && !g_task_scheduler->is_shutdown()) {
        try {
            const rml::JobExecutionContext context{
                .kind = detected_kind,
                .job = this_ptr,
                .stats = const_cast<RBX::Stats *>(&time_metrics),
                .delta_time = time_metrics.delta_time
            };
            g_task_scheduler->execute_jobs_for_kind(context);
        } catch (const std::exception &e) {
            LOG_ERROR("[hooks::on_job_step] Exception executing custom jobs for kind {}: {}",
                      std::to_underlying(detected_kind), e.what());
        } catch (...) {
            LOG_ERROR("[hooks::on_job_step] Unknown exception executing custom jobs for kind {}",
                      std::to_underlying(detected_kind));
        }
    }

    return g_hooking->m_jobs_hook[detected_kind]->get_original<decltype(&on_job_step)>(6)(
        this_ptr, time_metrics);
}

void hooks::on_job_destroy(void **this_ptr) {
    return hooking::get_original<&hooks::on_job_destroy>()(this_ptr);
}
