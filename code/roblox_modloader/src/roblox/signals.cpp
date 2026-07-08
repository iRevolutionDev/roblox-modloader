#include "RobloxModLoader/roblox/signals.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace RBX::Signals
{
	void Connection::add_weak_ref() const noexcept
	{
		if (weak_slot)
			_InterlockedIncrement(&weak_slot->weak);
	}

	void Connection::release_weak_ref() noexcept
	{
		if (!weak_slot)
			return;

		_InterlockedExchangeAdd(&weak_slot->weak, -1);
		weak_slot = nullptr;
	}

	Connection& Connection::operator=(const Connection& other) noexcept
	{
		if (this == &other || weak_slot == other.weak_slot)
			return *this;

		auto* previous = weak_slot;
		weak_slot = other.weak_slot;
		add_weak_ref();

		if (previous)
			_InterlockedExchangeAdd(&previous->weak, -1);

		return *this;
	}

	void Connection::disconnect() const
	{
		auto* slot = weak_slot;
		if (!slot)
			return;

		if (_InterlockedExchangeAdd(&slot->strong, -1) == 1)
		{
			if ((slot->flags & 4) != 0 && slot->destroy_fn)
				slot->destroy_fn(slot);
		}
	}

	bool Connection::connected() const
	{
		return weak_slot && weak_slot->strong > 0;
	}
}
