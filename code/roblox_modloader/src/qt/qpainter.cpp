#include "RobloxModLoader/qt/qpainter.hpp"

#include "RobloxModLoader/qt/qpaintdevice.hpp"
#include "RobloxModLoader/qt/qpixmap.hpp"
#include "RobloxModLoader/qt/qrect.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	QPainter::QPainter(const QPixmap& target)
	{
		static const auto ctor = detail::gui<void (*)(void*, void*)>("??0QPainter@@QEAA@PEAVQPaintDevice@@@Z");
		if (!ctor)
			return;
		ctor(m_storage, target.data());
		set_owned(true);
	}

	QPainter::QPainter(const QWidget& target)
	{
		static const auto ctor = detail::gui<void (*)(void*, void*)>("??0QPainter@@QEAA@PEAVQPaintDevice@@@Z");
		if (!ctor)
			return;

		auto* paint_device = static_cast<QPaintDevice*>(const_cast<QWidget*>(&target));
		ctor(m_storage, paint_device);
		set_owned(true);
	}

	QPainter::~QPainter()
	{
		if (!owned())
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("??1QPainter@@QEAA@XZ");
		if (dtor)
			dtor(m_storage);
	}

	void QPainter::set_render_hint(const RenderHint hint, const bool on)
	{
		static const auto fn = detail::gui<void (*)(void*, int, bool)>("?setRenderHint@QPainter@@QEAAXW4RenderHint@1@_N@Z");
		if (fn && owned())
			fn(m_storage, static_cast<int>(hint), on);
	}

	void QPainter::set_opacity(const double opacity)
	{
		static const auto fn = detail::gui<void (*)(void*, double)>("?setOpacity@QPainter@@QEAAXN@Z");
		if (fn && owned())
			fn(m_storage, opacity);
	}

	void QPainter::draw_pixmap(const int x, const int y, const QPixmap& pixmap)
	{
		static const auto fn = detail::gui<void (*)(void*, int, int, const void*)>("?drawPixmap@QPainter@@QEAAXHHAEBVQPixmap@@@Z");
		if (fn && owned())
			fn(m_storage, x, y, pixmap.data());
	}

	void QPainter::draw_pixmap(const QRect& target, const QPixmap& pixmap)
	{
		static const auto fn = detail::gui<void (*)(void*, const void*, const void*)>("?drawPixmap@QPainter@@QEAAXAEBVQRect@@AEBVQPixmap@@@Z");
		if (fn && owned())
			fn(m_storage, target.data(), pixmap.data());
	}
}
