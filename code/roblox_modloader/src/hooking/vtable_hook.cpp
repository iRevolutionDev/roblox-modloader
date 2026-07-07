#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/vtable_hook.hpp"

#include "utils/memory_protection_guard.hpp"

vtable_hook::vtable_hook(void **vft, std::size_t num_funcs) : m_num_funcs(num_funcs),
                                                              m_table(vft),
                                                              m_backup_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_hook_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_enabled(false) {
	std::memcpy(m_backup_table.get(), m_table, m_num_funcs * sizeof(void *));
	std::memcpy(m_hook_table.get(), m_table, m_num_funcs * sizeof(void *));

	enable();
}

vtable_hook::~vtable_hook() {
	disable();
}

void vtable_hook::hook(std::size_t index, void *func) {
	LOG_DEBUG("Hooking vtable index {} with function 0x{:X}", index, reinterpret_cast<uintptr_t>(func));
	m_hook_table[index] = func;

	if (!m_enabled) return;

	auto guard = rml::utils::MemoryProtectionGuard::create(&m_table[index], sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (!guard) return;

	m_table[index] = func;
}

void vtable_hook::unhook(std::size_t index) {
	m_hook_table[index] = m_backup_table[index];
}

void vtable_hook::enable() {
	if (m_enabled) {
		return;
	}

	auto guard = rml::utils::MemoryProtectionGuard::create(m_table, m_num_funcs * sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (!guard) {
		return;
	}

	std::memcpy(m_table, m_hook_table.get(), m_num_funcs * sizeof(void *));
	m_enabled = true;
}

void vtable_hook::disable() {
	if (!m_enabled) {
		return;
	}

	auto guard = rml::utils::MemoryProtectionGuard::create(m_table, m_num_funcs * sizeof(void *), rml::utils::MemoryProtection::ReadWrite);
	if (guard) {
		std::memcpy(m_table, m_backup_table.get(), m_num_funcs * sizeof(void *));
	}

	m_enabled = false;
}
