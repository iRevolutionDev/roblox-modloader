#pragma once
#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"
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
			if (!is_valid(index))
				return false;
			const auto& v = m_args[index - 1];
			switch (v.tag)
			{
			case InteropValueTag::Bool: value = v.as_bool; return true;
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
			case InteropValueTag::Bool: value = v.as_bool ? 1L : 0L; return true;
			default: return false;
			}
		}

		bool get_double(const int index, double& value) const override
		{
			if (!is_valid(index))
				return false;
			switch (const auto& v = m_args[index - 1]; v.tag)
			{
			case InteropValueTag::Double: value = v.as_double; return true;
			case InteropValueTag::Float: value = v.as_float; return true;
			case InteropValueTag::Int64: value = static_cast<double>(v.as_int64); return true;
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

			value = std::shared_ptr<RBX::Reflection::DescribedBase>(ptr, [](RBX::Reflection::DescribedBase*) {
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

		[[nodiscard]] void* get([[maybe_unused]] const int index) const override
		{
			if (!is_valid(index))
				return nullptr;

			const auto& v = m_args[index - 1];
			return reinterpret_cast<void*>(v.as_instance);
		}

	private:
		[[nodiscard]] bool is_valid(const int index) const noexcept
		{
			return index >= 1 && static_cast<uint32_t>(index) <= m_count;
		}
	};

	inline void write_return_value(const RBX::Reflection::Type* type, const uint64_t ret, const uint64_t ret_slot, const uintptr_t ret_slot_addr, InteropVariant& out) noexcept
	{
		if (!type)
		{
			out.tag       = InteropValueTag::Null;
			out.as_uint64 = ret;
			return;
		}
		if (!ret)
		{
			out.tag       = InteropValueTag::Null;
			out.as_uint64 = 0;
			return;
		}

		if (type->name == "Instances")
		{
			out.tag = InteropValueTag::InstanceArray;

			const auto* instances_sptr = reinterpret_cast<const std::shared_ptr<RBX::Instances>*>(ret_slot_addr);
			if (!instances_sptr || !*instances_sptr || (*instances_sptr)->empty())
			{
				out.as_instance = 0;
				return;
			}

			const auto& instances = **instances_sptr;
			const auto count      = static_cast<uint32_t>(instances.size());

			const auto buf_size = sizeof(uint64_t) + static_cast<size_t>(count) * sizeof(uintptr_t);
			auto* buf           = static_cast<uint8_t*>(std::malloc(buf_size));
			if (!buf)
			{
				out.as_instance = 0;
				return;
			}

			auto* count_field = reinterpret_cast<uint32_t*>(buf);
			auto* handles     = reinterpret_cast<uintptr_t*>(buf + sizeof(uint64_t));
			uint32_t written  = 0;

			for (const auto& element : instances)
			{
				if (const auto handle = reinterpret_cast<uintptr_t>(element.get()))
					handles[written++] = handle;
			}
			*count_field = written;

			out.as_instance = reinterpret_cast<uintptr_t>(buf);
			return;
		}

		out.tag      = InteropValueTag::Int64;
		out.as_int64 = static_cast<int64_t>(ret);
	}

} // namespace rml::dotnet