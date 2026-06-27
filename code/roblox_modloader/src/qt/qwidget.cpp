#include "RobloxModLoader/qt/qwidget.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	void QWidget::setWindowTitle(const QString& title)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setWindowTitle@QWidget@@QEAAXAEBVQString@@@Z");
		if (fn)
			fn(this, title.data());
	}

	void QWidget::setStyleSheet(const QString& style)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setStyleSheet@QWidget@@QEAAXAEBVQString@@@Z");
		if (fn)
			fn(this, style.data());
	}

	void QWidget::show()
	{
		static const auto fn = detail::widgets<void (*)(void*)>("?show@QWidget@@QEAAXXZ");
		if (fn)
			fn(this);
	}

	void QWidget::resize(const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int)>("?resize@QWidget@@QEAAXHH@Z");
		if (fn)
			fn(this, width, height);
	}

	void QWidget::setFixedSize(const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int)>("?setFixedSize@QWidget@@QEAAXHH@Z");
		if (fn)
			fn(this, width, height);
	}

	void QWidget::setGeometry(const int x, const int y, const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int, int, int)>("?setGeometry@QWidget@@QEAAXHHHH@Z");
		if (fn)
			fn(this, x, y, width, height);
	}

	void QWidget::move(const int x, const int y)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int)>("?move@QWidget@@QEAAXHH@Z");
		if (fn)
			fn(this, x, y);
	}

	void QWidget::setEnabled(const bool enabled)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setEnabled@QWidget@@QEAAX_N@Z");
		if (fn)
			fn(this, enabled);
	}

	void QWidget::setToolTip(const QString& text)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setToolTip@QWidget@@QEAAXAEBVQString@@@Z");
		if (fn)
			fn(this, text.data());
	}

	bool QWidget::close()
	{
		static const auto fn = detail::widgets<bool (*)(void*)>("?close@QWidget@@QEAA_NXZ");
		return fn && fn(this);
	}

	QWidget* QWidget::viewport() const
	{
		static const auto fn = detail::widgets<void* (*)(const void*)>("?viewport@QAbstractScrollArea@@QEBAPEAVQWidget@@XZ");
		return fn ? static_cast<QWidget*>(fn(this)) : nullptr;
	}

	QPalette QWidget::palette() const
	{
		static const auto fn = detail::widgets<const void* (*)(const void*)>("?palette@QWidget@@QEBAAEBVQPalette@@XZ");
		return fn ? QPalette(fn(this)) : QPalette();
	}

	void QWidget::set_palette(const QPalette& palette)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setPalette@QWidget@@QEAAXAEBVQPalette@@@Z");
		if (fn && palette.valid())
			fn(this, palette.data());
	}

	int QWidget::width() const
	{
		static const auto fn = detail::widgets<int (*)(const void*)>("?width@QWidget@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	int QWidget::height() const
	{
		static const auto fn = detail::widgets<int (*)(const void*)>("?height@QWidget@@QEBAHXZ");
		return fn ? fn(this) : 0;
	}

	void QWidget::set_auto_fill_background(const bool enabled)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setAutoFillBackground@QWidget@@QEAAX_N@Z");
		if (fn)
			fn(this, enabled);
	}

	void QWidget::update()
	{
		static const auto fn = detail::widgets<void (*)(void*)>("?update@QWidget@@QEAAXXZ");
		if (fn)
			fn(this);
	}
}
