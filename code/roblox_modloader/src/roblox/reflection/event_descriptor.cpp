#include "RobloxModLoader/roblox/reflection/event_descriptor.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/reflection/event.hpp"
#include "pointers.hpp"

namespace RBX::Reflection
{
	template<typename Fn>
	void walk_slots_locked(Signals::Signal* signal, Fn&& fn)
	{
		if (!signal)
			return;

		void* mtx = g_pointers && g_pointers->m_roblox_pointers.signal_mutex_get ? g_pointers->m_roblox_pointers.signal_mutex_get() : nullptr;
		if (mtx)
			_Mtx_lock(static_cast<_Mtx_t>(mtx));

		for (auto* slot = signal->head; slot; slot = slot->next)
		{
			if (slot->source)
				fn(slot);
		}

		if (mtx)
			_Mtx_unlock(static_cast<_Mtx_t>(mtx));
	}

	Signals::Signal* EventDescriptor::get_signal(EventSource* source) const
	{
		if (!source)
			return nullptr;

		const auto offset = static_cast<const EventDesc*>(this)->signal;
		return *reinterpret_cast<Signals::Signal**>(reinterpret_cast<std::uint8_t*>(source) + offset);
	}

	std::vector<Signals::Connection> EventDescriptor::snapshot_connections(EventSource* source) const
	{
		std::vector<Signals::Connection> out;
		walk_slots_locked(get_signal(source), [&](Signals::Slot* slot) {
			out.push_back(Signals::Connection::observe(slot));
		});
		return out;
	}
}
