#include "RobloxModLoader/qt/qpainter.hpp"

#include "RobloxModLoader/qt/qpaintdevice.hpp"
#include "RobloxModLoader/qt/qpixmap.hpp"
#include "RobloxModLoader/qt/qrect.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

#include <cstring>

namespace rml::qt
{
	QPainter::QPainter(const QPixmap& target)
	{
		static const auto ctor = detail::gui<void (*)(void*, void*)>("QPainter::QPainter(QPaintDevice*)");
		if (!ctor)
			return;
		ctor(m_storage, target.data());
		set_owned(true);
	}

	QPainter::QPainter(const QWidget& target)
	{
		static const auto ctor = detail::gui<void (*)(void*, void*)>("QPainter::QPainter(QPaintDevice*)");
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

		static const auto dtor = detail::gui<void (*)(void*)>("QPainter::~QPainter()");
		if (dtor)
			dtor(m_storage);
	}

	void QPainter::set_render_hint(const RenderHint hint, const bool on)
	{
		static const auto fn = detail::gui<void (*)(void*, int, bool)>("QPainter::setRenderHint(QPainter::RenderHint, bool)");
		if (fn && owned())
			fn(m_storage, static_cast<int>(hint), on);
	}

	void QPainter::set_opacity(const double opacity)
	{
		static const auto fn = detail::gui<void (*)(void*, double)>("QPainter::setOpacity(double)");
		if (fn && owned())
			fn(m_storage, opacity);
	}

	void QPainter::draw_pixmap(const int x, const int y, const QPixmap& pixmap)
	{
		static void* const inlined = detail::gui_export_optional("QPainter::drawPixmap(int, int, QPixmap const&)");
		if (inlined && owned())
		{
			reinterpret_cast<void (*)(void*, int, int, const void*)>(inlined)(m_storage, x, y, pixmap.data());
			return;
		}

		static const auto at_point = detail::gui<void (*)(void*, const void*, const void*)>("QPainter::drawPixmap(QPointF const&, QPixmap const&)");
		if (!at_point || !owned())
			return;

		const double point[]{static_cast<double>(x), static_cast<double>(y)};
		at_point(m_storage, point, pixmap.data());
	}

	void QPainter::draw_pixmap(const QRect& target, const QPixmap& pixmap)
	{
		static void* const inlined = detail::gui_export_optional("QPainter::drawPixmap(QRect const&, QPixmap const&)");
		if (inlined && owned())
		{
			reinterpret_cast<void (*)(void*, const void*, const void*)>(inlined)(m_storage, target.data(), pixmap.data());
			return;
		}

		static const auto into_rect = detail::gui<void (*)(void*, const void*, const void*, const void*)>("QPainter::drawPixmap(QRectF const&, QPixmap const&, QRectF const&)");
		if (!into_rect || !owned())
			return;

		int corners[4]{};
		std::memcpy(corners, target.data(), sizeof(corners));

		const double destination[]{
		    static_cast<double>(corners[0]),
		    static_cast<double>(corners[1]),
		    static_cast<double>(corners[2] - corners[0] + 1),
		    static_cast<double>(corners[3] - corners[1] + 1),
		};
		
		const double source[]{0.0, 0.0, static_cast<double>(pixmap.width()), static_cast<double>(pixmap.height())};

		into_rect(m_storage, destination, pixmap.data(), source);
	}
}
