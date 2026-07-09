#include "RobloxModLoader/roblox/signals.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "pointers.hpp"

namespace RBX::Signals
{
	void Connection::Deleter::operator()(Slot* slot) const noexcept
	{
		if (g_pointers && g_pointers->m_roblox_pointers.signal_slot_free)
			g_pointers->m_roblox_pointers.signal_slot_free(slot);
	}

	void Connection::disconnect() const
	{
		Slot* slot = m_slot.get();
		if (!slot)
			return;

		if (g_pointers && g_pointers->m_roblox_pointers.signal_disconnect)
			g_pointers->m_roblox_pointers.signal_disconnect(slot);
	}
}
