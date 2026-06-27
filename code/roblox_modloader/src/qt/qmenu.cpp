#include "RobloxModLoader/qt/qmenu.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QAction* QMenu::addAction(const QString& text)
	{
		static const auto fn = detail::widgets<void* (*)(void*, const void*)>("?addAction@QMenu@@QEAAPEAVQAction@@AEBVQString@@@Z");
		return fn ? static_cast<QAction*>(fn(this, text.data())) : nullptr;
	}

	QAction* QMenu::addSeparator()
	{
		static const auto fn = detail::widgets<void* (*)(void*)>("?addSeparator@QMenu@@QEAAPEAVQAction@@XZ");
		return fn ? static_cast<QAction*>(fn(this)) : nullptr;
	}
}
