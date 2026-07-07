#include "RobloxModLoader/qt/qbrush.hpp"

#include "RobloxModLoader/qt/qpixmap.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QBrush::QBrush(const QPixmap& texture)
	{
		static const auto ctor = detail::gui<void (*)(void*, const void*)>("??0QBrush@@QEAA@AEBVQPixmap@@@Z");
		if (!ctor)
			return;

		ctor(m_storage, texture.data());
		set_owned(true);
	}

	QBrush::~QBrush()
	{
		if (!owned())
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("??1QBrush@@QEAA@XZ");
		if (dtor)
			dtor(m_storage);
	}
}
