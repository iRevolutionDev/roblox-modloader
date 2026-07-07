
#include "roblox_interop_provider.hpp"

#include "RobloxModLoader/roblox/data_model.hpp"
#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/util/memory.hpp"
#include "dotnet_arguments.hpp"
#include "dotnet_event_descriptor.hpp"
#include "dotnet_variant.hpp"
#include "dotnet_yield.hpp"
#include "type_marshaler.hpp"

#include <RobloxModLoader/qt/mods_menu.hpp>
#include <RobloxModLoader/qt/qt_integration.hpp>
#include <RobloxModLoader/roblox/instance.hpp>
#include <RobloxModLoader/roblox/reflection/object.hpp>
#include <RobloxModLoader/roblox/reflection/property_descriptor.hpp>

RML_LOG_SCOPE("Interop");

namespace rml::dotnet
{
	[[nodiscard]] RBX::Instance* as_instance(const uintptr_t handle)
	{
		if (!utils::memory::is_valid_pointer(handle))
			return nullptr;
		return reinterpret_cast<RBX::Instance*>(handle);
	}

	void invoke_reflection_function(RBX::Reflection::DescribedBase* instance, const RBX::Reflection::FunctionDescriptor& descriptor, const InteropVariant* args, const uint32_t arg_count, InteropVariant& out)
	{
		DotNetArguments arguments{args, arg_count};

		const auto function = RBX::Function(descriptor, instance);
		const auto ret = function.invoke(arguments);
		const auto type = descriptor.get_signature().first_result_type();

		TypeMarshaler::encode_return_value(type, ret, reinterpret_cast<uintptr_t>(&arguments.return_value), out);
	}

	void RobloxInteropProvider::populate(InteropTable& table)
	{
		table.reflection_invoke = [](const uintptr_t instance_ptr, const char* function_name, const InteropVariant* args, const uint32_t arg_count, InteropVariant* out_result) {
			if (out_result)
			{
				out_result->tag = InteropValueTag::Null;
				out_result->as_uint64 = 0;
			}

			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !function_name)
					return;

				const auto* descriptor = instance->get_descriptor().find_function(function_name);
				if (!descriptor)
					return;

				InteropVariant local_result{};
				invoke_reflection_function(instance, *descriptor, args, arg_count, out_result ? *out_result : local_result);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("invoke('{}') failed: {}", function_name ? function_name : "?", e.what());
			}
			catch (...)
			{
				RML_ERROR("invoke('{}') failed: unknown exception", function_name ? function_name : "?");
			}
		};

		table.reflection_invoke_async = [](const uintptr_t instance_ptr, const char* function_name, const InteropVariant* args, const uint32_t arg_count, ManagedYieldCallback callback, void* state) {
			if (!callback)
				return;

			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !function_name)
				{
					callback(state, nullptr, "invalid instance or function name");
					return;
				}

				auto& class_descriptor = instance->get_descriptor();

				if (const auto* yield_descriptor = class_descriptor.find_yield_function_descriptor(function_name))
				{
					DotNetArguments arguments{args, arg_count, &yield_descriptor->get_signature()};
					YieldInvocation::dispatch(*yield_descriptor, *instance, arguments, callback, state);
					return;
				}

				const auto* descriptor = class_descriptor.find_function(function_name);
				if (!descriptor)
				{
					callback(state, nullptr, "function not found");
					return;
				}

				InteropVariant result{};
				invoke_reflection_function(instance, *descriptor, args, arg_count, result);
				callback(state, &result, nullptr);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("invoke_async('{}') failed: {}", function_name ? function_name : "?", e.what());
				callback(state, nullptr, e.what());
			}
			catch (...)
			{
				RML_ERROR("invoke_async('{}') failed: unknown exception", function_name ? function_name : "?");
				callback(state, nullptr, "unknown exception");
			}
		};

		table.reflection_get_property = [](const uintptr_t instance_ptr, const char* property_name, InteropVariant* out_value) {
			if (!out_value)
				return;
			*out_value = null_value();

			try
			{
				const auto* instance = as_instance(instance_ptr);
				if (!instance || !property_name)
					return;

				const auto property_descriptor = instance->get_descriptor().find_property(property_name);
				if (!property_descriptor)
					return;

				*out_value = TypeMarshaler::encode_property(property_descriptor, instance);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("get_property('{}') failed: {}", property_name ? property_name : "?", e.what());
			}
			catch (...)
			{
				RML_ERROR("get_property('{}') failed: unknown exception", property_name ? property_name : "?");
			}
		};

		table.reflection_set_property = [](const uintptr_t instance_ptr, const char* property_name, const InteropVariant* value) {
			if (!value)
				return;

			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !property_name)
					return;

				const auto property_descriptor = instance->get_descriptor().find_property(property_name);
				if (!property_descriptor)
					return;

				(void) TypeMarshaler::decode_property(property_descriptor, instance, *value);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("set_property('{}') failed: {}", property_name ? property_name : "?", e.what());
			}
			catch (...)
			{
				RML_ERROR("set_property('{}') failed: unknown exception", property_name ? property_name : "?");
			}
		};

		table.reflection_event_connect = [](const uintptr_t instance_ptr, const char* event_name, const ManagedEventCallback callback, void* state) -> uintptr_t {
			try
			{
				if (!callback)
					return 0;

				auto* instance = as_instance(instance_ptr);
				if (!instance || !event_name)
					return 0;

				const auto* event_descriptor = instance->get_descriptor().find_event(event_name);
				if (!event_descriptor)
					return 0;

				const auto slot = std::make_shared<ManagedEventSlot>(callback, state);

				auto holder = std::make_unique<ManagedEventConnection>();
				holder->slot = slot;
				holder->connection = event_descriptor->connect(instance, slot);

				return reinterpret_cast<uintptr_t>(holder.release());
			}
			catch (const std::exception& e)
			{
				RML_ERROR("event_connect('{}') failed: {}", event_name ? event_name : "?", e.what());
				return 0;
			}
			catch (...)
			{
				RML_ERROR("event_connect('{}') failed: unknown exception", event_name ? event_name : "?");
				return 0;
			}
		};

		table.reflection_event_disconnect = [](const uintptr_t connection_handle) {
			const auto* holder = reinterpret_cast<ManagedEventConnection*>(connection_handle);
			if (!holder)
				return;

			try
			{
				holder->connection.disconnect();
			}
			catch (const std::exception& e)
			{
				RML_ERROR("event_disconnect failed: {}", e.what());
			}
			catch (...)
			{
				RML_ERROR("event_disconnect failed: unknown exception");
			}

			delete holder;
		};

		table.object_create_by_name = [](const char* class_name, const int creator_role) -> uintptr_t {
			if (!class_name)
				return 0;

			try
			{
				const auto atom = g_pointers->m_roblox_pointers.get_string_atom(class_name);

				uintptr_t out{};
				g_pointers->m_roblox_pointers.object_create_by_name(&out, 0, atom, creator_role);

				if (!out)
				{
					RML_ERROR("creator_create_by_name('{}') failed: null instance", class_name);
					return 0;
				}

				return out;
			}
			catch (const std::exception& e)
			{
				RML_ERROR("create_by_name('{}') failed: {}", class_name, e.what());
				return 0;
			}
			catch (...)
			{
				RML_ERROR("create_by_name('{}') failed: unknown exception", class_name);
				return 0;
			}
		};

		table.free_string = [](const char* str) {
			free(const_cast<char*>(str));
		};

		table.free_native_ptr = [](const void* ptr) {
			free(const_cast<void*>(ptr));
		};

		table.mods_menu_add_action = [](const char* text, const ManagedEventCallback callback, void* state) -> uintptr_t {
			if (!text || !callback)
				return 0;

			auto* const integration = rml::qt::QtIntegration::instance();
			if (!integration)
				return 0;

			return integration->menu().register_action(text, [callback, state] {
				callback(state, nullptr, 0);
			});
		};

		table.mods_menu_remove_action = [](const uintptr_t action_id) {
			if (auto* const integration = rml::qt::QtIntegration::instance())
				integration->menu().remove_action(action_id);
		};
	}
} // namespace rml::dotnet
