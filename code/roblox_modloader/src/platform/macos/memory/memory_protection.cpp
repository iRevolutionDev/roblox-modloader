#include "RobloxModLoader/platform/memory/memory_protection.hpp"
#include "utils/memory_protection_guard.hpp"

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <sys/mman.h>
#include <unistd.h>

namespace rml::platform
{
	static std::pair<void*, std::size_t> page_align(void* address, const std::size_t size)
	{
		const auto page_size = static_cast<std::uintptr_t>(::getpagesize());
		const auto start = reinterpret_cast<std::uintptr_t>(address);
		const auto aligned_start = start & ~(page_size - 1);
		const auto aligned_size = ((start + size) - aligned_start + page_size - 1) & ~(page_size - 1);

		return {reinterpret_cast<void*>(aligned_start), static_cast<std::size_t>(aligned_size)};
	}

	static std::expected<vm_prot_t, std::error_code> current_protection(void* address)
	{
		auto region_address = reinterpret_cast<mach_vm_address_t>(address);
		mach_vm_size_t region_size = 0;
		vm_region_basic_info_data_64_t info{};
		mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;
		mach_port_t object_name = MACH_PORT_NULL;

		const kern_return_t result = mach_vm_region(mach_task_self(), &region_address, &region_size,
		    VM_REGION_BASIC_INFO_64, reinterpret_cast<vm_region_info_t>(&info), &info_count, &object_name);

		if (result != KERN_SUCCESS)
			return std::unexpected(std::make_error_code(std::errc::bad_address));

		return info.protection;
	}

	std::expected<unsigned long, std::error_code> set_protection(void* address, const std::size_t size, const utils::MemoryProtection protection)
	{
		const auto previous = current_protection(address);
		if (!previous)
			return std::unexpected(previous.error());

		int new_protection = PROT_READ | PROT_WRITE;

		switch (protection)
		{
		case utils::MemoryProtection::ReadWrite:
			new_protection = PROT_READ | PROT_WRITE;
			break;
		case utils::MemoryProtection::ExecuteReadWrite:
			new_protection = PROT_READ | PROT_WRITE;
			break;
		}

		const auto [aligned_address, aligned_size] = page_align(address, size);

		if (mprotect(aligned_address, aligned_size, new_protection) != 0)
			return std::unexpected(std::error_code(errno, std::system_category()));

		return static_cast<unsigned long>(*previous);
	}

	void restore_protection(void* address, const std::size_t size, const unsigned long previous) noexcept
	{
		const auto [aligned_address, aligned_size] = page_align(address, size);

		::mprotect(aligned_address, aligned_size, static_cast<int>(previous));
	}
}
