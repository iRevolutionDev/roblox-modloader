#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace rml::qt
{
	class QAction;

	class RML_EXPORT ActionDispatcher
	{
	public:
		bool ensure_hook();

		[[nodiscard]] bool is_hook_ready() const;

		void connect(QAction* action, std::function<void()> callback);
		void connect_toggled(QAction* action, std::function<void(bool)> callback);
		void disconnect(QAction* action);
		void dispatch(QAction* action) const;

	private:
		void dispatch_toggled(QAction* action, bool value) const;

		mutable std::mutex m_callbacks_mutex;
		std::unordered_map<QAction*, std::vector<std::function<void()>>> m_callbacks;
		std::unordered_map<QAction*, std::vector<std::function<void(bool)>>> m_toggle_callbacks;

		mutable std::mutex m_hook_mutex;
		bool m_hook_installed = false;
	};
}
