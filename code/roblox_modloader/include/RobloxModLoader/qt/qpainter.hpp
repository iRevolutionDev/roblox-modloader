#pragma once

#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

namespace rml::qt
{
	class QPixmap;
	class QRect;
	class QWidget;
	
	class RML_EXPORT QPainter
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

	private:
		static constexpr std::size_t STORAGE_SIZE = 16;

		alignas(void*) unsigned char m_storage[STORAGE_SIZE]{};
		bool m_active = false;

		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
