#pragma once

#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"
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
		struct CollectionEntry
		{
			MemberDescriptorType* descriptor;
			uint64_t              unk;
		};
		using Collection = Vector<CollectionEntry>;

		struct ConstIterator
		{
			const CollectionEntry* ptr;

			MemberDescriptorType* operator*() const noexcept { return ptr->descriptor; }
			ConstIterator& operator++() noexcept { ++ptr; return *this; }
			bool operator!=(const ConstIterator& o) const noexcept { return ptr != o.ptr; }
		};
		using Iterator = ConstIterator;

		struct DescriptorView
		{
			ConstIterator m_begin, m_end;
			ConstIterator begin() const noexcept { return m_begin; }
			ConstIterator end() const noexcept { return m_end; }
			[[nodiscard]] std::size_t size() const noexcept { return static_cast<std::size_t>(m_end.ptr - m_begin.ptr); }
			[[nodiscard]] bool empty() const noexcept { return m_begin.ptr == m_end.ptr; }
		};

	private:
		using ResolvedLookup = std::unordered_map<std::string_view, MemberDescriptorType*>;

	protected:
		char _descriptor_pre_pad[0x18];
		Collection descriptors;
		char _descriptor_post_pad[0x30];

	public:
		const Collection& get_descriptors() const
		{
			return descriptors;
		}

		DescriptorView get_descriptor_view() const noexcept
		{
			return {descriptors_begin(), descriptors_end()};
		}

		ConstIterator descriptors_begin() const noexcept
		{
			return {descriptors.begin()};
		}
		ConstIterator descriptors_end() const noexcept
		{
			return {descriptors.end()};
		}

		std::size_t descriptor_size() const
		{
			return descriptors.size();
		}

		MemberDescriptorType* find_descriptor(const char* name) const
		{
			for (const CollectionEntry& entry : descriptors)
			{
				if (entry.descriptor && entry.descriptor->name == name)
				{
					return entry.descriptor;
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

	RML_ASSERT_LAYOUT_SIZE(MemberDescriptorContainer<ClassDescriptor>, 0x60);

	class MemberDescriptor : public Descriptor
	{
	public:
		static void (*member_hiding_hook)(MemberDescriptor*, MemberDescriptor*);

		const Name& category;
		const ClassDescriptor& owner;
		const Security::Permissions security;

	protected:
		virtual ~MemberDescriptor() = default;

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(MemberDescriptor, 0x40);
			RML_ASSERT_LAYOUT_OFFSET(MemberDescriptor, security, 0x38);
		RML_LAYOUT_GUARD_END()
	};
}