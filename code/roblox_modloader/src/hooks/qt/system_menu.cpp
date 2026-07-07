#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qmenubar.hpp"
#include "RobloxModLoader/qt/qt_integration.hpp"

void* hooks::build_menu_bar_from_dom(void* out_menu_bar, void* dom, void* context)
{
	void* menu_bar = hooking::get_original<&hooks::build_menu_bar_from_dom>()(out_menu_bar, dom, context);

	if (rml::qt::QtIntegration* qt = rml::qt::QtIntegration::instance())
		qt->on_menu_bar_built(static_cast<rml::qt::QMenuBar*>(menu_bar));

	return menu_bar;
}

void hooks::qt_action_activate(void* self, const int event)
{
	if (static_cast<rml::qt::QAction::ActionEvent>(event) == rml::qt::QAction::Trigger)
	{
		if (const rml::qt::QtIntegration* qt = rml::qt::QtIntegration::instance())
			qt->on_action_triggered(static_cast<rml::qt::QAction*>(self));
	}

	hooking::get_original<&hooks::qt_action_activate>()(self, event);
}
