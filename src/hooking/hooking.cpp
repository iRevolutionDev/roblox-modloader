#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

#include "pointers.hpp"

hooking::hooking() {
	LOG_INFO("Initializing hooking");

	for (auto &detour_hook_helper: m_detour_hook_helpers) {
		detour_hook_helper.m_detour_hook->set_target_and_create_hook(detour_hook_helper.m_on_hooking_available());
	}

	detour_hook_helper::add<hooks::rbx_crash>("RBX_CRASH", g_pointers->m_roblox_pointers.m_rbx_crash);
	detour_hook_helper::add<hooks::render_prepare>("RENDER_PREPARE", g_pointers->m_roblox_pointers.m_render_prepare);
	detour_hook_helper::add<hooks::render_perform>("RENDER_PERFORM", g_pointers->m_roblox_pointers.m_render_perform);
	detour_hook_helper::add<hooks::render_view>("RENDER_VIEW", g_pointers->m_roblox_pointers.m_render_view);

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
