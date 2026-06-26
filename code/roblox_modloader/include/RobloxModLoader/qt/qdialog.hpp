#pragma once

#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QDialog : public QWidget
	{
	public:
		QDialog() = default;

		explicit QDialog(void* instance) :
		    QWidget(instance)
		{
		}

		int exec() const;
	};
}
