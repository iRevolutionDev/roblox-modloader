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

	void ClosureRegistry::register_wrapper(Closure* wrapper, vm::Ref original)
	{
		m_wrappers.insert_or_assign(wrapper, std::move(original));
	}

	void ClosureRegistry::unregister_wrapper(Closure* wrapper) { m_wrappers.erase(wrapper); }

	bool ClosureRegistry::is_wrapper(Closure* wrapper) const noexcept { return m_wrappers.contains(wrapper); }

	const vm::Ref* ClosureRegistry::wrapped_original(Closure* wrapper) const noexcept
	{
		const auto entry = m_wrappers.find(wrapper);

		return entry == m_wrappers.end() ? nullptr : &entry->second;
	}

	void ClosureRegistry::protect(Closure* closure) { m_protected.insert(closure); }

	bool ClosureRegistry::is_protected(Closure* closure) const noexcept { return m_protected.contains(closure); }

	void ClosureRegistry::adopt(Closure* closure) { m_ours.insert(closure); }

	bool ClosureRegistry::is_ours(Closure* closure) const noexcept
	{
		return m_ours.contains(closure) || m_wrappers.contains(closure);
	}

	void ClosureRegistry::clear() noexcept
	{
		m_hooks.clear();
		m_wrappers.clear();
		m_protected.clear();
		m_ours.clear();
	}
}
