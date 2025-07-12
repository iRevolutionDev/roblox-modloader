#include "roblox/jobs/data_model_watcher_job.hpp"
#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include <unordered_map>
#include <utility>
#include "tracy/Tracy.hpp"

void hooks::on_job_step(void **this_ptr, const uintptr_t time_metrics) {
    if (!this_ptr || !*this_ptr) {
        return;
    }

    const auto vtable = static_cast<void **>(*this_ptr);
    const auto detected_kind = [&]() -> JobKind {
        if (!g_task_scheduler) {
            LOG_ERROR("[hooks::on_job_step] Task scheduler is not initialized, cannot determine job kind.");
            return JobKind::Heartbeat;
        }

        const auto kind = g_task_scheduler->get_job_kind_from_vtable(vtable);
        if (!kind.has_value()) return JobKind::Heartbeat;

        return *kind;
    }();

    if (g_task_scheduler && !g_task_scheduler->is_shutdown()) {
        try {
            const JobExecutionContext context{
                .kind = detected_kind,
                .roblox_job_ptr = *this_ptr,
                .time_stats = time_metrics,
                .delta_time = 0
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

    g_hooking->m_jobs_hook[detected_kind]->get_original<decltype(&on_job_step)>(6)(this_ptr, time_metrics);
}

void hooks::on_job_destroy(void **this_ptr) {
    return hooking::get_original<&hooks::on_job_destroy>()(this_ptr);
}
