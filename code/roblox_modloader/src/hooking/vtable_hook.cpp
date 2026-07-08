#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/vtable_hook.hpp"

#include "utils/memory_protection_guard.hpp"

RML_LOG_SCOPE("VtableHook");

vtable_hook::vtable_hook(void **vft, std::size_t num_funcs) : m_num_funcs(num_funcs),
                                                              m_table(vft),
                                                              m_backup_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_hook_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_enabled(false) {
	std::memcpy(m_backup_table.get(), m_table, m_num_funcs * sizeof(void *));
	std::memcpy(m_hook_table.get(), m_table, m_num_funcs * sizeof(void *));
}

vtable_hook::~vtable_hook() {
	(void)disable();
}

void vtable_hook::hook(std::size_t index, void *func) {
	RML_DEBUG("Hooking vtable index {} with function 0x{:X}", index, reinterpret_cast<uintptr_t>(func));
	m_hook_table[index] = func;

	if (!m_enabled) return;

	auto guard = rml::utils::MemoryProtectionGuard::create(&m_table[index], sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (!guard) {
		RML_ERROR("Failed to change memory protection while hooking vtable index {}: {}", index, guard.error().message());
		return;
	}

	m_table[index] = func;
}

void vtable_hook::unhook(std::size_t index) {
	m_hook_table[index] = m_backup_table[index];
}

std::expected<void, rml::HookError> vtable_hook::enable() {
	if (m_enabled) {
		return {};
	}

	auto guard = rml::utils::MemoryProtectionGuard::create(m_table, m_num_funcs * sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (!guard) {
		const auto error = rml::HookError::from_system_error("VtableHook::enable", reinterpret_cast<std::uintptr_t>(m_table), guard.error());
		RML_ERROR("Failed to enable vtable hook: {}", error.describe());
		return std::unexpected(error);
	}

	std::memcpy(m_table, m_hook_table.get(), m_num_funcs * sizeof(void *));
	m_enabled = true;
	return {};
}

std::expected<void, rml::HookError> vtable_hook::disable() {
	if (!m_enabled) {
		return {};
	}

	auto guard = rml::utils::MemoryProtectionGuard::create(m_table, m_num_funcs * sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (!guard) {
		const auto error = rml::HookError::from_system_error("VtableHook::disable", reinterpret_cast<std::uintptr_t>(m_table), guard.error());
		RML_WARN("Failed to disable vtable hook: {}", error.describe());
		return std::unexpected(error);
	}

	std::memcpy(m_table, m_backup_table.get(), m_num_funcs * sizeof(void *));
	m_enabled = false;
	return {};
}

bool vtable_hook::is_enabled() const {
	return m_enabled;
}
