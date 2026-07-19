#pragma once

#include "RobloxModLoader/platform/memory/memory_protection.hpp"

#include <cstddef>
#include <expected>
#include <system_error>

namespace rml::utils
{
	class MemoryProtectionGuard
	{
	public:
		[[nodiscard]] static std::expected<MemoryProtectionGuard, std::error_code> create(void* address, std::size_t size, MemoryProtection protection);

		MemoryProtectionGuard(const MemoryProtectionGuard&) = delete;
		MemoryProtectionGuard& operator=(const MemoryProtectionGuard&) = delete;

		MemoryProtectionGuard(MemoryProtectionGuard&& other) noexcept;
		MemoryProtectionGuard& operator=(MemoryProtectionGuard&& other) noexcept;

		~MemoryProtectionGuard();

	private:
		MemoryProtectionGuard(void* address, std::size_t size, unsigned long previous_protection);

		void restore() noexcept;

		void* m_address;
		std::size_t m_size;
		unsigned long m_previous_protection;
		bool m_active;
	};
}
