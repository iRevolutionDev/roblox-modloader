#pragma once

#include "RobloxModLoader/rml_export.hpp"

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
		alignas(void*) unsigned char m_storage[24]{};
	};
}
