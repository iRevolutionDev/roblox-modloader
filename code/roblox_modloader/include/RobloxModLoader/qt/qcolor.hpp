#pragma once

#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

namespace rml::qt
{
	class RML_EXPORT QColor
	{
	public:
		QColor() = default;
		QColor(int red, int green, int blue, int alpha = 255);

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		static constexpr std::size_t STORAGE_SIZE = 24;

		alignas(void*) unsigned char m_storage[STORAGE_SIZE]{};

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
