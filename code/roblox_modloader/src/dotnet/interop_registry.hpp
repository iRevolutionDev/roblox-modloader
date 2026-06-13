#pragma once
#include <cstdint>

namespace rml::dotnet
{
	enum class InteropValueTag : uint8_t
	{
		Null          = 0,
		Bool          = 1,
		Int64         = 2,
		Double        = 3,
		Float         = 4,
		String        = 5,
		Instance      = 6,
		InstanceArray = 7,
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

	struct alignas(8) InteropTable
	{
		uint32_t version;
		uint32_t size;

		void*(__cdecl* get_proc_address)(const char* name);

		void(__cdecl* reflection_invoke)(uintptr_t instance, const char* function_name, const InteropVariant* args, uint32_t arg_count, InteropVariant* out_result);

		void(__cdecl* reflection_get_property)(uintptr_t instance, const char* property_name, InteropVariant* out_value);

		void(__cdecl* reflection_set_property)(uintptr_t instance, const char* property_name, const InteropVariant* value);

		uintptr_t(__cdecl* instance_get_class_descriptor)(uintptr_t instance);

		void(__cdecl* managed_log)(int32_t level, const char* utf8, int32_t len);

		void(__cdecl* free_string)(const char* str);
		
		void(__cdecl* free_native_ptr)(const void* ptr);
	};

	inline constexpr uint32_t RML_INTEROP_VERSION = 2;

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
