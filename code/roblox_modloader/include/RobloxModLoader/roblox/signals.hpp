#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>

namespace RBX::Signals
{
	// IK ITS HARDCODED BUT WORKS, I'LL WORK ON IT LATER
	// I NEED TO REVERSE MORE TO KNOW WHEN THE ENGINE INCREMENT/DECREMENT THE STRONG/WEAK COUNT

	class Connection
	{
	public:
		struct Slot
		{
			volatile long strong;
			volatile long weak;
			void* fire_fn;

		private:
			void* unk_0x10;

		public:
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
				RML_ASSERT_LAYOUT_OFFSET(Slot, unk_0x10, 0x10);
				RML_ASSERT_LAYOUT_OFFSET(Slot, flags, 0x18);
				RML_ASSERT_LAYOUT_OFFSET(Slot, source, 0x20);
				RML_ASSERT_LAYOUT_OFFSET(Slot, destroy_fn, 0x28);
				RML_ASSERT_LAYOUT_OFFSET(Slot, wrapper_ptr, 0x30);
				RML_ASSERT_LAYOUT_OFFSET(Slot, wrapper_rep, 0x38);
			RML_LAYOUT_GUARD_END()
		};

		Connection() noexcept :
		    weak_slot(nullptr)
		{
		}

		explicit Connection(Slot* slot) noexcept :
		    weak_slot(slot)
		{
		}

		Connection(const Connection& other) noexcept :
		    weak_slot(other.weak_slot)
		{
			add_weak_ref();
		}

		Connection& operator=(const Connection& other) noexcept;

		~Connection()
		{
			release_weak_ref();
		}

		void disconnect() const;
		[[nodiscard]] bool connected() const;

		bool operator==(const Connection& other) const
		{
			return weak_slot == other.weak_slot;
		}
		bool operator!=(const Connection& other) const
		{
			return weak_slot != other.weak_slot;
		}

	private:
		void add_weak_ref() const noexcept;
		void release_weak_ref() noexcept;

		Slot* weak_slot;
	};
}
