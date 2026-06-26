#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace rml::qt
{
	class ActionDispatcher
	{
	public:
		bool ensure_hook();

		[[nodiscard]] bool is_hook_ready() const;

		void connect(void* action, std::function<void()> callback);
		void disconnect(void* action);
		void dispatch(const void* action) const;

	private:
		mutable std::mutex m_callbacks_mutex;
		std::unordered_map<void*, std::vector<std::function<void()>>> m_callbacks;

		mutable std::mutex m_hook_mutex;
		bool m_hook_installed = false;
	};
}
