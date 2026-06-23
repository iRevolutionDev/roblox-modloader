#pragma once
#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"
#include "dotnet_variant.hpp"
#include "interop_registry.hpp"

namespace rml::dotnet
{
	class DotNetArguments final : public RBX::Reflection::FunctionDescriptor::Arguments
	{
		uint64_t m_return_value_hi;
		const InteropVariant* m_args;
		uint32_t m_count;

	public:
		DotNetArguments(const InteropVariant* args, const uint32_t count) noexcept :
		    m_return_value_hi(0),
		    m_args(args),
		    m_count(count)
		{
			return_value = 0;
		}

		[[nodiscard]] size_t size() const override
		{
			return m_count;
		}

		bool get_varint([[maybe_unused]] int index, [[maybe_unused]] RBX::Reflection::Variant& value) const override
		{
			return false;
		}

		bool get_bool(const int index, bool& value) const override
		{
			return is_valid(index) && read_bool(m_args[index - 1], value);
		}

		bool get_long(const int index, long& value) const override
		{
			int64_t wide;
			if (!is_valid(index) || !read_int64(m_args[index - 1], wide))
				return false;
			value = static_cast<long>(wide);
			return true;
		}

		bool get_double(const int index, double& value) const override
		{
			return is_valid(index) && read_double(m_args[index - 1], value);
		}

		bool get_string(const int index, std::string& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto* str = read_string(m_args[index - 1]);
			if (!str)
				return false;
			value = str;
			return true;
		}

		bool get_vector3(const int index, RBX::Vector3& value) const override
		{
			return read_struct_arg(index, value);
		}

		bool get_vector3_int16(const int index, RBX::Vector3int16& value) const override
		{
			return read_struct_arg(index, value);
		}

		bool get_region3_int16([[maybe_unused]] int index, [[maybe_unused]] void* value) const override
		{
			return false;
		}

		bool get_region3([[maybe_unused]] int index, [[maybe_unused]] void* value) const override
		{
			return false;
		}

		bool get_rect(const int index, RBX::Rect2D& value) const override
		{
			return read_struct_arg(index, value);
		}

		bool get_object(const int index, std::shared_ptr<RBX::Reflection::DescribedBase>& value) const override
		{
			if (!is_valid(index))
				return false;
			auto* ptr = read_instance<RBX::Reflection::DescribedBase>(m_args[index - 1]);
			if (!ptr)
				return false;

			value = std::shared_ptr<RBX::Reflection::DescribedBase>(ptr, [](RBX::Reflection::DescribedBase*) {
			});
			return true;
		}

		bool get_enum(const int index, [[maybe_unused]] const RBX::Reflection::EnumDescriptor& desc, int& value) const override
		{
			int64_t wide;
			if (!is_valid(index) || !read_int64(m_args[index - 1], wide))
				return false;
			value = static_cast<int>(wide);
			return true;
		}

		[[nodiscard]] void* get([[maybe_unused]] const int index) const override
		{
			if (!is_valid(index))
				return nullptr;
			
			return reinterpret_cast<void*>(m_args[index - 1].as_instance);
		}

	private:
		[[nodiscard]] bool is_valid(const int index) const noexcept
		{
			return index >= 1 && static_cast<uint32_t>(index) <= m_count;
		}

		template<typename T>
		[[nodiscard]] bool read_struct_arg(const int index, T& out) const
		{
			if (!is_valid(index))
				return false;
			const auto* ptr = read_instance<const T>(m_args[index - 1]);
			if (!ptr)
				return false;
			out = *ptr;
			return true;
		}
	};

} // namespace rml::dotnet