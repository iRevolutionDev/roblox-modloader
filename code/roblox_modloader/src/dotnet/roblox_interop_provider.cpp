
#include "roblox_interop_provider.hpp"

#include "RobloxModLoader/logger/logger.hpp"
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
#include <cassert>
#include <string_view>
#include <utility>

RML_LOG_SCOPE("Interop");

namespace rml::dotnet
{
	void RobloxInteropProvider::verify_populated(const InteropTable& table)
	{
		const std::pair<const void*, std::string_view> members[]{
		    {reinterpret_cast<const void*>(table.reflection_invoke), "reflection_invoke"},
		    {reinterpret_cast<const void*>(table.reflection_invoke_async), "reflection_invoke_async"},
		    {reinterpret_cast<const void*>(table.reflection_get_property), "reflection_get_property"},
		    {reinterpret_cast<const void*>(table.reflection_set_property), "reflection_set_property"},
		    {reinterpret_cast<const void*>(table.reflection_event_connect), "reflection_event_connect"},
		    {reinterpret_cast<const void*>(table.reflection_event_disconnect), "reflection_event_disconnect"},
		    {reinterpret_cast<const void*>(table.object_create_by_name), "object_create_by_name"},
		    {reinterpret_cast<const void*>(table.managed_log), "managed_log"},
		    {reinterpret_cast<const void*>(table.free_string), "free_string"},
		    {reinterpret_cast<const void*>(table.free_native_ptr), "free_native_ptr"},
		    {reinterpret_cast<const void*>(table.mods_menu_add_action), "mods_menu_add_action"},
		    {reinterpret_cast<const void*>(table.mods_menu_add_submenu), "mods_menu_add_submenu"},
		    {reinterpret_cast<const void*>(table.mods_menu_add_separator), "mods_menu_add_separator"},
		    {reinterpret_cast<const void*>(table.mods_menu_add_checkable), "mods_menu_add_checkable"},
		    {reinterpret_cast<const void*>(table.mods_menu_set_item_icon), "mods_menu_set_item_icon"},
		    {reinterpret_cast<const void*>(table.mods_menu_remove), "mods_menu_remove"},
		    {reinterpret_cast<const void*>(table.reflection_event_fire), "reflection_event_fire"},
		    {reinterpret_cast<const void*>(table.reflection_event_disconnect_all), "reflection_event_disconnect_all"},
		    {reinterpret_cast<const void*>(table.reflection_event_slots), "reflection_event_slots"},
		    {reinterpret_cast<const void*>(table.event_slot_fire), "event_slot_fire"},
		    {reinterpret_cast<const void*>(table.event_slot_disconnect), "event_slot_disconnect"},
		    {reinterpret_cast<const void*>(table.event_slot_release), "event_slot_release"},
		};

		for (const auto& [pointer, name] : members)
		{
			if (!pointer)
				RML_ERROR("InteropTable::{} was never populated", name);

			assert(pointer && "InteropTable member left unpopulated after populate");
		}
	}

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

	RBX::Reflection::EventArguments build_event_fire_args(const RBX::Reflection::EventDescriptor* descriptor, const InteropVariant* args, const uint32_t arg_count)
	{
		RBX::Reflection::EventArguments event_args;
		const auto& signature = descriptor->get_signature();
		const auto sig_args = signature.arguments();

		if (sig_args.size() == 1 && sig_args[0].type && sig_args[0].type->type_id == RBX::Reflection::TypeId::Tuple)
		{
			RBX::Reflection::Variant tuple_variant;
			if (TypeMarshaler::build_tuple_variant(args, arg_count, sig_args[0].type, tuple_variant))
				event_args.push_back(std::move(tuple_variant));
			return event_args;
		}

		const DotNetArguments arguments{args, arg_count, &signature};
		event_args.reserve(arg_count);
		for (uint32_t i = 0; i < arg_count; ++i)
		{
			RBX::Reflection::Variant value;
			if (arguments.get_varint(static_cast<int>(i) + 1, value))
				event_args.push_back(std::move(value));
		}
		return event_args;
	}

	void release_fire_args(RBX::Reflection::EventArguments& event_args)
	{
		for (auto& value : event_args)
		{
			if (!value.is_void() && value.type().type_id == RBX::Reflection::TypeId::Tuple)
				std::destroy_at(static_cast<std::shared_ptr<const RBX::Reflection::Tuple>*>(value.storage()));
		}
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

				(void)TypeMarshaler::decode_property(property_descriptor, instance, *value);
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

		table.managed_log = [](const int32_t level, const char* utf8, const int32_t len) {
			if (!utf8 || len < 0)
				return;

			const std::string_view message{utf8, static_cast<size_t>(len)};

			switch (level)
			{
			case 0: rml_scoped_logger()->log(spdlog::level::trace, message); break;
			case 1: rml_scoped_logger()->log(spdlog::level::debug, message); break;
			case 2: rml_scoped_logger()->log(spdlog::level::info, message); break;
			case 3: rml_scoped_logger()->log(spdlog::level::warn, message); break;
			case 4: rml_scoped_logger()->log(spdlog::level::err, message); break;
			case 5: rml_scoped_logger()->log(spdlog::level::critical, message); break;
			default: rml_scoped_logger()->log(spdlog::level::info, message); break;
			}
		};

		table.free_string = [](const char* str) {
			free(const_cast<char*>(str));
		};

		table.free_native_ptr = [](const void* ptr) {
			free(const_cast<void*>(ptr));
		};

		table.mods_menu_add_action = [](const uintptr_t parent_id, const char* text, const ManagedEventCallback callback, void* state) -> uintptr_t {
			if (!text || !callback)
				return 0;

			auto* const integration = rml::qt::QtIntegration::instance();
			if (!integration)
				return 0;

			return integration->menu().add_action(parent_id, text, [callback, state] {
				callback(state, nullptr, 0);
			});
		};

		table.mods_menu_add_submenu = [](const uintptr_t parent_id, const char* text) -> uintptr_t {
			if (!text)
				return 0;

			auto* const integration = rml::qt::QtIntegration::instance();
			if (!integration)
				return 0;

			return integration->menu().add_submenu(parent_id, text);
		};

		table.mods_menu_add_separator = [](const uintptr_t parent_id) -> uintptr_t {
			auto* const integration = rml::qt::QtIntegration::instance();
			if (!integration)
				return 0;

			return integration->menu().add_separator(parent_id);
		};

		table.mods_menu_add_checkable = [](const uintptr_t parent_id, const char* text, const int initial, const ManagedEventCallback callback, void* state) -> uintptr_t {
			if (!text || !callback)
				return 0;

			auto* const integration = rml::qt::QtIntegration::instance();
			if (!integration)
				return 0;

			return integration->menu().add_checkable(parent_id, text, initial != 0, [callback, state](const bool value) {
				const InteropVariant arg = bool_value(value);
				callback(state, &arg, 1);
			});
		};

		table.mods_menu_set_item_icon = [](const uintptr_t id, const char* utf8_path) {
			if (!utf8_path)
				return;

			if (auto* const integration = rml::qt::QtIntegration::instance())
				integration->menu().set_item_icon(id, utf8_path);
		};

		table.mods_menu_remove = [](const uintptr_t id) {
			if (auto* const integration = rml::qt::QtIntegration::instance())
				integration->menu().remove(id);
		};

		table.reflection_event_fire = [](const uintptr_t instance_ptr, const char* event_name, const InteropVariant* args, const uint32_t arg_count) {
			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !event_name)
					return;

				const auto* descriptor = instance->get_descriptor().find_event(event_name);
				if (!descriptor)
					return;

				auto event_args = build_event_fire_args(descriptor, args, arg_count);
				descriptor->fire_event(instance, event_args);
				release_fire_args(event_args);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("event_fire('{}') failed: {}", event_name ? event_name : "?", e.what());
			}
			catch (...)
			{
				RML_ERROR("event_fire('{}') failed: unknown exception", event_name ? event_name : "?");
			}
		};

		table.reflection_event_disconnect_all = [](const uintptr_t instance_ptr, const char* event_name) {
			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !event_name)
					return;

				if (const auto* descriptor = instance->get_descriptor().find_event(event_name))
					descriptor->disconnect_all(instance);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("event_disconnect_all('{}') failed: {}", event_name ? event_name : "?", e.what());
			}
			catch (...)
			{
				RML_ERROR("event_disconnect_all('{}') failed: unknown exception", event_name ? event_name : "?");
			}
		};

		table.reflection_event_slots = [](const uintptr_t instance_ptr, const char* event_name, uint32_t* out_count) -> uintptr_t* {
			if (out_count)
				*out_count = 0;

			try
			{
				auto* instance = as_instance(instance_ptr);
				if (!instance || !event_name)
					return nullptr;

				const auto* descriptor = instance->get_descriptor().find_event(event_name);
				if (!descriptor)
					return nullptr;

				auto snapshot = descriptor->snapshot_connections(instance);
				if (snapshot.empty())
					return nullptr;

				auto* result = static_cast<uintptr_t*>(malloc(snapshot.size() * sizeof(uintptr_t)));
				if (!result)
					return nullptr;

				for (size_t i = 0; i < snapshot.size(); ++i)
					result[i] = reinterpret_cast<uintptr_t>(new RBX::Signals::Connection(std::move(snapshot[i])));

				if (out_count)
					*out_count = static_cast<uint32_t>(snapshot.size());
				return result;
			}
			catch (...)
			{
				return nullptr;
			}
		};

		table.event_slot_fire = [](const uintptr_t instance_ptr, const char* event_name, const uintptr_t slot_handle, const InteropVariant* args, const uint32_t arg_count) {
			try
			{
				const auto* connection = reinterpret_cast<RBX::Signals::Connection*>(slot_handle);
				if (!connection)
					return;

				const auto* slot = connection->raw_slot();
				if (!slot || !slot->source)
					return;

				auto* wrapper = static_cast<RBX::Reflection::GenericSlotWrapper*>(slot->wrapper_ptr);
				if (!wrapper)
					return;

				RBX::Reflection::EventArguments event_args;
				if (const auto* instance = as_instance(instance_ptr); instance && event_name)
				{
					if (const auto* descriptor = instance->get_descriptor().find_event(event_name))
						event_args = build_event_fire_args(descriptor, args, arg_count);
				}

				wrapper->deliver(event_args);
				release_fire_args(event_args);
			}
			catch (const std::exception& e)
			{
				RML_ERROR("event_slot_fire failed: {}", e.what());
			}
			catch (...)
			{
				RML_ERROR("event_slot_fire failed: unknown exception");
			}
		};

		table.event_slot_disconnect = [](const uintptr_t slot_handle) {
			if (const auto* connection = reinterpret_cast<RBX::Signals::Connection*>(slot_handle))
				connection->disconnect();
		};

		table.event_slot_release = [](const uintptr_t slot_handle) {
			delete reinterpret_cast<RBX::Signals::Connection*>(slot_handle);
		};

		verify_populated(table);
	}
} // namespace rml::dotnet
