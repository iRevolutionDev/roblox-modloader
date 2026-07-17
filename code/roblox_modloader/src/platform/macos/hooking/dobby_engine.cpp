#include "RobloxModLoader/hooking/i_hook_engine.hpp"

#include <dobby.h>
#include <mutex>
#include <unordered_map>

namespace rml
{
	class DobbyEngine final : public IHookEngine
	{
	public:
		~DobbyEngine() override
		{
			std::scoped_lock lock(m_mutex);

			for (const auto& [target, hook] : m_hooks)
			{
				if (hook.installed)
					DobbyDestroy(target);
			}
		}

		std::expected<void, HookError> create(const std::string& name, void* target, void* detour, void** original) override
		{
			if (!target)
				return std::unexpected(HookError::from_status(name, 0, "hook target is null"));

			std::scoped_lock lock(m_mutex);
			m_hooks.insert_or_assign(target, Hook{name, detour, original, false, false});

			return {};
		}

		std::expected<void, HookError> remove(const std::string& name, void* target) override
		{
			std::scoped_lock lock(m_mutex);

			const auto it = m_hooks.find(target);
			if (it == m_hooks.end())
				return std::unexpected(HookError::from_status(name, address_of(target), "hook was never created"));

			if (it->second.installed)
			{
				if (const int result = DobbyDestroy(target); result != 0)
					return std::unexpected(HookError::from_status(name, address_of(target), describe(result)));
			}

			m_hooks.erase(it);

			return {};
		}

		std::expected<void, HookError> queue_enable(const std::string& name, void* target) override
		{
			return set_desired_state(name, target, true);
		}

		std::expected<void, HookError> queue_disable(const std::string& name, void* target) override
		{
			return set_desired_state(name, target, false);
		}

		void apply_queued() override
		{
			std::scoped_lock lock(m_mutex);

			for (auto& [target, hook] : m_hooks)
			{
				if (hook.desired == hook.installed)
					continue;

				if (hook.desired)
					hook.installed = DobbyHook(target, hook.detour, hook.original) == 0;
				else
					hook.installed = DobbyDestroy(target) != 0;
			}
		}

		void* resolve_thunk(void* target) const override
		{
			return target;
		}

	private:
		struct Hook
		{
			std::string name;
			void* detour;
			void** original;
			bool desired;
			bool installed;
		};

		static std::uintptr_t address_of(void* target)
		{
			return reinterpret_cast<std::uintptr_t>(target);
		}

		static std::string describe(const int result)
		{
			return "Dobby returned " + std::to_string(result);
		}

		std::expected<void, HookError> set_desired_state(const std::string& name, void* target, const bool enabled)
		{
			if (!target)
				return std::unexpected(HookError::from_status(name, 0, "hook target is null"));

			std::scoped_lock lock(m_mutex);

			const auto it = m_hooks.find(target);
			if (it == m_hooks.end())
				return std::unexpected(HookError::from_status(name, address_of(target), "hook was never created"));

			it->second.desired = enabled;

			return {};
		}

		std::mutex m_mutex;
		std::unordered_map<void*, Hook> m_hooks;
	};

	std::unique_ptr<IHookEngine> create_hook_engine()
	{
		return std::make_unique<DobbyEngine>();
	}
}
