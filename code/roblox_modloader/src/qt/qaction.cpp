#include "RobloxModLoader/qt/qaction.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	void* QAction::activate_address()
	{
		return detail::widgets_export("?activate@QAction@@QEAAXW4ActionEvent@1@@Z");
	}
}
