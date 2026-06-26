#pragma once

#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	class QPushButton : public QWidget
	{
	public:
		QPushButton() = default;
		
		explicit QPushButton(void* instance) :
		    QWidget(instance)
		{
		}
	};
}
