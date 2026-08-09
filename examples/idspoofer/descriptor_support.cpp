#include "descriptor_support.hpp"

#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/roblox/reflection/generated/reflection_layout.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

RML_LOG_SCOPE("IdSpoofer")

namespace idspoofer::detail
{
	namespace
	{
		const RobloxPointers* s_pointers{};

		struct HashTableHeader
		{
			std::uint32_t size;
			std::uint32_t reserved;
			const std::uint32_t* control;
			const void* entries;
			std::uint32_t mask;
			std::uint32_t shift;
		};
		static_assert(sizeof(HashTableHeader) == 0x20);

#if defined(_WIN32)
		struct CompleteObjectLocator64
		{
			std::uint32_t signature;
			std::uint32_t offset;
			std::uint32_t constructor_displacement;
			std::int32_t type_descriptor_rva;
			std::int32_t class_descriptor_rva;
			std::int32_t self_rva;
		};
		static_assert(sizeof(CompleteObjectLocator64) == 0x18);
#endif

		[[nodiscard]] bool validate_member_table(const void* owner) noexcept
		{
			if (!s_pointers)
				return false;

			const auto* table = static_cast<const std::byte*>(owner) + s_pointers->member_table_offset;
			const auto header = read_memory<HashTableHeader>(table);
			if (!header || header->size == 0 || header->shift < 4 || header->shift > 16)
				return false;

			const std::uint32_t control_capacity = 1u << header->shift;
			const std::uint32_t entry_capacity = header->mask + 1;
			return entry_capacity != 0 && (entry_capacity & (entry_capacity - 1)) == 0
			    && entry_capacity <= control_capacity && control_capacity <= entry_capacity * 2
			    && header->size <= entry_capacity
			    && memory_has_access(header->control,
			        static_cast<std::size_t>(control_capacity) * sizeof(std::uint32_t))
			    && memory_has_access(header->entries,
			        static_cast<std::size_t>(entry_capacity) * sizeof(void*) * 2);
		}
	}

	bool initialize_engine_api() noexcept
	{
		if (s_pointers)
			return true;

		try
		{
			s_pointers = get_roblox_pointers();
		}
		catch (const std::exception& exception)
		{
			RML_ERROR("[idspoofer] could not obtain loader reflection pointers: {}", exception.what());
			return false;
		}
		catch (...)
		{
			RML_ERROR("[idspoofer] could not obtain loader reflection pointers");
			return false;
		}

		if (!s_pointers || !s_pointers->get_string_atom || !s_pointers->descriptor_lookup)
		{
			RML_ERROR("[idspoofer] required reflection functions are unresolved");
			s_pointers = nullptr;
			return false;
		}

		const std::uint64_t offset = s_pointers->member_table_offset;
		if (offset < 0x40 || offset > 0x1000 || (offset % alignof(void*)) != 0)
		{
			RML_ERROR("[idspoofer] invalid ClassDescriptor member-table offset: {:#x}", offset);
			s_pointers = nullptr;
			return false;
		}

		RML_INFO("[idspoofer] reflection API ready (ClassDescriptor member table +{:#x})", offset);
		return true;
	}

	const RobloxPointers* engine_pointers() noexcept
	{
		return s_pointers;
	}

	bool memory_has_access(const void* pointer, const std::size_t length, const bool execute) noexcept
	{
#if defined(_WIN32)
		if (!pointer || length == 0)
			return false;

		const auto start = reinterpret_cast<std::uintptr_t>(pointer);
		if (start < 0x10000 || start > 0x7FFFFFFFFFFFULL || length > 0x100000 || start + length < start)
			return false;

		MEMORY_BASIC_INFORMATION information{};
		if (VirtualQuery(pointer, &information, sizeof(information)) != sizeof(information)
		    || information.State != MEM_COMMIT
		    || (information.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0)
			return false;

		const auto region_end = reinterpret_cast<std::uintptr_t>(information.BaseAddress) + information.RegionSize;
		if (start + length > region_end)
			return false;
		if (!execute)
			return true;

		const DWORD protection = information.Protect & 0xff;
		return protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ
		    || protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
#else
		(void)length;
		(void)execute;
		return pointer != nullptr;
#endif
	}

	bool is_engine_object(const void* object) noexcept
	{
		const auto vtable = read_memory<void* const*>(object);
		if (!vtable || !memory_has_access(*vtable, sizeof(void*)))
			return false;

		const auto first = read_memory<void*>(*vtable);
		return first && memory_has_access(*first, 1, true);
	}

	std::optional<std::string_view> rtti_name(const void* object) noexcept
	{
#if defined(_WIN32)
		if (!is_engine_object(object))
			return std::nullopt;

		const auto module = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
		const auto vtable = read_memory<const void* const*>(object);
		if (module == 0 || !vtable || !memory_has_access(*vtable - 1, sizeof(void*)))
			return std::nullopt;

		const auto locator = read_memory<const CompleteObjectLocator64*>((*vtable) - 1);
		if (!locator || !memory_has_access(*locator, sizeof(CompleteObjectLocator64))
		    || (*locator)->signature != 1
		    || module + static_cast<std::uint32_t>((*locator)->self_rva)
		        != reinterpret_cast<std::uintptr_t>(*locator))
			return std::nullopt;

		const auto* name = reinterpret_cast<const char*>(
		    module + static_cast<std::uint32_t>((*locator)->type_descriptor_rva) + sizeof(void*) * 2);
		for (std::size_t length = 0; length != 1024; ++length)
		{
			if (!memory_has_access(name + length, 1))
				return std::nullopt;
			if (name[length] == '\0')
				return std::string_view{name, length};
		}
#else
		(void)object;
#endif
		return std::nullopt;
	}

	bool has_rtti(const void* object, const std::string_view fragment) noexcept
	{
		const auto name = rtti_name(object);
		return name && name->find(fragment) != std::string_view::npos;
	}

	void* lookup_member(void* owner, const char* name) noexcept
	{
		if (!s_pointers || !owner || !name
		    || !has_rtti(owner, ".?AVClassDescriptor@Reflection@RBX@@")
		    || !validate_member_table(owner))
			return nullptr;

		std::uintptr_t atom = s_pointers->get_string_atom(name);
		if (atom == 0)
			return nullptr;

		auto* result = s_pointers->descriptor_lookup(
		    reinterpret_cast<std::uintptr_t>(owner) + s_pointers->member_table_offset, &atom);
		return result && *result ? reinterpret_cast<void*>(*result) : nullptr;
	}

	void* discover_descriptor(const char* name, const std::string_view rtti_fragment,
	    const std::size_t descriptor_size) noexcept
	{
#if !defined(_WIN32)
		(void)name;
		(void)rtti_fragment;
		(void)descriptor_size;
		return nullptr;
#else
		using namespace rml::roblox::reflection::layout;
		if (!s_pointers || !name || descriptor_size < member_owner + sizeof(void*))
			return nullptr;

		const auto scan_started = std::chrono::steady_clock::now();
		std::size_t name_matches = 0;
		std::size_t rtti_matches = 0;
		std::size_t owner_matches = 0;
		const std::uintptr_t atom = s_pointers->get_string_atom(name);
		const auto module = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
		if (atom == 0 || module == 0
		    || !memory_has_access(reinterpret_cast<void*>(module), sizeof(IMAGE_DOS_HEADER)))
			return nullptr;

		const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
			return nullptr;

		const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
		    module + static_cast<std::uintptr_t>(dos->e_lfanew));
		if (!memory_has_access(nt, sizeof(*nt)) || nt->Signature != IMAGE_NT_SIGNATURE)
			return nullptr;

		const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);
		if (!memory_has_access(sections,
		        static_cast<std::size_t>(nt->FileHeader.NumberOfSections) * sizeof(IMAGE_SECTION_HEADER)))
			return nullptr;

		for (std::uint16_t section_index = 0;
		     section_index != nt->FileHeader.NumberOfSections; ++section_index)
		{
			const IMAGE_SECTION_HEADER& section = sections[section_index];
			if ((section.Characteristics & IMAGE_SCN_MEM_WRITE) == 0
			    || section.Misc.VirtualSize < descriptor_size)
				continue;

			const std::uintptr_t begin = module + section.VirtualAddress;
			const std::uintptr_t end = begin + section.Misc.VirtualSize;
			std::uintptr_t cursor = begin;
			while (cursor < end)
			{
				MEMORY_BASIC_INFORMATION information{};
				if (VirtualQuery(reinterpret_cast<void*>(cursor), &information, sizeof(information))
				    != sizeof(information))
					break;

				const auto region_begin = std::max(cursor,
				    reinterpret_cast<std::uintptr_t>(information.BaseAddress));
				const auto region_end = std::min(end,
				    reinterpret_cast<std::uintptr_t>(information.BaseAddress) + information.RegionSize);
				if (region_end <= region_begin)
					break;

				if (information.State == MEM_COMMIT
				    && (information.Protect & (PAGE_GUARD | PAGE_NOACCESS)) == 0)
				{
					std::uintptr_t candidate =
					    (region_begin + alignof(void*) - 1) & ~(alignof(void*) - 1);
					for (; candidate + descriptor_size <= region_end; candidate += sizeof(void*))
					{
						// The entire region was validated once above. Calling VirtualQuery
						// through read_memory for every 8-byte candidate turns this scan
						// into millions of system calls on Studio's large .data section.
						std::uintptr_t candidate_name{};
						std::memcpy(&candidate_name,
						    reinterpret_cast<const void*>(candidate + descriptor_name),
						    sizeof(candidate_name));
						if (candidate_name != atom)
							continue;

						++name_matches;
						auto* descriptor = reinterpret_cast<void*>(candidate);
						if (!has_rtti(descriptor, rtti_fragment))
							continue;

						++rtti_matches;
						const auto owner = read_memory<void*>(
						    static_cast<std::byte*>(descriptor) + member_owner);
						if (owner && lookup_member(*owner, name) == descriptor)
						{
							++owner_matches;
							const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
							    std::chrono::steady_clock::now() - scan_started);
							RML_INFO("[idspoofer] descriptor '{}' found at {:#x} in {} ms",
							    name, reinterpret_cast<std::uintptr_t>(descriptor), elapsed.count());
							return descriptor;
						}
					}
				}
				cursor = region_end;
			}
		}

		const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
		    std::chrono::steady_clock::now() - scan_started);
		RML_ERROR("[idspoofer] descriptor '{}' not found after {} ms: name={} RTTI={} owner-roundtrip={}",
		    name, elapsed.count(), name_matches, rtti_matches, owner_matches);
		return nullptr;
#endif
	}

	bool atomic_replace_pointer(void* address, void* expected, void* replacement) noexcept
	{
#if defined(_WIN32)
		if (!memory_has_access(address, sizeof(void*)))
			return false;

		DWORD old_protection = 0;
		if (VirtualProtect(address, sizeof(void*), PAGE_READWRITE, &old_protection) == 0)
			return false;

		void* previous = InterlockedCompareExchangePointer(
		    reinterpret_cast<void* volatile*>(address), replacement, expected);
		DWORD ignored = 0;
		(void)VirtualProtect(address, sizeof(void*), old_protection, &ignored);
		return previous == expected;
#else
		auto** slot = static_cast<void**>(address);
		if (!slot || *slot != expected)
			return false;
		*slot = replacement;
		return true;
#endif
	}
}
