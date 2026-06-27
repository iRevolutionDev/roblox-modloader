#include "RobloxModLoader/qt/qt_module.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/symbol_resolver.hpp"

namespace rml::qt::detail
{
	void* core_export(const char* mangled)
	{
		return memory::resolve_export_exact(memory::loaded_module({"Qt5Core.dll", "Qt5Cored.dll", "QtCore"}), mangled);
	}

	void* widgets_export(const char* mangled)
	{
		return memory::resolve_export_exact(memory::loaded_module({"Qt5Widgets.dll", "Qt5Widgetsd.dll", "QtWidgets"}), mangled);
	}

	void* gui_export(const char* mangled)
	{
		return memory::resolve_export_exact(memory::loaded_module({"Qt5Gui.dll", "Qt5Guid.dll", "QtGui"}), mangled);
	}
}
