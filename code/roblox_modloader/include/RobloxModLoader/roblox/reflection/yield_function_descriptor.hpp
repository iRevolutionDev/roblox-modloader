#pragma once

#include "member.hpp"
#include "type.hpp"

namespace RBX::Reflection
{
	class YieldFunction;

	class YieldFunctionDescriptor : public MemberDescriptor
	{
	public:
		typedef YieldFunction ConstMember;
		typedef YieldFunction Member;

	protected:
		SignatureDescriptor signature;

	public:
		[[nodiscard]] const SignatureDescriptor& get_signature() const
		{
			return signature;
		}
	};
}