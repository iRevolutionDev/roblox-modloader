#include "RobloxModLoader/qt/qt_integration.hpp"

#include "RobloxModLoader/logger/logger.hpp"

RML_LOG_SCOPE("QtIntegration")

namespace rml::qt
{
	QtIntegration* QtIntegration::s_instance = nullptr;

	QtIntegration::QtIntegration() :
	    m_menu(m_dispatcher)
	{
		s_instance = this;
	}

	QtIntegration::~QtIntegration()
	{
		s_instance = nullptr;
	}

	ModsMenu& QtIntegration::menu()
	{
		return m_menu;
	}

	void QtIntegration::on_menu_bar_built(QMenuBar* menu_bar)
	{
		m_menu.rebuild(menu_bar);

		if (m_dispatch_timer)
			return;

		m_dispatch_timer = QTimer::create_owned();
		if (!m_dispatch_timer)
			return;

		m_dispatch_timer->setInterval(0);
		m_dispatch_timer->on_timeout([this] {
			drain_tasks();
		});
		m_dispatch_timer->start();
	}

	void QtIntegration::run_on_gui_thread(std::function<void()> task)
	{
		if (!task)
			return;

		const std::scoped_lock lock(m_tasks_mutex);
		m_tasks.push_back(std::move(task));
	}

	void QtIntegration::drain_tasks()
	{
		std::vector<std::function<void()>> pending;
		{
			const std::scoped_lock lock(m_tasks_mutex);
			pending.swap(m_tasks);
		}

		for (auto& task : pending)
		{
			try
			{
				task();
			}
			catch (const std::exception& error)
			{
				RML_ERROR("GUI task threw: {}", error.what());
			}
			catch (...)
			{
				RML_ERROR("GUI task threw an unknown exception");
			}
		}
	}

	void QtIntegration::on_action_triggered(QAction* action) const
	{
		m_dispatcher.dispatch(action);
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
