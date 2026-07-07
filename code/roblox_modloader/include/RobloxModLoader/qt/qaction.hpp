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

		void setCheckable(bool checkable);
		void setChecked(bool checked);
		[[nodiscard]] bool isChecked() const;

		[[nodiscard]] static void* activate_address();
	};
}
