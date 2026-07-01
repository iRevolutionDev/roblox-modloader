#pragma once

#include "function_descriptor.hpp"
#include "member.hpp"
#include "type.hpp"

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
	};
}
