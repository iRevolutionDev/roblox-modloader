#include "RobloxModLoader/qt/qrect.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QRect::QRect(const int x, const int y, const int width, const int height)
	{
		static const auto ctor = detail::core<void (*)(void*, int, int, int, int)>("??0QRect@@QEAA@HHHH@Z");
		if (ctor)
			ctor(m_storage, x, y, width, height);
	}
}
