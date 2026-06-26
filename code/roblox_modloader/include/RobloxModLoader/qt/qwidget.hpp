#pragma once

#include "RobloxModLoader/qt/qobject.hpp"

namespace rml::qt
{
	class QString;

	class QWidget : public QObject
	{
	public:
		QWidget() = default;

		explicit QWidget(void* instance) :
		    QObject(instance)
		{
		}

		void setWindowTitle(const QString& title) const;
		void setStyleSheet(const QString& style) const;
		void show() const;
	};
}
