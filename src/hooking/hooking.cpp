#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

hooking::hooking() {
	LOG_INFO("Initializing hooking");

	for (auto &detour_hook_helper: m_detour_hook_helpers) {
		detour_hook_helper.m_detour_hook->set_target_and_create_hook(detour_hook_helper.m_on_hooking_available());
	}

	// auto authentication_func = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0x54A9800;
	const auto on_authentication = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0x54AA1D0;
	detour_hook_helper::add<hooks::on_authentication>("AUTH", reinterpret_cast<void *>(on_authentication));

	const auto is_internal = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)) + 0x40EA280;
	detour_hook_helper::add<hooks::is_internal>("IS_INTERNAL", reinterpret_cast<void *>(is_internal));

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
