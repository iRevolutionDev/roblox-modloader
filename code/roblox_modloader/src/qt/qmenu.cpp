#include "RobloxModLoader/qt/qmenu.hpp"

#include "RobloxModLoader/qt/qicon.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QAction* QMenu::addAction(const QString& text)
	{
		static const auto fn = detail::widgets<void* (*)(void*, const void*)>("?addAction@QMenu@@QEAAPEAVQAction@@AEBVQString@@@Z");
		return fn ? static_cast<QAction*>(fn(this, text.data())) : nullptr;
	}

	QMenu* QMenu::addMenu(const QString& title)
	{
		static const auto fn = detail::widgets<void* (*)(void*, const void*)>("?addMenu@QMenu@@QEAAPEAV1@AEBVQString@@@Z");
		return fn ? static_cast<QMenu*>(fn(this, title.data())) : nullptr;
	}

	QAction* QMenu::addSeparator()
	{
		static const auto fn = detail::widgets<void* (*)(void*)>("?addSeparator@QMenu@@QEAAPEAVQAction@@XZ");
		return fn ? static_cast<QAction*>(fn(this)) : nullptr;
	}

	void QMenu::clear()
	{
		static const auto fn = detail::widgets<void (*)(void*)>("?clear@QMenu@@QEAAXXZ");
		if (fn)
			fn(this);
	}

	void QMenu::setIcon(const QIcon& icon)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setIcon@QMenu@@QEAAXAEBVQIcon@@@Z");
		if (fn)
			fn(this, icon.data());
	}

	QAction* QMenu::menuAction() const
	{
		static const auto fn = detail::widgets<void* (*)(const void*)>("?menuAction@QMenu@@QEBAPEAVQAction@@XZ");
		return fn ? static_cast<QAction*>(fn(this)) : nullptr;
	}
}
