#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QPixmap;
	class QRect;
	class QWidget;
	
	class RML_EXPORT QPainter
	{
	public:
		explicit QPainter(const QPixmap& target);
		explicit QPainter(const QWidget& target);
		~QPainter();

		QPainter(const QPainter&) = delete;
		QPainter& operator=(const QPainter&) = delete;

		void set_opacity(double opacity);
		void draw_pixmap(int x, int y, const QPixmap& pixmap);
		void draw_pixmap(const QRect& target, const QPixmap& pixmap);

	private:
		alignas(void*) unsigned char m_storage[16]{};
		bool m_active = false;
	};
}
