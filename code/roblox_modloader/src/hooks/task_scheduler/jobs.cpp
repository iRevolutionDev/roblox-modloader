#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/roblox/task_scheduler.job.hpp"

#include <unordered_map>
#include <utility>

RBX::TaskScheduler::StepResult hooks::on_job_step(void** this_ptr, const RBX::Stats& time_metrics)
{
	if (!this_ptr || !*this_ptr)
	{
		// Intentional no-op: no job object to step, nothing failed.
		return RBX::TaskScheduler::StepResult::Stepped;
	}

	if (!g_hooking)
	{
		LOG_ERROR("[hooks::on_job_step] g_hooking is not initialized!");
		// Can't reach the original step function without g_hooking. Report Stepped rather
		// than Done so the engine keeps rescheduling the job instead of tearing it down.
		return RBX::TaskScheduler::StepResult::Stepped;
	}

	const auto vtable        = static_cast<void**>(*this_ptr);
	const auto detected_kind = [&]() -> rml::JobKind {
		if (!g_task_scheduler)
		{
			LOG_ERROR("[hooks::on_job_step] Task scheduler is not initialized, cannot determine job kind.");
			return rml::JobKind::Heartbeat;
		}

		const auto kind = g_task_scheduler->get_job_kind_from_vtable(vtable);
		if (!kind.has_value())
			return rml::JobKind::Heartbeat;

		return *kind;
	}();

	if (g_task_scheduler && !g_task_scheduler->is_shutdown())
	{
		try
		{
			const rml::JobExecutionContext context
			{
				.kind = detected_kind,
			    .job = this_ptr,
			    .stats = &time_metrics,
			    .delta_time = time_metrics.delta_time
			};

			g_task_scheduler->execute_jobs_for_kind(context);
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("[hooks::on_job_step] Exception executing custom jobs for kind {}: {}", std::to_underlying(detected_kind), e.what());
		}
		catch (...)
		{
			LOG_ERROR("[hooks::on_job_step] Unknown exception executing custom jobs for kind {}", std::to_underlying(detected_kind));
		}
	}

	if (const auto it = g_hooking->m_jobs_hook.find(detected_kind); it != g_hooking->m_jobs_hook.end() && it->second)
	{
		return it->second->get_original<decltype(&on_job_step)>(6)(this_ptr, time_metrics);
	}

	// No hook registered for this kind, so the real step could not be invoked this tick.
	// Report Stepped (not Done) so the engine retries instead of destroying the job.
	LOG_WARN("[hooks::on_job_step] No hook found for job kind {}, real step not invoked; reporting Stepped", std::to_underlying(detected_kind));
	return RBX::TaskScheduler::StepResult::Stepped;
}

void hooks::on_job_destroy(void** this_ptr)
{
	return hooking::get_original<&hooks::on_job_destroy>()(this_ptr);
}
