#include "RobloxModLoader/qt/action_dispatcher.hpp"

#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/qt/qaction.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "qt_connect.hpp"

namespace rml::qt
{
	bool ActionDispatcher::ensure_hook()
	{
		std::scoped_lock lock(m_hook_mutex);
		if (m_hook_installed)
			return true;

		void* target = QAction::activate_address();
		if (!target)
			return false;

		Hooking::DetourHookHelper::add<&Hooks::qt_action_activate>("QAction::activate", target);
		m_hook_installed = true;
		LOG_INFO("[qt] hooked QAction::activate at {}", target);
		return true;
	}

	bool ActionDispatcher::is_hook_ready() const
	{
		std::scoped_lock lock(m_hook_mutex);
		return m_hook_installed;
	}

	void ActionDispatcher::connect(QAction* action, std::function<void()> callback)
	{
		if (!action || !callback)
			return;

		std::scoped_lock lock(m_callbacks_mutex);
		m_callbacks[action].push_back(std::move(callback));
	}

	void ActionDispatcher::connect_toggled(QAction* action, std::function<void(bool)> callback)
	{
		if (!action || !callback)
			return;

		bool first_connection = false;
		{
			std::scoped_lock lock(m_callbacks_mutex);
			auto& handlers = m_toggle_callbacks[action];
			first_connection = handlers.empty();
			handlers.push_back(std::move(callback));
		}

		if (!first_connection)
			return;

		static void* const signal = detail::widgets_export("?toggled@QAction@@QEAAX_N@Z");
		static const void* const meta = detail::widgets_export("?staticMetaObject@QAction@@2UQMetaObject@@B");
		detail::connect_function(action, signal, meta, [this, action](void** args) {
			if (args && args[1])
				dispatch_toggled(action, *static_cast<bool*>(args[1]));
		});
	}

	void ActionDispatcher::disconnect(QAction* action)
	{
		if (!action)
			return;

		std::scoped_lock lock(m_callbacks_mutex);
		m_callbacks.erase(action);
		m_toggle_callbacks.erase(action);
	}

	void ActionDispatcher::dispatch(QAction* action) const
	{
		std::vector<std::function<void()>> handlers;
		{
			std::scoped_lock lock(m_callbacks_mutex);
			if (const auto it = m_callbacks.find(action); it != m_callbacks.end())
				handlers = it->second;
		}

		for (const auto& handler : handlers)
		{
			try
			{
				handler();
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("[qt] action callback threw: {}", e.what());
			}
			catch (...)
			{
				LOG_ERROR("[qt] action callback threw a non-standard exception");
			}
		}
	}

	void ActionDispatcher::dispatch_toggled(QAction* action, const bool value) const
	{
		std::vector<std::function<void(bool)>> handlers;
		{
			std::scoped_lock lock(m_callbacks_mutex);
			if (const auto it = m_toggle_callbacks.find(action); it != m_toggle_callbacks.end())
				handlers = it->second;
		}

		for (const auto& handler : handlers)
		{
			try
			{
				handler(value);
			}
			catch (const std::exception& e)
			{
				LOG_ERROR("[qt] toggled callback threw: {}", e.what());
			}
			catch (...)
			{
				LOG_ERROR("[qt] toggled callback threw a non-standard exception");
			}
		}
	}
}
