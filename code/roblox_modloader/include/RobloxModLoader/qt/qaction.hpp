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
		
		enum MenuRole
		{
			NoRole = 0,
			TextHeuristicRole = 1,
			ApplicationSpecificRole = 2,
			AboutQtRole = 3,
			AboutRole = 4,
			PreferencesRole = 5,
			QuitRole = 6,
		};

		void setCheckable(bool checkable);
		void setChecked(bool checked);
		[[nodiscard]] bool isChecked() const;
		void setIcon(const QIcon& icon);
		void setMenuRole(MenuRole role);

		[[nodiscard]] static void* activate_address();
	};
}
