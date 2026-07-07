#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/detour_hook.hpp"
#include "RobloxModLoader/memory/handle.hpp"
#include <MinHook.h>

RML_LOG_SCOPE("DetourHook");

detour_hook::detour_hook() {
}

detour_hook::detour_hook(const std::string &name, void *detour) {
	set_instance(name, detour);
}

detour_hook::detour_hook(const std::string &name, void *target, void *detour) {
	set_instance(name, target, detour);
}

void detour_hook::set_instance(const std::string &name, void *detour) {
	m_name = name;
	m_detour = detour;
}

void detour_hook::set_instance(const std::string &name, void *target, void *detour) {
	m_name = name;
	m_target = target;
	m_detour = detour;

	create_hook();
}

void detour_hook::set_target_and_create_hook(void *target) {
	m_target = target;
	create_hook();
}

void detour_hook::create_hook() {
	if (!m_target)
		return;

	fix_hook_address();
	if (const auto status = MH_CreateHook(m_target, m_detour, &m_original); status != MH_OK)
		RML_ERROR("Failed to create hook '{}' at 0x{:X} (error: {})", m_name, uintptr_t(m_target),
	          MH_StatusToString(status));
}

detour_hook::~detour_hook() noexcept {
	if (!m_target)
		return;

	if (const auto status = MH_RemoveHook(m_target); status != MH_OK)
		RML_ERROR("Failed to remove hook '{}' at 0x{:X} (error: {})", m_name, uintptr_t(m_target),
	          MH_StatusToString(status));
}

std::expected<void, rml::HookError> detour_hook::enable() {
	if (!m_target) {
		const auto error = rml::HookError::from_minhook(m_name, 0, MH_ERROR_NOT_CREATED);
		RML_ERROR("Failed to enable hook '{}': {}", m_name, error.describe());
		return std::unexpected(error);
	}

	if (const auto status = MH_QueueEnableHook(m_target); status != MH_OK) {
		const auto error = rml::HookError::from_minhook(m_name, uintptr_t(m_target), status);
		RML_ERROR("Failed to enable hook '{}': {}", m_name, error.describe());
		return std::unexpected(error);
	}

	m_enabled = true;
	return {};
}

std::expected<void, rml::HookError> detour_hook::disable() {
	if (!m_target) {
		const auto error = rml::HookError::from_minhook(m_name, 0, MH_ERROR_NOT_CREATED);
		RML_WARN("Failed to disable hook '{}': {}", m_name, error.describe());
		return std::unexpected(error);
	}

	if (const auto status = MH_QueueDisableHook(m_target); status != MH_OK) {
		const auto error = rml::HookError::from_minhook(m_name, uintptr_t(m_target), status);
		RML_WARN("Failed to disable hook '{}': {}", m_name, error.describe());
		return std::unexpected(error);
	}

	m_enabled = false;
	return {};
}

bool detour_hook::is_enabled() const {
	return m_enabled;
}

DWORD exp_handler(PEXCEPTION_POINTERS exp, std::string const &name) {
	return exp->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION
		       ? EXCEPTION_EXECUTE_HANDLER
		       : EXCEPTION_CONTINUE_SEARCH;
}

void detour_hook::fix_hook_address() {
	auto ptr = memory::handle(m_target);
	while (ptr.as<uint8_t &>() == 0xE9)
		ptr = ptr.add(1).rip();
	m_target = ptr.as<void *>();
}
