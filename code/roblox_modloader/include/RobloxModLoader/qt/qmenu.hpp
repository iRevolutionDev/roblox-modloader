#pragma once

#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QString;

	class QMenu : public QWidget
	{
	public:
		QMenu() = default;

		explicit QMenu(void* instance) :
		    QWidget(instance)
		{
		}

		[[nodiscard]] QAction addAction(const QString& text) const;

		QAction addSeparator() const;
	};
}
