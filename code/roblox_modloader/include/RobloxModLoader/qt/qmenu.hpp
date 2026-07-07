#pragma once

#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QString;
	
	class QMenu : public QWidget
	{
	public:
		[[nodiscard]] QAction* addAction(const QString& text);
		QAction* addSeparator();
		void clear();
	};
}
