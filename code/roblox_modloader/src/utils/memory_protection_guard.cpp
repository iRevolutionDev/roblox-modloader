#include "memory_protection_guard.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace rml::utils
{
	MemoryProtectionGuard::MemoryProtectionGuard(void* address, const std::size_t size, const unsigned long previous_protection) :
	    m_address(address),
	    m_size(size),
	    m_previous_protection(previous_protection),
	    m_active(true)
	{
	}

	std::expected<MemoryProtectionGuard, std::error_code> MemoryProtectionGuard::create(void* address, const std::size_t size, const MemoryProtection protection)
	{
#if defined(RML_WINDOWS)
		DWORD new_protect = PAGE_READWRITE;
		switch (protection)
		{
			case MemoryProtection::ReadWrite:
				new_protect = PAGE_READWRITE;
				break;
			case MemoryProtection::ExecuteReadWrite:
				new_protect = PAGE_EXECUTE_READWRITE;
				break;
		}

		DWORD old_protect = 0;
		if (!VirtualProtect(address, size, new_protect, &old_protect))
			return std::unexpected(std::error_code(static_cast<int>(GetLastError()), std::system_category()));

		return MemoryProtectionGuard(address, size, old_protect);
#else
		(void)address;
		(void)size;
		(void)protection;
		return std::unexpected(std::make_error_code(std::errc::not_supported));
#endif
	}

	MemoryProtectionGuard::MemoryProtectionGuard(MemoryProtectionGuard&& other) noexcept :
	    m_address(other.m_address),
	    m_size(other.m_size),
	    m_previous_protection(other.m_previous_protection),
	    m_active(other.m_active)
	{
		other.m_active = false;
	}

	MemoryProtectionGuard& MemoryProtectionGuard::operator=(MemoryProtectionGuard&& other) noexcept
	{
		if (this != &other)
		{
			restore();
			m_address = other.m_address;
			m_size = other.m_size;
			m_previous_protection = other.m_previous_protection;
			m_active = other.m_active;
			other.m_active = false;
		}
		return *this;
	}

	MemoryProtectionGuard::~MemoryProtectionGuard()
	{
		restore();
	}

	void MemoryProtectionGuard::restore() noexcept
	{
		if (!m_active)
			return;

		m_active = false;

#if defined(RML_WINDOWS)
		DWORD temp = 0;
		VirtualProtect(m_address, m_size, static_cast<DWORD>(m_previous_protection), &temp);
#endif
	}
}
