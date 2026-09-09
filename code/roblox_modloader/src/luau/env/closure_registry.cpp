#include "RobloxModLoader/luau/env/closure_registry.hpp"

RML_LOG_SCOPE("ClosureRegistry");

namespace rml::luau
{
	void ClosureRegistry::register_hook(Closure* target, HookRecord record)
	{
		m_hooks.insert_or_assign(target, std::move(record));
	}

	void ClosureRegistry::unregister_hook(Closure* target) { m_hooks.erase(target); }

	bool ClosureRegistry::is_hooked(Closure* target) const noexcept { return m_hooks.contains(target); }

	const HookRecord* ClosureRegistry::hook_for(Closure* target) const noexcept
	{
		const auto entry = m_hooks.find(target);

		return entry == m_hooks.end() ? nullptr : &entry->second;
	}

	void ClosureRegistry::register_wrapper(Closure* wrapper, vm::Ref original, std::string owner)
	{
		m_wrappers.insert_or_assign(wrapper, WrapperRecord{.original = std::move(original), .owner = std::move(owner)});
	}

	void ClosureRegistry::unregister_wrapper(Closure* wrapper) { m_wrappers.erase(wrapper); }

	bool ClosureRegistry::is_wrapper(Closure* wrapper) const noexcept { return m_wrappers.contains(wrapper); }

	const vm::Ref* ClosureRegistry::wrapped_original(Closure* wrapper) const noexcept
	{
		const auto entry = m_wrappers.find(wrapper);

		return entry == m_wrappers.end() ? nullptr : &entry->second.original;
	}

	void ClosureRegistry::protect(Closure* closure, std::string owner)
	{
		m_protected.insert_or_assign(closure, std::move(owner));
	}

	bool ClosureRegistry::is_protected(Closure* closure) const noexcept { return m_protected.contains(closure); }

	void ClosureRegistry::adopt(Closure* closure, std::string owner)
	{
		m_ours.insert_or_assign(closure, std::move(owner));
	}

	bool ClosureRegistry::is_ours(Closure* closure) const noexcept
	{
		return m_ours.contains(closure) || m_wrappers.contains(closure);
	}

	std::vector<OwnedHook> ClosureRegistry::take_hooks_of(const std::string_view owner)
	{
		std::vector<OwnedHook> taken;

		for (auto it = m_hooks.begin(); it != m_hooks.end();)
		{
			if (it->second.owner != owner)
			{
				++it;
				continue;
			}

			taken.push_back(OwnedHook{.target = it->first, .record = std::move(it->second)});
			it = m_hooks.erase(it);
		}

		return taken;
	}

	void ClosureRegistry::release_owner(const std::string_view owner)
	{
		std::erase_if(m_wrappers, [owner](const auto& entry) { return entry.second.owner == owner; });
		std::erase_if(m_protected, [owner](const auto& entry) { return entry.second == owner; });
		std::erase_if(m_ours, [owner](const auto& entry) { return entry.second == owner; });
	}

	void ClosureRegistry::clear() noexcept
	{
		m_hooks.clear();
		m_wrappers.clear();
		m_protected.clear();
		m_ours.clear();
	}

	void ClosureRegistry::release() noexcept
	{
		for (auto& record : m_hooks | std::views::values)
		{
			record.original.release();
			record.replacement.release();
		}

		for (auto& record : m_wrappers | std::views::values)
		{
			record.original.release();
		}

		clear();
	}
}
