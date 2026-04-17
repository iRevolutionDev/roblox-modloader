#pragma once

#include "descriptor.hpp"
#include "member.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>

namespace RBX::Reflection
{
	class Type : public Descriptor
	{
	public:
		const Name& tag;
		const bool is_float;
		const bool is_number;
		const bool is_enum;

		bool operator==(const Type& other) const noexcept
		{
			return this == &other;
		}
		bool operator!=(const Type& other) const noexcept
		{
			return this != &other;
		}
	};

	template<typename T>
	class TType : public Type
	{
		friend class Type;

	protected:
		explicit TType(const char* name) :
		    Type(name, const_cast<T*>(nullptr))
		{
		}
		TType(const char* name, const char* tag) :
		    Type(name, tag, const_cast<T*>(nullptr))
		{
		}
	};

	class Variant
	{
		struct Storage
		{
			std::byte data[88]{};
		};

		const Type* m_type{nullptr};
		alignas(8) Storage m_storage;

	public:
		Variant()                          = default;
		Variant(const Variant&)            = default;
		Variant& operator=(const Variant&) = default;

		[[nodiscard]] const Type& type() const
		{
			return *m_type;
		}

		bool is_float() const
		{
			return m_type->is_float;
		}

		bool is_number() const
		{
			return m_type->is_number;
		}

		bool is_enum() const
		{
			return m_type->is_enum;
		}

		[[nodiscard]] bool is_void() const noexcept
		{
			return m_type == nullptr;
		}

		template<typename T>
		T* try_cast()
		{
			return reinterpret_cast<T*>(m_storage.data);
		}

		template<typename T>
		const T* try_cast() const
		{
			return reinterpret_cast<const T*>(m_storage.data);
		}
	};

	using EventArguments = std::vector<Variant>;
	using ValueArray     = std::vector<Variant>;
	using ValueMap       = std::unordered_map<std::string, Variant>;

	struct Tuple
	{
		ValueArray values;

		Tuple() = default;
		explicit Tuple(const std::size_t count) :
		    values(count)
		{
		}

		Variant& at(const std::size_t i)
		{
			return values[i];
		}

		[[nodiscard]] const Variant& at(const std::size_t i) const
		{
			return values[i];
		}
	};

	class SignatureDescriptor
	{
		struct Item
		{
			friend class SignatureDescriptor;

			const Name* name;
			const Type* type;
			const Variant default_handle;

			[[nodiscard]] bool has_default_value() const noexcept
			{
				return !default_handle.is_void();
			}
		};

		struct ResultItem
		{
			const Type* type;
			std::uint64_t _unk0;
		};

		Vector<Item> m_arguments;
		Vector<ResultItem> m_result_types;

	public:
		[[nodiscard]] const Vector<Item>& arguments() const noexcept
		{
			return m_arguments;
		}

		[[nodiscard]] const Vector<ResultItem>& result_types() const noexcept
		{
			return m_result_types;
		}
	};
}