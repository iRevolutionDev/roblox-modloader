#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>

extern "C"
{
	RML_EXPORT std::uint64_t rml_reflection_invoke(void* instance_ptr, const char* function_name, std::uint64_t arg0, std::uint64_t arg1, std::uint64_t arg2, std::uint64_t arg3, std::uint32_t arg_count);

	RML_EXPORT std::uint64_t rml_reflection_get_property(void* instance_ptr, const char* property_name);

	RML_EXPORT std::uint64_t rml_reflection_set_property(void* instance_ptr, const char* property_name, std::uint64_t value_ptr_or_value);

	RML_EXPORT std::uint64_t rml_instance_get_class_descriptor(void* instance_ptr);
}
