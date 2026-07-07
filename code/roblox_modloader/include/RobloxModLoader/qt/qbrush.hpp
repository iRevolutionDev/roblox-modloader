#pragma once

#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

namespace rml::qt
{
	class QPixmap;

	class RML_EXPORT QBrush
	{
	public:
		explicit QBrush(const QPixmap& texture);
		~QBrush();

		QBrush(const QBrush&) = delete;
		QBrush& operator=(const QBrush&) = delete;

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		static constexpr std::size_t STORAGE_SIZE = 16;

		alignas(void*) unsigned char m_storage[STORAGE_SIZE]{};
		bool m_constructed = false;

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
