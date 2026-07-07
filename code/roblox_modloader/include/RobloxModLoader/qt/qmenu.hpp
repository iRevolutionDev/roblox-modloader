#pragma once

#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QString;

	class RML_EXPORT QMenu : public QWidget
	{
	public:
		[[nodiscard]] QAction* addAction(const QString& text);
		QAction* addSeparator();
		void clear();
	};
}
