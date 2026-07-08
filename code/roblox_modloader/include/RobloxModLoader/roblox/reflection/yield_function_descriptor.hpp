#pragma once

#include "function_descriptor.hpp"
#include "member.hpp"
#include "type.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstddef>

namespace RBX::Reflection
{
	class YieldFunction;

	class YieldFunctionDescriptor : public MemberDescriptor
	{
	public:
		typedef YieldFunction ConstMember;
		typedef YieldFunction Member;

		struct Context
		{
			void* state;
			void* control_block;
		};

		using ResumeCallback = void (*)(void* continuation, Variant* result);
		using ErrorCallback = void (*)(void* continuation, Variant* message);

		virtual void execute(DescribedBase* instance, FunctionDescriptor::Arguments& arguments, Context context, ResumeCallback resume, ErrorCallback error) const = 0;

		[[nodiscard]] const SignatureDescriptor& get_signature() const
		{
			return signature;
		}

	protected:
		std::byte pad[0x8];
		SignatureDescriptor signature;

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(YieldFunctionDescriptor, 0x78);
			RML_ASSERT_LAYOUT_OFFSET(YieldFunctionDescriptor, pad, 0x40);
			RML_ASSERT_LAYOUT_OFFSET(YieldFunctionDescriptor, signature, 0x48);
		RML_LAYOUT_GUARD_END()
	};
}
