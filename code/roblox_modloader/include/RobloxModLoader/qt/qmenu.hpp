#pragma once

#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QString;
	class QIcon;

	class RML_EXPORT QMenu : public QWidget
	{
	public:
		[[nodiscard]] QAction* addAction(const QString& text);
		[[nodiscard]] QMenu* addMenu(const QString& title);
		QAction* addSeparator();
		void clear();
		void setIcon(const QIcon& icon);
		[[nodiscard]] QAction* menuAction() const;
	};
}
