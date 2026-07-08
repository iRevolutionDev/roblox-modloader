#pragma once

#include "RobloxModLoader/qt/qobject.hpp"

namespace rml::qt
{
	class QIcon;

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
		void setIcon(const QIcon& icon);

		[[nodiscard]] static void* activate_address();
	};
}
