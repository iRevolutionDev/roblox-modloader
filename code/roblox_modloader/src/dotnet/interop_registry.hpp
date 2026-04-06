#pragma once
#include <cstdint>

namespace rml::dotnet
{
	struct alignas(8) InteropTable
	{
		uint32_t version;
		uint32_t size;

		uint64_t(__cdecl* reflection_invoke)(uintptr_t instance, const char* function_name, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint32_t arg_count);

		uint64_t(__cdecl* reflection_get_property)(uintptr_t instance, const char* property_name);

		uint64_t(__cdecl* reflection_set_property)(uintptr_t instance, const char* property_name, uint64_t value);

		uintptr_t(__cdecl* instance_get_class_descriptor)(uintptr_t instance);

		void(__cdecl* managed_log)(int32_t level, const char* utf8, int32_t len);
	};

	inline constexpr uint32_t RML_INTEROP_VERSION = 1;

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
