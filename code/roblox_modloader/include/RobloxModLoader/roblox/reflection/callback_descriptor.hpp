#pragma once

#include "member.hpp"
#include "type.hpp"

namespace RBX::Reflection
{
	class Callback;

	class CallbackDescriptor : public MemberDescriptor
	{
	public:
		typedef Callback ConstMember;
		typedef Callback Member;

	protected:
		SignatureDescriptor signature;
		bool async_flag;

	public:
		const SignatureDescriptor& get_signature() const
		{
			return signature;
		}

		bool is_async() const
		{
			return async_flag;
		}
	};
}