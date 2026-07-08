#pragma once

#include "RobloxModLoader/util/layout_assert.hpp"
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

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(CallbackDescriptor, 0x78);
			RML_ASSERT_LAYOUT_OFFSET(CallbackDescriptor, signature, 0x40);
			RML_ASSERT_LAYOUT_OFFSET(CallbackDescriptor, async_flag, 0x70);
		RML_LAYOUT_GUARD_END()
	};
}