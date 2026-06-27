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
		m_constructed = true;
	}

	QBrush::~QBrush()
	{
		if (!m_constructed)
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("??1QBrush@@QEAA@XZ");
		if (dtor)
			dtor(m_storage);
	}
}
