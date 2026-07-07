#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QPixmap;
	class QRect;
	class QWidget;

	class RML_EXPORT QPainter : public detail::QtValueType<16>
	{
	public:
		enum RenderHint
		{
			Antialiasing = 0x01,
			TextAntialiasing = 0x02,
			SmoothPixmapTransform = 0x04,
		};

		explicit QPainter(const QPixmap& target);
		explicit QPainter(const QWidget& target);
		~QPainter();

		QPainter(const QPainter&) = delete;
		QPainter& operator=(const QPainter&) = delete;

		void set_render_hint(RenderHint hint, bool on = true);
		void set_opacity(double opacity);
		void draw_pixmap(int x, int y, const QPixmap& pixmap);
		void draw_pixmap(const QRect& target, const QPixmap& pixmap);
	};
}
