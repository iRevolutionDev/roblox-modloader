#pragma once

#include "descriptor.hpp"

#include <cstddef>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

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

	class Variant
	{
		const Type* m_type{nullptr};
		alignas(8) std::byte m_data[96]{};

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

		template<typename T>
		T* try_cast()
		{
			return reinterpret_cast<T*>(m_data);
		}

		template<typename T>
		const T* try_cast() const
		{
			return reinterpret_cast<const T*>(m_data);
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

	struct SignatureDescriptor
	{
		struct Item
		{
			const char* name;
			const Type* type;
			Variant default_value;

			bool has_default_value() const
			{
				return type && default_value.type() == *type;
			}
		};

		using Arguments = std::list<Item>;

		const Type* result_type{nullptr};
		Arguments arguments{};

		[[nodiscard]] bool has_result() const
		{
			return result_type != nullptr;
		}
		[[nodiscard]] bool has_arguments() const
		{
			return !arguments.empty();
		}

		[[nodiscard]] std::size_t argument_count() const
		{
			return arguments.size();
		}
	};
}