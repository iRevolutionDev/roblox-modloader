
#include "roblox_interop_provider.hpp"

#include "dotnet_arguments.hpp"

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

		table.get_proc_address = [](const char*) -> void* {
			return nullptr;
		};

		table.reflection_invoke = [](const uintptr_t instance_ptr, const char* function_name, const InteropVariant* args, const uint32_t arg_count, InteropVariant* out_result) {
			if (out_result)
			{
				out_result->tag       = InteropValueTag::Null;
				out_result->as_uint64 = 0;
			}

			const auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance)
				return;

			if (const auto* descriptor = instance->get_descriptor().find_function_in_hierarchy(function_name); !descriptor)
				return;

			const DotNetArguments arguments{args, arg_count};

			// TODO: I need to call the function directly

			if (out_result)
				write_return_value(arguments.return_value, *out_result);
		};

		table.reflection_get_property = [](const uintptr_t instance_ptr, const char* property_name, InteropVariant* out_value) {
			if (out_value)
			{
				out_value->tag       = InteropValueTag::Null;
				out_value->as_uint64 = 0;
			}

			const auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance || !out_value)
				return;

			const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
			if (!property_descriptor)
				return;

			if (property_descriptor->type.name == "string")
			{
				const auto value     = property_descriptor->get_string_value(instance);
				out_value->tag       = InteropValueTag::String;
				out_value->as_string = strdup(value.c_str());
				return;
			}

			if (RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(*property_descriptor))
			{
				const auto* ref_desc = dynamic_cast<const RBX::Reflection::RefPropertyDescriptor*>(property_descriptor);
				out_value->tag       = InteropValueTag::Instance;
				out_value->as_instance = reinterpret_cast<uintptr_t>(ref_desc->get_ref_value(instance));
				return;
			}

			const auto property  = RBX::Property(*property_descriptor, instance);
			out_value->tag       = InteropValueTag::Int64;
			out_value->as_uint64 = property.get<uint64_t>();
		};

		table.reflection_set_property = [](const uintptr_t instance_ptr, const char* property_name, const InteropVariant* value) {
			if (!value)
				return;

			auto* instance = reinterpret_cast<RBX::Instance*>(instance_ptr);
			if (!instance)
				return;

			const auto property_descriptor = instance->get_descriptor().find_property_in_hierarchy(property_name);
			if (!property_descriptor)
				return;

			if (RBX::Reflection::RefPropertyDescriptor::is_ref_property_descriptor(*property_descriptor))
			{
				const auto* ref_desc = dynamic_cast<const RBX::Reflection::RefPropertyDescriptor*>(property_descriptor);
				auto* target =
				    value->tag == InteropValueTag::Instance ? reinterpret_cast<RBX::Reflection::DescribedBase*>(value->as_instance) : nullptr;
				ref_desc->set_ref_value(instance, target);
				return;
			}

			auto property = RBX::Property(*property_descriptor, instance);
			property.set(value->as_uint64);
		};

		table.free_string = [](const char* str) {
			free(const_cast<char*>(str));
		};
	}
} // namespace rml::dotnet
