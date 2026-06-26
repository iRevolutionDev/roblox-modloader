#include "RobloxModLoader/qt/qt_integration.hpp"

namespace rml::qt
{
	QtIntegration::QtIntegration() :
	    m_menu(m_dispatcher)
	{
		s_instance = this;
	}

	QtIntegration::~QtIntegration()
	{
		s_instance = nullptr;
	}

	bool QtIntegration::ensure_action_hook()
	{
		return m_dispatcher.ensure_hook();
	}

	bool QtIntegration::is_action_hook_ready() const
	{
		return m_dispatcher.is_hook_ready();
	}

	QtIntegration* QtIntegration::instance()
	{
		return s_instance;
	}
}
