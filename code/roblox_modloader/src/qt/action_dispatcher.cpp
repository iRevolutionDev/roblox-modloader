#include "RobloxModLoader/qt/action_dispatcher.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/qt/qaction.hpp"

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

		hooking::detour_hook_helper::add<&hooks::qt_action_activate>("QAction::activate", target);
		m_hook_installed = true;
		LOG_INFO("[qt] hooked QAction::activate at {}", target);
		return true;
	}

	bool ActionDispatcher::is_hook_ready() const
	{
		std::scoped_lock lock(m_hook_mutex);
		return m_hook_installed;
	}

	void ActionDispatcher::connect(void* action, std::function<void()> callback)
	{
		if (!action || !callback)
			return;

		std::scoped_lock lock(m_callbacks_mutex);
		m_callbacks[action].push_back(std::move(callback));
	}

	void ActionDispatcher::disconnect(void* action)
	{
		if (!action)
			return;

		std::scoped_lock lock(m_callbacks_mutex);
		m_callbacks.erase(action);
	}

	void ActionDispatcher::dispatch(const void* action) const
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
}
