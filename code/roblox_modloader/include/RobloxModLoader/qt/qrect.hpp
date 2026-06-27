#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QRect
	{
	public:
		QRect() = default;
		QRect(int x, int y, int width, int height);

		[[nodiscard]] void* data() const
		{
			return const_cast<unsigned char*>(m_storage);
		}

	private:
		alignas(void*) unsigned char m_storage[16]{};
	};
}
