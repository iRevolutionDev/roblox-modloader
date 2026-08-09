#pragma once

#include <RobloxModLoader/internal/roblox_pointers.hpp>

#include <cstddef>
#include <cstring>
#include <optional>
#include <string_view>

namespace idspoofer::detail
{
	[[nodiscard]] bool initialize_engine_api() noexcept;
	[[nodiscard]] const RobloxPointers* engine_pointers() noexcept;

	[[nodiscard]] bool memory_has_access(const void* pointer, std::size_t length, bool execute = false) noexcept;

	template<typename T>
	[[nodiscard]] std::optional<T> read_memory(const void* address) noexcept
	{
		if (!memory_has_access(address, sizeof(T)))
			return std::nullopt;

		T value{};
		std::memcpy(&value, address, sizeof(value));
		return value;
	}

	[[nodiscard]] std::optional<std::string_view> rtti_name(const void* object) noexcept;
	[[nodiscard]] bool has_rtti(const void* object, std::string_view fragment) noexcept;
	[[nodiscard]] bool is_engine_object(const void* object) noexcept;

	[[nodiscard]] void* lookup_member(void* owner, const char* name) noexcept;
	[[nodiscard]] void* discover_descriptor(
	    const char* name, std::string_view rtti_fragment, std::size_t descriptor_size) noexcept;

	[[nodiscard]] bool atomic_replace_pointer(
	    void* address, void* expected, void* replacement) noexcept;
}
