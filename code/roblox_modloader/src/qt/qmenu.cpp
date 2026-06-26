#include "RobloxModLoader/qt/qmenu.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QAction QMenu::addAction(const QString& text) const
	{
		static const auto fn = detail::widgets<void* (*)(void*, const void*)>("?addAction@QMenu@@QEAAPEAVQAction@@AEBVQString@@@Z");
		return (fn && m_this) ? QAction{fn(m_this, text.data())} : QAction{};
	}

	QAction QMenu::addSeparator() const
	{
		static const auto fn = detail::widgets<void* (*)(void*)>("?addSeparator@QMenu@@QEAAPEAVQAction@@XZ");
		return (fn && m_this) ? QAction{fn(m_this)} : QAction{};
	}
}
