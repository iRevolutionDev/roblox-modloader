#pragma once

#include "RobloxModLoader/qt/qt_owned.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

#include <string_view>

namespace rml::qt
{
	class QString;

	class RML_EXPORT QLabel : public QWidget
	{
	public:
		enum Alignment
		{
			AlignLeft = 0x0001,
			AlignRight = 0x0002,
			AlignHCenter = 0x0004,
			AlignTop = 0x0020,
			AlignBottom = 0x0040,
			AlignVCenter = 0x0080,
			AlignCenter = AlignHCenter | AlignVCenter,
		};

		[[nodiscard]] static QLabel* create(std::string_view text, QWidget* parent);
		[[nodiscard]] static QtOwned<QLabel> create_owned(std::string_view text);

		static void destroy(QLabel* label);

		void setText(std::string_view text);
		void setAlignment(int alignment);
	};
}
