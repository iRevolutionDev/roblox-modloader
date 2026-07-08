#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstddef>
#include <cstring>

namespace rml::qt::detail
{
	template<std::size_t N>
	class QtValueType
	{
	public:
		QtValueType() = default;

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

		[[nodiscard]] void* storage()
		{
			return m_storage;
		}

	protected:
		static constexpr std::size_t STORAGE_SIZE = N;

		void adopt(QtValueType& other) noexcept
		{
			std::memcpy(m_storage, other.m_storage, sizeof(m_storage));
			m_owned = other.m_owned;
			other.m_owned = false;
		}

		[[nodiscard]] bool owned() const noexcept
		{
			return m_owned;
		}

		void set_owned(const bool owned) noexcept
		{
			m_owned = owned;
		}

		alignas(void*) unsigned char m_storage[STORAGE_SIZE]{};

	private:
		bool m_owned = false;

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
