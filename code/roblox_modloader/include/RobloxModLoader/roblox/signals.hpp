#pragma once

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
