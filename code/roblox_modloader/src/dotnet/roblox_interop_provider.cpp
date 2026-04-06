
#include "roblox_interop_provider.hpp"

#include <RobloxModLoader/roblox/instance.hpp>
#include <RobloxModLoader/roblox/reflection/function_descriptor.hpp>
#include <RobloxModLoader/roblox/reflection/object.hpp>
#include <RobloxModLoader/roblox/reflection/property_descriptor.hpp>

namespace rml::dotnet
{
	void RobloxInteropProvider::populate(InteropTable& table)
	{
		table.version = RML_INTEROP_VERSION;
		table.size    = sizeof(InteropTable);

		table.reflection_invoke = [](const uintptr_t instance_ptr, const char* function_name, const uint64_t arg0, const uint64_t arg1, const uint64_t arg2, const uint64_t arg3, uint32_t arg_count) -> uint64_t {
			auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance)
				return 0;

			auto* method = instance->get_descriptor().find_function_descriptor(function_name);
			if (!method)
				return 0;

			switch (arg_count)
			{
			case 0: return method->invoke(instance);
			case 1: return method->invoke(instance, arg0);
			case 2: return method->invoke(instance, arg0, arg1);
			case 3: return method->invoke(instance, arg0, arg1, arg2);
			case 4: return method->invoke(instance, arg0, arg1, arg2, arg3);
			default: return 0;
			}
		};

		table.reflection_get_property = [](const uintptr_t instance_ptr, const char* property_name) -> uint64_t {
			auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance)
				return 0;

			const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
			if (!property_descriptor)
				return 0;

			// if (property_descriptor->has_string_value())
			// 	return 0;

			const auto property = RBX::Property(*property_descriptor, instance);
			return property.get<uint64_t>();
		};

		table.reflection_set_property = [](const uintptr_t instance_ptr, const char* property_name, const uint64_t value) -> uint64_t {
			auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance)
				return 0;

			const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
			if (!property_descriptor)
				return 0;

			// Same ABI hazard as get: don't call set<uint64_t>() on string properties.
			// if (property_descriptor->has_string_value())
			// 	return 0;

			auto property = RBX::Property(*property_descriptor, instance);

			property.set(value);
			return 1;
		};

		// v2: dedicated string property accessors
		// table.reflection_get_string_property = [](const uintptr_t instance_ptr, const char* property_name) -> const char* {
		// 	auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
		// 	if (!instance)
		// 		return nullptr;
		//
		// 	const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
		// 	if (!property_descriptor || !property_descriptor->has_string_value())
		// 		return nullptr;
		//
		// 	const std::string str = property_descriptor->get_string_value(instance);
		//
		// 	// Allocate with malloc so the .NET caller can free it via Marshal.FreeHGlobal
		// 	auto* buf = static_cast<char*>(std::malloc(str.size() + 1));
		// 	if (!buf)
		// 		return nullptr;
		//
		// 	std::memcpy(buf, str.c_str(), str.size() + 1);
		// 	return buf;
		// };
		//
		// table.reflection_set_string_property = [](const uintptr_t instance_ptr, const char* property_name, const char* value) -> bool {
		// 	auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
		// 	if (!instance || !value)
		// 		return false;
		//
		// 	const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
		// 	if (!property_descriptor || !property_descriptor->has_string_value())
		// 		return false;
		//
		// 	return property_descriptor->set_string_value(instance, std::string(value));
		// };
		//
		// table.instance_get_class_descriptor = [](const uintptr_t instance_ptr) -> uintptr_t {
		// 	if (const auto* instance = reinterpret_cast<const RBX::Instance*>(instance_ptr); !instance)
		// 		return 0;
		// 	return 0;
		// };
	}
} // namespace rml::dotnet
