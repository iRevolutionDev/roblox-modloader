#pragma once
#include "interop_registry.hpp"

#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"

namespace rml::dotnet
{
	class DotNetArguments final : public RBX::Reflection::FunctionDescriptor::Arguments
	{
		const InteropVariant* m_args;
		uint32_t              m_count;

	public:
		DotNetArguments(const InteropVariant* args, const uint32_t count) noexcept :
		    m_args(args),
		    m_count(count)
		{
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
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			switch (v.tag)
			{
				case InteropValueTag::Bool:  value = v.as_bool;       return true;
				case InteropValueTag::Int64: value = v.as_int64 != 0; return true;
				default: return false;
			}
		}

		bool get_long(const int index, long& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			switch (v.tag)
			{
				case InteropValueTag::Int64: value = static_cast<long>(v.as_int64); return true;
				case InteropValueTag::Bool:  value = v.as_bool ? 1L : 0L;           return true;
				default: return false;
			}
		}

		bool get_double(const int index, double& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			switch (v.tag)
			{
				case InteropValueTag::Double: value = v.as_double;                     return true;
				case InteropValueTag::Float:  value = v.as_float;                      return true;
				case InteropValueTag::Int64:  value = static_cast<double>(v.as_int64); return true;
				default: return false;
			}
		}

		bool get_string(const int index, std::string& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::String || !v.as_string)
				return false;
			value = v.as_string;
			return true;
		}

		bool get_vector3(const int index, RBX::Vector3& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::Instance)
				return false;
			const auto* ptr = reinterpret_cast<const RBX::Vector3*>(v.as_instance);
			if (!ptr)
				return false;
			value = *ptr;
			return true;
		}

		bool get_vector3_int16(const int index, RBX::Vector3int16& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::Instance)
				return false;
			const auto* ptr = reinterpret_cast<const RBX::Vector3int16*>(v.as_instance);
			if (!ptr)
				return false;
			value = *ptr;
			return true;
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
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::Instance)
				return false;
			const auto* ptr = reinterpret_cast<const RBX::Rect2D*>(v.as_instance);
			if (!ptr)
				return false;
			value = *ptr;
			return true;
		}

		bool get_object(const int index, std::shared_ptr<RBX::Reflection::DescribedBase>& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::Instance)
				return false;
			auto* ptr = reinterpret_cast<RBX::Reflection::DescribedBase*>(v.as_instance);
			value     = std::shared_ptr<RBX::Reflection::DescribedBase>(ptr, [](RBX::Reflection::DescribedBase*) {
            });
			return true;
		}

		bool get_enum(const int index, [[maybe_unused]] const RBX::Reflection::EnumDescriptor& desc, int& value) const override
		{
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			if (v.tag != InteropValueTag::Int64)
				return false;
			value = static_cast<int>(v.as_int64);
			return true;
		}

	private:
		[[nodiscard]] bool is_valid(const int index) const noexcept
		{
			return index >= 1 && static_cast<uint32_t>(index) <= m_count;
		}
	};

	inline void write_return_value(const RBX::Reflection::Variant& ret, InteropVariant& out) noexcept
	{
		if (ret.is_void())
		{
			out.tag      = InteropValueTag::Null;
			out.as_uint64 = 0;
			return;
		}

		if (ret.is_float())
		{
			out.as_uint64 = 0;
			out.tag       = InteropValueTag::Float;
			out.as_float  = *ret.try_cast<float>();
			return;
		}

		out.tag      = InteropValueTag::Int64;
		out.as_uint64 = *ret.try_cast<uint64_t>();
	}

} // namespace rml::dotnet