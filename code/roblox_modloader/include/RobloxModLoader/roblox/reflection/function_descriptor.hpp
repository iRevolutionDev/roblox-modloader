#pragma once

#include "member.hpp"
#include "type.hpp"

#include <cstdint>

class Function;

namespace RBX::Reflection
{
	class FunctionDescriptor : public MemberDescriptor
	{
	public:
		typedef Function ConstMember;
		typedef Function Member;

		enum Kind : std::uint32_t
		{
			Kind_Default = 0,
			Kind_Custom  = 1,
		};

	protected:
		SignatureDescriptor signature;
		Kind kind;

	public:
		const SignatureDescriptor& get_signature() const
		{
			return signature;
		}

		Kind get_kind() const
		{
			return kind;
		}

		template<typename T = uintptr_t>
		T get_bound_function()
		{
			return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + 0x88);
		}

		template<typename Return>
		Return invoke()
		{
			return get_bound_function<Return>();
		}

		template<typename Return = std::uint64_t, typename This>
		Return invoke(This* this_ptr)
		{
			return invoke<Return(__fastcall*)(This*)>()(this_ptr);
		}

		template<typename Return = std::uint64_t, typename This, typename Arg0>
		Return invoke(This* this_ptr, Arg0 a0)
		{
			return invoke<Return(__fastcall*)(This*, Arg0)>()(this_ptr, a0);
		}

		template<typename Return = std::uint64_t, typename This, typename Arg0, typename Arg1>
		Return invoke(This* this_ptr, Arg0 a0, Arg1 a1)
		{
			return invoke<Return(__fastcall*)(This*, Arg0, Arg1)>()(this_ptr, a0, a1);
		}

		template<typename Return = std::uint64_t, typename This, typename Arg0, typename Arg1, typename Arg2>
		Return invoke(This* this_ptr, Arg0 a0, Arg1 a1, Arg2 a2)
		{
			return invoke<Return(__fastcall*)(This*, Arg0, Arg1, Arg2)>()(this_ptr, a0, a1, a2);
		}

		template<typename Return = std::uint64_t, typename This, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
		Return invoke(This* this_ptr, Arg0 a0, Arg1 a1, Arg2 a2, Arg3 a3)
		{
			return invoke<Return(__fastcall*)(This*, Arg0, Arg1, Arg2, Arg3)>()(this_ptr, a0, a1, a2, a3);
		}
	};
}