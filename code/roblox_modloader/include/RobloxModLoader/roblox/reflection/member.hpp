#pragma once

#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "descriptor.hpp"

#include <string_view>
#include <unordered_map>

namespace RBX::Reflection
{
	class ClassDescriptor;

	template<typename T>
	struct Vector
	{
		using value_type     = T;
		using iterator       = T*;
		using const_iterator = T*;

		T* m_data;
		std::size_t m_size;
		std::size_t m_capacity;

		[[nodiscard]] std::size_t size() const noexcept
		{
			return m_size;
		}
		[[nodiscard]] std::size_t capacity() const noexcept
		{
			return m_capacity;
		}
		[[nodiscard]] bool empty() const noexcept
		{
			return m_size == 0;
		}
		[[nodiscard]] T* data() const noexcept
		{
			return m_data;
		}

		[[nodiscard]] T* begin() const noexcept
		{
			return m_data;
		}
		[[nodiscard]] T* end() const noexcept
		{
			return m_data + m_size;
		}

		T& operator[](std::size_t i) const noexcept
		{
			return m_data[i];
		}
		[[nodiscard]] T& at(std::size_t i) const noexcept
		{
			return m_data[i];
		}
	};

	struct StringHashPredicate
	{
		size_t operator()(const char* s) const;
	};

	struct StringEqualPredicate
	{
		bool operator()(const char* lhs, const char* rhs) const
		{
			return strcmp(lhs, rhs) == 0;
		}
	};

	template<typename MemberDescriptorType>
	class MemberDescriptorContainer
	{
	public:
		using Collection = Vector<MemberDescriptorType*>;

		using ConstIterator = MemberDescriptorType**;
		using Iterator      = MemberDescriptorType**;

	private:
		using ResolvedLookup = std::unordered_map<std::string_view, MemberDescriptorType*>;

	protected:
		Collection descriptors;
		char _descriptor_lookup_pad[0x48];

	public:
		const Collection& get_descriptors() const
		{
			return descriptors;
		}

		ConstIterator descriptors_begin() const noexcept
		{
			return descriptors.begin();
		}
		ConstIterator descriptors_end() const noexcept
		{
			return descriptors.end();
		}

		std::size_t descriptor_size() const
		{
			return descriptors.size();
		}

		MemberDescriptorType* find_descriptor(const char* name) const
		{
			for (MemberDescriptorType* descriptor : descriptors)
			{
				if (!descriptor)
				{
					continue;
				}

				if (descriptor->name == name)
				{
					return descriptor;
				}
			}
			return nullptr;
		}

		ConstIterator members_begin(const void*) const
		{
			return descriptors_begin();
		}
		ConstIterator members_end(const void*) const
		{
			return descriptors_end();
		}
		Iterator members_begin(void*) const
		{
			return descriptors_begin();
		}
		Iterator members_end(void*) const
		{
			return descriptors_end();
		}
	};

	class MemberDescriptor : public Descriptor
	{
	public:
		static void (*member_hiding_hook)(MemberDescriptor*, MemberDescriptor*);
		const std::string_view& category;
		const ClassDescriptor& owner;
		const Security::Permissions security;

	protected:
		virtual ~MemberDescriptor() = default;
	};
}