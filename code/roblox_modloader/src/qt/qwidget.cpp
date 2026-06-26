#include "RobloxModLoader/qt/qwidget.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	void QWidget::setWindowTitle(const QString& title) const
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setWindowTitle@QWidget@@QEAAXAEBVQString@@@Z");
		if (fn && m_this)
			fn(m_this, title.data());
	}

	void QWidget::setStyleSheet(const QString& style) const
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setStyleSheet@QWidget@@QEAAXAEBVQString@@@Z");
		if (fn && m_this)
			fn(m_this, style.data());
	}

	void QWidget::show() const
	{
		static const auto fn = detail::widgets<void (*)(void*)>("?show@QWidget@@QEAAXXZ");
		if (fn && m_this)
			fn(m_this);
	}
}
