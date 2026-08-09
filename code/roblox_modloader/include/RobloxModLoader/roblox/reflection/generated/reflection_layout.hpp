// Verified against Roblox Studio 0.733.0.7330989 (Windows x64).
// Keep build-sensitive reflection offsets in the loader SDK instead of mods.
#pragma once

#include <cstddef>

namespace rml::roblox::reflection::layout
{
	inline constexpr const char* studio_version = "0.733.0.7330989";

	inline constexpr std::size_t descriptor_name = 0x8;
	inline constexpr std::size_t member_owner = 0x30;
	inline constexpr std::size_t typed_property_get_set = 0x90;
	inline constexpr std::size_t typed_property_variant_accessor = 0x98;

	// TypedPropertyDescriptor<V>::GetSet has a deleting destructor followed by
	// is_read_only, is_write_only, get, set, equal_values, and equals_value.
	inline constexpr std::size_t typed_get_set_vtable_entries = 7;
	inline constexpr std::size_t typed_get_set_get_vtable_slot = 3;

	// VariantAccessor has a deleting destructor, mutable/const get overloads,
	// set, data_size, and feature_check.
	inline constexpr std::size_t typed_variant_accessor_vtable_entries = 6;
	inline constexpr std::size_t typed_variant_accessor_get_vtable_slot = 1;
	inline constexpr std::size_t typed_variant_accessor_const_get_vtable_slot = 2;

	// Direct reflected int64 functions use an MSVC member-pointer pair here.
	inline constexpr std::size_t function_invoke_target = 0x80;
	inline constexpr std::size_t function_bound_this_delta = 0x88;
	inline constexpr std::size_t function_descriptor_size = 0x90;
}
