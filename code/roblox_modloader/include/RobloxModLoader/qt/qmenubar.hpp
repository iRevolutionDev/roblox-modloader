#pragma once

#include "RobloxModLoader/qt/qmenu.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QString;

	class QMenuBar : public QWidget
	{
	public:
		QMenuBar() = default;

		explicit QMenuBar(void* instance) :
		    QWidget(instance)
		{
		}

		[[nodiscard]] QMenu addMenu(const QString& title) const;
	};
}
