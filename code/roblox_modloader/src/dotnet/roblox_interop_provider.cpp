
#include "roblox_interop_provider.hpp"

#include <RobloxModLoader/roblox/data_model.hpp>
#include <RobloxModLoader/roblox/instance.hpp>

namespace rml::dotnet
{
	void RobloxInteropProvider::populate(InteropTable& table)
	{
		table.version = RML_INTEROP_VERSION;
		table.size    = sizeof(InteropTable);

		table.reflection_invoke = [](const uintptr_t instance_ptr, const char* function_name, const uint64_t arg0, const uint64_t arg1, const uint64_t arg2, const uint64_t arg3, uint32_t arg_count) -> uint64_t {
			auto* instance = reinterpret_cast<Instance*>(instance_ptr);
			if (!instance)
				return 0;

			auto* class_descriptor = instance->class_descriptor;
			if (!class_descriptor)
				return 0;

			const auto method = class_descriptor->find_function_descriptor(function_name);
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
			auto* instance = reinterpret_cast<Instance*>(instance_ptr);
			if (!instance)
				return 0;

			auto* class_descriptor = instance->class_descriptor;
			if (!class_descriptor)
				return 0;

			const auto property = class_descriptor->find_property_descriptor(property_name);
			if (!property)
				return 0;

			{
				using getter_fn_t = uint64_t(__fastcall*)(Instance*);
				auto fn = property->template getter<getter_fn_t>();
				return fn ? fn(instance) : 0;
			}
		};

		table.reflection_set_property = [](const uintptr_t instance_ptr, const char* property_name, uint64_t value) -> uint64_t {
			auto* instance = reinterpret_cast<Instance*>(instance_ptr);
			if (!instance)
				return 0;

			auto* class_descriptor = instance->class_descriptor;
			if (!class_descriptor)
				return 0;

			const auto property = class_descriptor->find_property_descriptor(property_name);
			if (!property)
				return 0;

			{
				using setter_fn_t = uint64_t(__thiscall*)(Instance*, uint64_t);
				auto fn = property->template setter<setter_fn_t>();
				return fn ? fn(instance, value) : 0;
			}
		};
	}
} // namespace rml::dotnet