#include "RobloxModLoader/qt/qmenubar.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QMenu QMenuBar::addMenu(const QString& title) const
	{
		static const auto fn = detail::widgets<void* (*)(void*, const void*)>("?addMenu@QMenuBar@@QEAAPEAVQMenu@@AEBVQString@@@Z");
		return (fn && m_this) ? QMenu{fn(m_this, title.data())} : QMenu{};
	}
}
