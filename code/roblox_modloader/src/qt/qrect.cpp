#include "RobloxModLoader/qt/qrect.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

#include <cstring>

namespace rml::qt
{
	QRect::QRect(const int x, const int y, const int width, const int height)
	{
		static void* const ctor = detail::core_export_optional("QRect::QRect(int, int, int, int)");
		if (ctor)
		{
			reinterpret_cast<void (*)(void*, int, int, int, int)>(ctor)(m_storage, x, y, width, height);
			return;
		}
		
		const int corners[]{x, y, x + width - 1, y + height - 1};
		std::memcpy(m_storage, corners, sizeof(corners));
	}
}
