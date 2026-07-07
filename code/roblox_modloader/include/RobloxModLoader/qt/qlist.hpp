#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstddef>

namespace rml::qt::detail
{
	struct QListData
	{
		int ref;
		int alloc;
		int begin;
		int end;
		void* array[1];

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_OFFSET(QListData, begin, sizeof(int) * 2);
			RML_ASSERT_LAYOUT_OFFSET(QListData, end, sizeof(int) * 3);
			RML_ASSERT_LAYOUT_OFFSET(QListData, array, sizeof(int) * 4);
		RML_LAYOUT_GUARD_END()
	};
}

namespace rml::qt
{
	template<typename T>
	class QList
	{
	public:
		class ConstIterator
		{
		public:
			ConstIterator(const detail::QListData* data, const int index) noexcept :
			    m_data(data), m_index(index)
			{
			}

			[[nodiscard]] T operator*() const noexcept
			{
				return static_cast<T>(m_data->array[m_index]);
			}

			ConstIterator& operator++() noexcept
			{
				++m_index;
				return *this;
			}

			[[nodiscard]] bool operator!=(const ConstIterator& other) const noexcept
			{
				return m_index != other.m_index;
			}

		private:
			const detail::QListData* m_data;
			int m_index;
		};

		QList() = default;

		[[nodiscard]] int size() const noexcept
		{
			return d ? d->end - d->begin : 0;
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return size() == 0;
		}

		[[nodiscard]] T at(const int index) const noexcept
		{
			return static_cast<T>(d->array[d->begin + index]);
		}

		[[nodiscard]] ConstIterator begin() const noexcept
		{
			return ConstIterator(d, d ? d->begin : 0);
		}

		[[nodiscard]] ConstIterator end() const noexcept
		{
			return ConstIterator(d, d ? d->end : 0);
		}

		[[nodiscard]] void** raw_storage() noexcept
		{
			return reinterpret_cast<void**>(&d);
		}

	private:
		detail::QListData* d = nullptr;
	};
}
