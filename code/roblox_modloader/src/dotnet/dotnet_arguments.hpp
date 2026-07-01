#pragma once
#include "RobloxModLoader/roblox/instance.hpp"
#include "RobloxModLoader/roblox/reflection/function_descriptor.hpp"
#include "RobloxModLoader/roblox/reflection/type.hpp"
#include "dotnet_variant.hpp"
#include "interop_registry.hpp"

#include <cstddef>
#include <deque>
#include <new>
#include <string>

namespace rml::dotnet
{
	class DotNetArguments final : public RBX::Reflection::FunctionDescriptor::Arguments
	{
		std::byte m_return_value_tail[56]{};
		const InteropVariant* m_args;
		uint32_t m_count;

		const RBX::Reflection::SignatureDescriptor* m_signature{nullptr};

		mutable std::deque<std::string> m_string_storage;

	public:
		DotNetArguments(const InteropVariant* args, const uint32_t count,
		    const RBX::Reflection::SignatureDescriptor* signature = nullptr) noexcept :
		    m_args(args),
		    m_count(count),
		    m_signature(signature)
		{
			return_value = 0;
		}

		[[nodiscard]] size_t size() const override
		{
			return m_count;
		}

		bool get_varint(const int index, RBX::Reflection::Variant& value) const override
		{
			if (!is_valid(index) || !m_signature)
				return false;

			const auto sig_args = m_signature->arguments();
			if (static_cast<size_t>(index - 1) >= sig_args.size())
				return false;

			const auto* type = sig_args[index - 1].type;
			if (!type)
				return false;

			value.set_type_and_ops(type, borrow_value_ops(type));

			const auto& v = m_args[index - 1];
			void* const storage = value.storage();
			const auto& name = type->name;

			if (name == "string")
			{
				::new (storage) std::string(v.as_string ? v.as_string : "");
				return true;
			}
			if (name == "bool")
			{
				bool b = false;
				(void) read_bool(v, b);
				*static_cast<bool*>(storage) = b;
				return true;
			}
			if (name == "float" || name == "double")
			{
				double d = 0.0;
				(void) read_double(v, d);
				if (name == "float")
					*static_cast<float*>(storage) = static_cast<float>(d);
				else
					*static_cast<double*>(storage) = d;
				return true;
			}
			if (type->is_number || type->is_enum || name == "int" || name == "long")
			{
				int64_t wide = 0;
				(void) read_int64(v, wide);
				if (name == "int64" || name == "long")
					*static_cast<int64_t*>(storage) = wide;
				else
					*static_cast<int*>(storage) = static_cast<int>(wide);
				return true;
			}

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

		[[nodiscard]] void* get(const int index) const override
		{
			if (!is_valid(index))
				return nullptr;

			const auto& v = m_args[index - 1];

			if (v.tag == InteropValueTag::String)
				return &m_string_storage.emplace_back(v.as_string ? v.as_string : "");

			return reinterpret_cast<void*>(v.as_uint64);
		}

	private:
		[[nodiscard]] bool is_valid(const int index) const noexcept
		{
			return index >= 1 && static_cast<uint32_t>(index) <= m_count;
		}

		[[nodiscard]] const void* borrow_value_ops(const RBX::Reflection::Type* type) const
		{
			if (m_signature)
			{
				for (const auto& arg : m_signature->arguments())
				{
					if (arg.type == type && !arg.default_handle.is_void())
					{
						if (const void* ops = arg.default_handle.value_ops())
							return ops;
					}
				}
			}

			static void (*const destroy_string)(void*) = [](void* storage) {
				static_cast<std::string*>(storage)->~basic_string();
			};
			static void (*const destroy_trivial)(void*) = [](void*) {};
			static const void* string_ops[3] = {nullptr, nullptr, reinterpret_cast<void*>(destroy_string)};
			static const void* trivial_ops[3] = {nullptr, nullptr, reinterpret_cast<void*>(destroy_trivial)};

			return type && type->name == "string" ? string_ops : trivial_ops;
		}

		template<typename T>
		[[nodiscard]] bool read_struct_arg(const int index, T& out) const
		{
			if (!is_valid(index))
				return false;
			const auto* ptr = read_struct_ptr<T>(m_args[index - 1]);
			if (!ptr)
				return false;
			out = *ptr;
			return true;
		}
	};

} // namespace rml::dotnet