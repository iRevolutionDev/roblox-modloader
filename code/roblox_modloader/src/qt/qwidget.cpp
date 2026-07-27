#include "RobloxModLoader/qt/qwidget.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	void QWidget::setWindowTitle(const QString& title)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::setWindowTitle(QString const&)");
		if (fn)
			fn(this, title.data());
	}

	void QWidget::setStyleSheet(const QString& style)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::setStyleSheet(QString const&)");
		if (fn)
			fn(this, style.data());
	}

	void QWidget::show()
	{
		static const auto fn = detail::widgets<void (*)(void*)>("QWidget::show()");
		if (fn)
			fn(this);
	}

	void QWidget::resize(const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::resize(QSize const&)");
		const int size[2]{width, height};
		if (fn)
			fn(this, size);
	}

	void QWidget::setFixedSize(const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, int, int)>("QWidget::setFixedSize(int, int)");
		if (fn)
			fn(this, width, height);
	}

	void QWidget::setGeometry(const int x, const int y, const int width, const int height)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::setGeometry(QRect const&)");
		const int rect[4]{x, y, x + width - 1, y + height - 1};
		if (fn)
			fn(this, rect);
	}

	void QWidget::move(const int x, const int y)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::move(QPoint const&)");
		const int point[2]{x, y};
		if (fn)
			fn(this, point);
	}

	void QWidget::setEnabled(const bool enabled)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("QWidget::setEnabled(bool)");
		if (fn)
			fn(this, enabled);
	}

	void QWidget::setToolTip(const QString& text)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::setToolTip(QString const&)");
		if (fn)
			fn(this, text.data());
	}

	bool QWidget::close()
	{
		static const auto fn = detail::widgets<bool (*)(void*)>("QWidget::close()");
		return fn && fn(this);
	}

	QWidget* QWidget::viewport() const
	{
		static const auto fn = detail::widgets<void* (*)(const void*)>("QAbstractScrollArea::viewport() const");
		return fn ? static_cast<QWidget*>(fn(this)) : nullptr;
	}

	QPalette QWidget::palette() const
	{
		static const auto fn = detail::widgets<const void* (*)(const void*)>("QWidget::palette() const");
		return fn ? QPalette(fn(this)) : QPalette();
	}

	void QWidget::set_palette(const QPalette& palette)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QWidget::setPalette(QPalette const&)");
		if (fn && palette.valid())
			fn(this, palette.data());
	}
	
	static constexpr int PDM_WIDTH = 1;
	static constexpr int PDM_HEIGHT = 2;

	static int widget_metric(const QWidget* widget, const char* signature, const int metric)
	{
		if (void* const fn = detail::widgets_export_optional(signature))
			return reinterpret_cast<int (*)(const void*)>(fn)(widget);

		static const auto measure = detail::widgets<int (*)(const void*, int)>("QWidget::metric(QPaintDevice::PaintDeviceMetric) const");
		return measure ? measure(widget, metric) : 0;
	}

	int QWidget::width() const
	{
		return widget_metric(this, "QWidget::width() const", PDM_WIDTH);
	}

	int QWidget::height() const
	{
		return widget_metric(this, "QWidget::height() const", PDM_HEIGHT);
	}

	void QWidget::set_auto_fill_background(const bool enabled)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("QWidget::setAutoFillBackground(bool)");
		if (fn)
			fn(this, enabled);
	}

	void QWidget::update()
	{
		static const auto fn = detail::widgets<void (*)(void*)>("QWidget::update()");
		if (fn)
			fn(this);
	}
}
