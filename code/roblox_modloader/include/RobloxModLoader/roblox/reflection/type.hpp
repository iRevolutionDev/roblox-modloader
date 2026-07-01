#pragma once

#include "descriptor.hpp"
#include "member.hpp"

#include <cstddef>
#include <span>
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
			std::byte data[0x40]{};
		};

		const Type* m_type{nullptr};
		const void* m_value_ops{nullptr};
		alignas(8) Storage m_storage;

	public:
		Variant() = default;
		Variant(const Variant&) = default;
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

		[[nodiscard]] const void* value_ops() const noexcept
		{
			return m_value_ops;
		}

		[[nodiscard]] void* storage() noexcept
		{
			return m_storage.data;
		}

		void set_type_and_ops(const Type* type, const void* value_ops) noexcept
		{
			m_type = type;
			m_value_ops = value_ops;
		}
	};

	using EventArguments = std::vector<Variant>;
	using ValueArray = std::vector<Variant>;
	using ValueMap = std::unordered_map<std::string, Variant>;

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
			const void* _reserved0;
			const void* _reserved1;
			const Variant default_handle;

			[[nodiscard]] bool has_default_value() const noexcept
			{
				return !default_handle.is_void();
			}
		};
		
		struct ResultItem
		{
			const Type* type;
			std::uint64_t _unk0; // idk what's this
		};
		static_assert(sizeof(ResultItem) == 0x10);

		// i need to reverse more, but it's solve, im lazy
		template<typename T>
		struct LinkedList
		{
			T* m_begin{nullptr};
			T* m_end{nullptr};
			T* m_capacity{nullptr};

			[[nodiscard]] bool empty() const noexcept
			{
				return m_begin == m_end;
			}
			[[nodiscard]] std::size_t size() const noexcept
			{
				return static_cast<std::size_t>(m_end - m_begin);
			}
			[[nodiscard]] T& front() const
			{
				return *m_begin;
			}
			[[nodiscard]] std::span<T> span() const noexcept
			{
				return {m_begin, m_end};
			}
		};
		static_assert(sizeof(LinkedList<void*>) == 0x18);

		LinkedList<Item> m_arguments;
		LinkedList<ResultItem> m_result_types;

	public:
		[[nodiscard]] std::span<const Item> arguments() const noexcept
		{
			return m_arguments.span();
		}

		[[nodiscard]] std::span<const ResultItem> result_types() const noexcept
		{
			return m_result_types.span();
		}

		[[nodiscard]] const Type* first_result_type() const noexcept
		{
			return m_result_types.empty() ? nullptr : m_result_types.front().type;
		}
	};
	static_assert(sizeof(SignatureDescriptor) == 0x30);
}