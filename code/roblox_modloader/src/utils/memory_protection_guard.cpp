#include "memory_protection_guard.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/platform/memory/memory_protection.hpp"

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
		const auto previous = platform::set_protection(address, size, protection);

		if (!previous)
			return std::unexpected(previous.error());

		return MemoryProtectionGuard(address, size, *previous);
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

		platform::restore_protection(m_address, m_size, m_previous_protection);
	}
}
