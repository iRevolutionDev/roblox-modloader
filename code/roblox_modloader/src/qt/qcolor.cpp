#include "RobloxModLoader/qt/qcolor.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QColor::QColor(const int red, const int green, const int blue, const int alpha)
	{
		static const auto ctor = detail::gui<void (*)(void*, int, int, int, int)>("??0QColor@@QEAA@HHHH@Z");
		if (ctor)
			ctor(m_storage, red, green, blue, alpha);
	}
}
