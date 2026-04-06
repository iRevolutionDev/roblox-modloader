#pragma once

#include "member.hpp"
#include "type.hpp"

namespace RBX::Reflection
{
	class Event;

	class EventDescriptor : public MemberDescriptor
	{
	public:
		typedef Event ConstMember;
		typedef Event Member;

	protected:
		SignatureDescriptor signature;

	public:
		const SignatureDescriptor& get_signature() const
		{
			return signature;
		}

		virtual bool is_scriptable() const
		{
			return true;
		}
		bool is_public() const
		{
			return is_scriptable();
		}
	};
}