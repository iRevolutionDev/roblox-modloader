#pragma once

#include "RobloxModLoader/util/intrusive_weak_ptr.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>

namespace RBX::Signals
{
	struct Slot
	{
		volatile long strong;
		volatile long weak;
		void* fire_fn;
		Slot* next;
		std::uint64_t flags;
		void* source;
		void(__fastcall* destroy_fn)(Slot*);
		void* wrapper_ptr;
		void* wrapper_rep;

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_SIZE(Slot, 0x40);
		RML_ASSERT_LAYOUT_OFFSET(Slot, strong, 0x0);
		RML_ASSERT_LAYOUT_OFFSET(Slot, weak, 0x4);
		RML_ASSERT_LAYOUT_OFFSET(Slot, fire_fn, 0x8);
		RML_ASSERT_LAYOUT_OFFSET(Slot, next, 0x10);
		RML_ASSERT_LAYOUT_OFFSET(Slot, flags, 0x18);
		RML_ASSERT_LAYOUT_OFFSET(Slot, source, 0x20);
		RML_ASSERT_LAYOUT_OFFSET(Slot, destroy_fn, 0x28);
		RML_ASSERT_LAYOUT_OFFSET(Slot, wrapper_ptr, 0x30);
		RML_ASSERT_LAYOUT_OFFSET(Slot, wrapper_rep, 0x38);
		RML_LAYOUT_GUARD_END()
	};

	struct Signal
	{
		volatile long strong;
		volatile long weak;
		Slot* head;

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_OFFSET(Signal, strong, 0x0);
		RML_ASSERT_LAYOUT_OFFSET(Signal, weak, 0x4);
		RML_ASSERT_LAYOUT_OFFSET(Signal, head, 0x8);
		RML_LAYOUT_GUARD_END()
	};

	class Connection
	{
	public:
		using Slot = Slot;

		struct Deleter
		{
			void operator()(Slot* slot) const noexcept;
		};

		Connection() noexcept = default;

		explicit Connection(Slot* slot) noexcept :
		    m_slot(slot)
		{
		}

		Connection(const Connection&) noexcept = default;
		Connection(Connection&&) noexcept = default;
		Connection& operator=(const Connection&) noexcept = default;
		Connection& operator=(Connection&&) noexcept = default;
		~Connection() = default;

		[[nodiscard]] static Connection observe(Slot* slot) noexcept
		{
			if (slot)
				_InterlockedIncrement(&slot->weak);
			return Connection(slot);
		}

		void disconnect() const;

		[[nodiscard]] Slot* raw_slot() const noexcept
		{
			return m_slot.get();
		}

		[[nodiscard]] bool connected() const
		{
			return m_slot.alive();
		}

		bool operator==(const Connection& other) const
		{
			return m_slot == other.m_slot;
		}
		bool operator!=(const Connection& other) const
		{
			return m_slot != other.m_slot;
		}

	private:
		rml::utils::intrusive_weak_ptr<Slot, Deleter> m_slot;
	};

	static_assert(sizeof(Connection) == sizeof(void*), "Connection must stay a single-pointer handle");
}
