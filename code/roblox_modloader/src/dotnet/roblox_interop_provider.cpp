
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

			if (property_descriptor->type.name == "string")
			{
				// IDK WHY STD::STRING DOESN'T WORKS ON ROBLOX BUT IT DOESN'T SO WE HAVE TO DO THIS BS
				const auto value = property_descriptor->get_string_value(instance);
				return reinterpret_cast<uint64_t>(strdup(value.c_str()));
			}

			if (RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(*property_descriptor))
			{
				const auto* ref_desc = static_cast<const RBX::Reflection::RefPropertyDescriptor*>(property_descriptor);
				return reinterpret_cast<uint64_t>(ref_desc->get_ref_value(instance));
			}

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

			if (RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(*property_descriptor))
			{
				const auto* ref_desc = static_cast<const RBX::Reflection::RefPropertyDescriptor*>(property_descriptor);
				ref_desc->set_ref_value(instance, reinterpret_cast<RBX::Reflection::DescribedBase*>(value));
				return 1;
			}

			auto property = RBX::Property(*property_descriptor, instance);

			property.set(value);
			return 1;
		};

		table.free_string = [](const char* str) {
			free(const_cast<char*>(str));
		};
	}
} // namespace rml::dotnet
