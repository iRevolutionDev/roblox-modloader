#pragma once

#include "RobloxModLoader/qt/qmenu.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QString;
	
	class QMenuBar : public QWidget
	{
	public:
		[[nodiscard]] QMenu* addMenu(const QString& title);
	};
}
