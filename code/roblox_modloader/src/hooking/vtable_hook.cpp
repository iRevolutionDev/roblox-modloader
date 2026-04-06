#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/vtable_hook.hpp"

vtable_hook::vtable_hook(void **vft, std::size_t num_funcs) : m_num_funcs(num_funcs),
                                                              m_table(vft),
                                                              m_backup_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_hook_table(std::make_unique<void *[]>(m_num_funcs)),
                                                              m_old_protect(0) {
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

	if (m_old_protect == 0) return;

	DWORD temp;
	if (!VirtualProtect(&m_table[index], sizeof(void *), PAGE_READWRITE, &temp)) {
		return;
	}

	m_table[index] = func;
	VirtualProtect(&m_table[index], sizeof(void *), temp, &temp);
}

void vtable_hook::unhook(std::size_t index) {
	m_hook_table[index] = m_backup_table[index];
}

void vtable_hook::enable() {
	if (m_old_protect != 0) {
		return;
	}

	DWORD temp;
	if (!VirtualProtect(m_table, m_num_funcs * sizeof(void *), PAGE_READWRITE, &m_old_protect)) {
		return;
	}
	std::memcpy(m_table, m_hook_table.get(), m_num_funcs * sizeof(void *));
	VirtualProtect(m_table, m_num_funcs * sizeof(void *), m_old_protect, &temp);
}

void vtable_hook::disable() {
	if (m_old_protect == 0) {
		return;
	}

	DWORD temp;
	VirtualProtect(m_table, m_num_funcs * sizeof(void *), PAGE_READWRITE, &temp);
	std::memcpy(m_table, m_backup_table.get(), m_num_funcs * sizeof(void *));
	VirtualProtect(m_table, m_num_funcs * sizeof(void *), m_old_protect, &temp);
	m_old_protect = 0;
}
