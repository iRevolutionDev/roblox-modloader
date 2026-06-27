#pragma once

#include "RobloxModLoader/qt/qobject.hpp"

namespace rml::qt
{
	class QAction : public QObject
	{
	public:
		enum ActionEvent
		{
			Trigger = 0,
			Hover = 1,
		};

		[[nodiscard]] static void* activate_address();
	};
}
