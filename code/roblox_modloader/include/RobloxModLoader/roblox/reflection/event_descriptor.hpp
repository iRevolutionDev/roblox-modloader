#pragma once

#include "RobloxModLoader/roblox/signals.hpp"
#include "member.hpp"
#include "type.hpp"

#include <vector>

namespace RBX::Reflection
{
	typedef std::vector<Variant> EventArguments;

	class GenericSlotWrapper
	{
	public:
		virtual ~GenericSlotWrapper() = default;
		virtual bool use_submit_task()
		{
			return false;
		}
		virtual void deliver(const EventArguments& args) = 0;
		virtual void reserved_slot_3()
		{
		}
		virtual EventArguments* build_args(EventArguments& out)
		{
			return ::new (&out) EventArguments();
		}

		virtual void reserved_slot_5()
		{
		}
		virtual void reserved_slot_6()
		{
		}
	};

	class Event;

	class EventDescriptor : public MemberDescriptor
	{
	public:
		typedef Event ConstMember;
		typedef Event Member;

	protected:
		SignatureDescriptor signature;

		bool operator==(const EventDescriptor& other) const
		{
			return this == &other;
		}
		bool operator!=(const EventDescriptor& other) const
		{
			return this != &other;
		}

	public:
		const SignatureDescriptor& get_signature() const
		{
			return signature;
		}

		virtual Signals::Connection connect(EventSource* source, std::shared_ptr<GenericSlotWrapper> wrapper) const = 0;

		[[nodiscard]] virtual bool is_scriptable() const
		{
			return true;
		}
		[[nodiscard]] bool is_public() const
		{
			return is_scriptable();
		}

		[[nodiscard]] virtual bool isBroadcast() const
		{
			return false;
		}
		[[nodiscard]] virtual bool unk_0x40() const
		{
			return false;
		}
		[[nodiscard]] virtual bool unk_0x80() const
		{
			return false;
		}
		virtual void fire_event(EventSource* source, const EventArguments& args) const = 0;
		virtual void send_event(EventSource* source, const EventArguments& args) const;
		virtual void disconnect_all(EventSource* source) const = 0;
	};
}