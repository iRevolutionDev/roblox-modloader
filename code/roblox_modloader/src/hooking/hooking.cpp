#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "pointers.hpp"

#include <utility>

RML_LOG_SCOPE("Hooking");

hooking::hooking()
{
	RML_INFO("Initializing hooking");

	for (const auto kind : {rml::JobKind::Heartbeat, rml::JobKind::Physics, rml::JobKind::WaitingHybridScripts, rml::JobKind::Render})
	{
		const auto vtable = rml::task_scheduler().get_vtable_for_job_kind(kind);

		if (!vtable.has_value())
		{
			RML_WARN("Failed to get vtable for job kind {}", std::to_underlying(kind));
			continue;
		}

		auto job_hook = std::make_unique<vtable_hook>(*vtable, rml::JobVtable::kSlotCount);
		job_hook->hook(rml::JobVtable::kStepIndex, &hooks::on_job_step);
		m_jobs_hook[kind] = std::move(job_hook);
		RML_DEBUG("Hooked job kind {} with vtable 0x{:X}", std::to_underlying(kind), reinterpret_cast<std::uintptr_t>(*vtable));
	}

	for (auto& detour_hook_helper : m_detour_hook_helpers)
	{
		detour_hook_helper.m_detour_hook->set_target_and_create_hook(detour_hook_helper.m_on_hooking_available());
	}

	//detour_hook_helper::add<hooks::rbx_crash>("RBX_CRASH", g_pointers->m_roblox_pointers.m_rbx_crash);
	// detour_hook_helper::add<hooks::render_prepare>("RENDER_PREPARE", g_pointers->m_roblox_pointers.m_render_prepare);
	// detour_hook_helper::add<hooks::render_perform>("RENDER_PERFORM", g_pointers->m_roblox_pointers.m_render_perform);
	// detour_hook_helper::add<hooks::render_view>("RENDER_VIEW", g_pointers->m_roblox_pointers.m_render_view);
#if RML_ENABLE_LUAU
	detour_hook_helper::add<hooks::luau_load>("LUAU_LOAD", g_pointers->m_roblox_pointers.luau_load);
#endif
	detour_hook_helper::add<hooks::build_menu_bar_from_dom>("MENU_BUILD_FROM_DOM", g_pointers->m_roblox_pointers.build_menu_bar_from_dom);

	g_hooking = this;
}

hooking::~hooking()
{
	if (m_enabled)
	{
		disable();
	}

	g_hooking = nullptr;
}

void hooking::enable()
{
	for (auto& job_hook : m_jobs_hook | std::views::values)
	{
		if (!job_hook)
			continue;

		if (const auto result = job_hook->enable(); !result)
			RML_ERROR("Failed to enable job vtable hook: {}", result.error().describe());
	}

	for (auto& detour_hook_helper : m_detour_hook_helpers)
	{
		if (const auto result = detour_hook_helper.m_detour_hook->enable(); !result)
			RML_ERROR("Failed to enable detour hook: {}", result.error().describe());
	}

	MH_ApplyQueued();

	m_enabled = true;
}

void hooking::disable()
{
	m_enabled = false;

	for (auto& job_hook : m_jobs_hook | std::views::values)
	{
		if (job_hook)
		{
			if (const auto result = job_hook->disable(); !result)
				RML_WARN("Failed to disable job vtable hook: {}", result.error().describe());
		}
	}

	for (auto& detour_hook_helper : m_detour_hook_helpers)
	{
		if (const auto result = detour_hook_helper.m_detour_hook->disable(); !result)
			RML_WARN("Failed to disable detour hook: {}", result.error().describe());
	}

	MH_ApplyQueued();

	m_detour_hook_helpers.clear();
}

hooking::detour_hook_helper::~detour_hook_helper()
{
}

void hooking::detour_hook_helper::enable_hook_if_hooking_is_already_running() const
{
	if (g_hooking && g_hooking->m_enabled)
	{
		if (m_on_hooking_available)
		{
			m_detour_hook->set_target_and_create_hook(m_on_hooking_available());
		}

		if (const auto result = m_detour_hook->enable(); !result)
			RML_ERROR("Failed to enable late-registered detour hook: {}", result.error().describe());

		MH_ApplyQueued();
	}
}
