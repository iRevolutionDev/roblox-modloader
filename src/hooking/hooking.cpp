#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

#include "pointers.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include <utility>

hooking::hooking() {
	LOG_INFO("Initializing hooking");

	for (const auto kind: {
		     rml::JobKind::Heartbeat, rml::JobKind::Physics, rml::JobKind::WaitingHybridScripts, rml::JobKind::Render
	     }) {
		const auto vtable = g_task_scheduler->get_vtable_for_job_kind(kind);

		if (!vtable.has_value()) {
			LOG_WARN("[hooking] Failed to get vtable for job kind {}", std::to_underlying(kind));
			continue;
		}

		auto job_hook = std::make_unique<vtable_hook>(*vtable, 7);
		job_hook->hook(6, &hooks::on_job_step);
		m_jobs_hook[kind] = std::move(job_hook);
		LOG_DEBUG("[hooking] Hooked job kind {} with vtable 0x{:X}", std::to_underlying(kind),
		          reinterpret_cast<std::uintptr_t>(*vtable));
	}

	for (auto &detour_hook_helper: m_detour_hook_helpers) {
		detour_hook_helper.m_detour_hook->set_target_and_create_hook(detour_hook_helper.m_on_hooking_available());
	}

	detour_hook_helper::add<hooks::rbx_crash>("RBX_CRASH", g_pointers->m_roblox_pointers.m_rbx_crash);
	detour_hook_helper::add<hooks::render_prepare>("RENDER_PREPARE", g_pointers->m_roblox_pointers.m_render_prepare);
	detour_hook_helper::add<hooks::render_perform>("RENDER_PERFORM", g_pointers->m_roblox_pointers.m_render_perform);
	detour_hook_helper::add<hooks::render_view>("RENDER_VIEW", g_pointers->m_roblox_pointers.m_render_view);
	detour_hook_helper::add<hooks::resume_waiting_scripts>("RESUME_WAITING_SCRIPTS",
	                                                       g_pointers->m_roblox_pointers.resume_waiting_scripts);
	detour_hook_helper::add<hooks::profile_log>("PROFILE_BEGIN",
	                                            g_pointers->m_roblox_pointers.m_profile_log);

	g_hooking = this;
}

hooking::~hooking() {
	if (m_enabled) {
		disable();
	}

	g_hooking = nullptr;
}

void hooking::enable() {
	for (auto &detour_hook_helper: m_detour_hook_helpers) {
		detour_hook_helper.m_detour_hook->enable();
	}

	MH_ApplyQueued();

	m_enabled = true;
}

void hooking::disable() {
	m_enabled = false;

	for (auto &job_hook: m_jobs_hook | std::views::values) {
		if (job_hook) {
			job_hook->disable();
		}
	}

	for (auto &detour_hook_helper: m_detour_hook_helpers) {
		detour_hook_helper.m_detour_hook->disable();
	}

	MH_ApplyQueued();

	m_detour_hook_helpers.clear();
}

hooking::detour_hook_helper::~detour_hook_helper() {
}

void hooking::detour_hook_helper::enable_hook_if_hooking_is_already_running() {
	if (g_hooking && g_hooking->m_enabled) {
		if (m_on_hooking_available) {
			m_detour_hook->set_target_and_create_hook(m_on_hooking_available());
		}

		m_detour_hook->enable();
		MH_ApplyQueued();
	}
}
