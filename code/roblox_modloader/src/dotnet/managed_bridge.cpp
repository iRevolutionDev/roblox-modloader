#include "RobloxModLoader/dotnet/managed_bridge.hpp"

#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"
#include "RobloxModLoader/roblox/reflection/object.hpp"
#include "RobloxModLoader/roblox/reflection/property_descriptor.hpp"

#include <filesystem>
#include <string>

namespace
{
#if defined(_WIN32)
	using invoke0_t = std::uint64_t(__fastcall*)(void*);
	using invoke1_t = std::uint64_t(__fastcall*)(void*, std::uint64_t);
	using invoke2_t = std::uint64_t(__fastcall*)(void*, std::uint64_t, std::uint64_t);
	using invoke3_t = std::uint64_t(__fastcall*)(void*, std::uint64_t, std::uint64_t, std::uint64_t);
	using invoke4_t = std::uint64_t(__fastcall*)(void*, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t);

	using get_property_t = std::uint64_t(__fastcall*)(void*);
	using set_property_t = std::uint64_t(__fastcall*)(void*, std::uint64_t);
#else
	using invoke0_t = std::uint64_t (*)(void*);
	using invoke1_t = std::uint64_t (*)(void*, std::uint64_t);
	using invoke2_t = std::uint64_t (*)(void*, std::uint64_t, std::uint64_t);
	using invoke3_t = std::uint64_t (*)(void*, std::uint64_t, std::uint64_t, std::uint64_t);
	using invoke4_t = std::uint64_t (*)(void*, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t);

	using get_property_t = std::uint64_t (*)(void*);
	using set_property_t = std::uint64_t (*)(void*, std::uint64_t);
#endif

	ClassDescriptor* get_class_descriptor_from_instance(void* instance_ptr)
	{
		if (!instance_ptr)
		{
			return nullptr;
		}

		const auto* instance = static_cast<Instance*>(instance_ptr);
		return instance->class_descriptor;
	}
}

extern "C"
{
	std::uint64_t rml_reflection_invoke(void* instance_ptr, const char* function_name, const std::uint64_t arg0, const std::uint64_t arg1, const std::uint64_t arg2, const std::uint64_t arg3, const std::uint32_t arg_count)
	{
		auto* class_descriptor = get_class_descriptor_from_instance(instance_ptr);
		if (!class_descriptor || !function_name)
		{
			return 0;
		}

		auto* function_descriptor = class_descriptor->find_function_descriptor(function_name);
		if (!function_descriptor)
		{
			return 0;
		}

		const auto function_ptr = function_descriptor->get_bound_function<std::uintptr_t>();
		if (!function_ptr)
		{
			return 0;
		}

		switch (arg_count)
		{
		case 0: return reinterpret_cast<invoke0_t>(function_ptr)(instance_ptr);
		case 1: return reinterpret_cast<invoke1_t>(function_ptr)(instance_ptr, arg0);
		case 2: return reinterpret_cast<invoke2_t>(function_ptr)(instance_ptr, arg0, arg1);
		case 3: return reinterpret_cast<invoke3_t>(function_ptr)(instance_ptr, arg0, arg1, arg2);
		default: return reinterpret_cast<invoke4_t>(function_ptr)(instance_ptr, arg0, arg1, arg2, arg3);
		}
	}

	std::uint64_t rml_reflection_get_property(void* instance_ptr, const char* property_name)
	{
		auto* class_descriptor = get_class_descriptor_from_instance(instance_ptr);
		if (!class_descriptor || !property_name)
		{
			return 0;
		}

		auto* property_descriptor = class_descriptor->find_property_descriptor(property_name);
		if (!property_descriptor)
		{
			return 0;
		}

		const auto getter = property_descriptor->getter<std::uintptr_t>();
		if (!getter)
		{
			return 0;
		}

		return reinterpret_cast<get_property_t>(getter)(instance_ptr);
	}

	std::uint64_t rml_reflection_set_property(void* instance_ptr, const char* property_name, std::uint64_t value_ptr_or_value)
	{
		auto* class_descriptor = get_class_descriptor_from_instance(instance_ptr);
		if (!class_descriptor || !property_name)
		{
			return 0;
		}

		auto* property_descriptor = class_descriptor->find_property_descriptor(property_name);
		if (!property_descriptor)
		{
			return 0;
		}

		const auto setter = property_descriptor->setter<std::uintptr_t>();
		if (!setter)
		{
			return 0;
		}

		return reinterpret_cast<set_property_t>(setter)(instance_ptr, value_ptr_or_value);
	}

	std::uint64_t rml_instance_get_class_descriptor(void* instance_ptr)
	{
		return reinterpret_cast<std::uint64_t>(get_class_descriptor_from_instance(instance_ptr));
	}
}
