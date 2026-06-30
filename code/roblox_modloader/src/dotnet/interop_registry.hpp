#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
	#define RML_INTEROP_CALL __cdecl
#else
	#define RML_INTEROP_CALL
#endif

namespace rml::dotnet
{
	enum class InteropValueTag : uint8_t
	{
		Null = 0,
		Bool = 1,
		Int64 = 2,
		Double = 3,
		Float = 4,
		String = 5,
		Instance = 6,
		InstanceArray = 7,
		Blittable = 8,
	};

	struct alignas(8) InteropVariant
	{
		InteropValueTag tag;
		union {
			bool as_bool;
			int64_t as_int64;
			uint64_t as_uint64;
			double as_double;
			float as_float;
			uintptr_t as_instance;
			char* as_string;
		};
	};

	static_assert(sizeof(InteropVariant) == 16, "InteropVariant must be 16 bytes (managed mirror is Size = 16).");
	static_assert(alignof(InteropVariant) == 8, "InteropVariant must be 8-byte aligned.");
	static_assert(offsetof(InteropVariant, tag) == 0, "InteropVariant.tag must be at offset 0.");
	static_assert(offsetof(InteropVariant, as_uint64) == 8, "InteropVariant union must start at offset 8.");

	using ManagedEventCallback = void(RML_INTEROP_CALL*)(void* state, const InteropVariant* args, uint32_t arg_count);

	struct alignas(8) InteropTable
	{
		uint32_t version;
		uint32_t size;

		void*(RML_INTEROP_CALL* get_proc_address)(const char* name);

		void(RML_INTEROP_CALL* reflection_invoke)(uintptr_t instance, const char* function_name, const InteropVariant* args, uint32_t arg_count, InteropVariant* out_result);
		void(RML_INTEROP_CALL* reflection_get_property)(uintptr_t instance, const char* property_name, InteropVariant* out_value);
		void(RML_INTEROP_CALL* reflection_set_property)(uintptr_t instance, const char* property_name, const InteropVariant* value);

		uintptr_t(RML_INTEROP_CALL* reflection_event_connect)(uintptr_t instance, const char* event_name, ManagedEventCallback callback, void* state);
		void(RML_INTEROP_CALL* reflection_event_disconnect)(uintptr_t connection_handle);

		uintptr_t(RML_INTEROP_CALL* instance_get_class_descriptor)(uintptr_t instance);

		uintptr_t(RML_INTEROP_CALL* object_create_by_name)(const char* class_name, int creator_role);

		void(RML_INTEROP_CALL* managed_log)(int32_t level, const char* utf8, int32_t len);

		void(RML_INTEROP_CALL* free_string)(const char* str);
		void(RML_INTEROP_CALL* free_native_ptr)(const void* ptr);
		
		uintptr_t(RML_INTEROP_CALL* mods_menu_add_action)(const char* text, ManagedEventCallback callback, void* state);
		void(RML_INTEROP_CALL* mods_menu_remove_action)(uintptr_t action_id);
	};

	inline constexpr uint32_t RML_INTEROP_VERSION = 4;

	class InteropRegistry
	{
	public:
		InteropRegistry();

		[[nodiscard]] InteropTable* table() noexcept
		{
			return &m_table;
		}
		[[nodiscard]] const InteropTable* table() const noexcept
		{
			return &m_table;
		}

	private:
		InteropTable m_table{};
	};

} // namespace rml::dotnet
