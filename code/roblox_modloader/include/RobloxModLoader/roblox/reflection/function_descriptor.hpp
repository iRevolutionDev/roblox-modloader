#pragma once

#include "RobloxModLoader/memory/foreign_call.hpp"
#include "RobloxModLoader/roblox/util/G3DCore.h"
#include "RobloxModLoader/util/layout_assert.hpp"
#include "enum_descriptor.hpp"
#include "member.hpp"
#include "type.hpp"

#include <cstring>
#include <utility>

class Function;

namespace RBX::Reflection
{
	class DescribedBase;

	class FunctionDescriptor : public MemberDescriptor
	{
	public:
		typedef Function ConstMember;
		typedef Function Member;

		class Arguments
		{
		public:
			uint64_t return_value;

			virtual size_t size() const = 0;

			virtual bool get_varint(int index, Variant& value) const = 0;
			virtual bool get_bool(int index, bool& value) const = 0;
			virtual bool get_long(int index, long& value) const = 0;
			virtual bool get_double(int index, double& value) const = 0;
			virtual bool get_string(int index, std::string& value) const = 0;
			virtual bool get_vector3_int16(int index, Vector3int16& value) const = 0;
			virtual bool get_region3_int16(int index, void* value) const = 0;
			virtual bool get_vector3(int index, Vector3& value) const = 0;
			virtual bool get_region3(int index, void* value) const = 0;
			virtual bool get_rect(int index, Rect2D& value) const = 0;
			virtual bool get_object(int index, std::shared_ptr<DescribedBase>& value) const = 0;
			virtual bool get_enum(int index, const EnumDescriptor& desc, int& value) const = 0;

			// Generic type
			virtual void* get(int index) const = 0;
		};

		enum Kind : std::uint32_t
		{
			Default = 0,
			Custom = 1,
		};

		virtual int invoke_lua(DescribedBase* instance, lua_State*) const
		{
			return 0;
		}

		virtual uint64_t invoke(DescribedBase* instance, Arguments& arguments, std::intptr_t argument_base) const = 0;

		[[nodiscard]] const SignatureDescriptor& get_signature() const noexcept
		{
			return signature;
		}

		[[nodiscard]] Kind get_kind() const noexcept
		{
			return kind;
		}

		template<typename T>
		[[nodiscard]] T* native_func_ptr() const noexcept
		{
			return reinterpret_cast<T*>(invoke_func_ptr);
		}

		std::byte pad[0x8];
		SignatureDescriptor signature;
		Kind kind;
		void* invoke_func_ptr;
		std::intptr_t bound_this_delta;

	private:
		RML_LAYOUT_GUARD_BEGIN()
		RML_ASSERT_LAYOUT_SIZE(FunctionDescriptor, 0x90);
		RML_ASSERT_LAYOUT_OFFSET(FunctionDescriptor, pad, 0x40);
		RML_ASSERT_LAYOUT_OFFSET(FunctionDescriptor, signature, 0x48);
		RML_ASSERT_LAYOUT_OFFSET(FunctionDescriptor, kind, 0x78);
		RML_ASSERT_LAYOUT_OFFSET(FunctionDescriptor, invoke_func_ptr, 0x80);
		RML_ASSERT_LAYOUT_OFFSET(FunctionDescriptor, bound_this_delta, 0x88);
		RML_LAYOUT_GUARD_END()
	};

	class Function
	{
	protected:
		const FunctionDescriptor* m_descriptor;
		DescribedBase* m_instance;

		template<std::size_t>
		using arg_slot = void*;

		static constexpr std::size_t engine_return_bytes = 64;

#if !defined(RML_WINDOWS)
		static constexpr std::intptr_t itanium_virtual_flag = 1;

		[[nodiscard]] void* resolve_target(DescribedBase*& self) const noexcept
		{
			const auto adjustment = m_descriptor->bound_this_delta;
			auto* const base = reinterpret_cast<std::byte*>(self) + (adjustment >> 1);

			self = reinterpret_cast<DescribedBase*>(base);

			if ((adjustment & itanium_virtual_flag) == 0)
				return m_descriptor->invoke_func_ptr;

			const auto vtable_offset = reinterpret_cast<std::intptr_t>(m_descriptor->invoke_func_ptr);
			auto* const vtable = *reinterpret_cast<std::byte* const*>(base);

			return *reinterpret_cast<void* const*>(vtable + vtable_offset);
		}
#endif

		template<std::size_t... I>
		uint64_t invoke_fixed(FunctionDescriptor::Arguments& arguments, const bool indirect_result, std::index_sequence<I...>) const
		{
#if defined(RML_WINDOWS)
			(void)indirect_result;
			using fn_t = uint64_t(DescribedBase*, uint64_t&, arg_slot<I>...);
			return m_descriptor->native_func_ptr<fn_t>()(m_instance, arguments.return_value, arguments.get(static_cast<int>(I) + 1)...);
#else
			DescribedBase* self = m_instance;
			void* const target = resolve_target(self);

			if (!indirect_result)
			{
				using direct_fn_t = uint64_t(DescribedBase*, arg_slot<I>...);
				return reinterpret_cast<direct_fn_t*>(target)(self, arguments.get(static_cast<int>(I) + 1)...);
			}

			using Slot = rml::memory::detail::IndirectResult<engine_return_bytes>;
			using fn_t = Slot(DescribedBase*, arg_slot<I>...);

			const Slot value = reinterpret_cast<fn_t*>(target)(self, arguments.get(static_cast<int>(I) + 1)...);

			std::memcpy(&arguments.return_value, value.m_storage, engine_return_bytes);
			return arguments.return_value;
#endif
		}

	public:
		Function(const FunctionDescriptor& descriptor, DescribedBase* instance) :
		    m_descriptor(&descriptor),
		    m_instance(instance)
		{
		}

		Function(const Function& other) = default;

		Function& operator=(const Function& other) = default;

		[[nodiscard]] const Name& name() const
		{
			return m_descriptor->name;
		}

		[[nodiscard]] const FunctionDescriptor* descriptor() const
		{
			return m_descriptor;
		}

		uint64_t invoke(FunctionDescriptor::Arguments& arguments, const bool indirect_result) const
		{
			switch (arguments.size())
			{
			case 0: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<0>{});
			case 1: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<1>{});
			case 2: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<2>{});
			case 3: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<3>{});
			case 4: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<4>{});
			case 5: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<5>{});
			case 6: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<6>{});
			case 7: return invoke_fixed(arguments, indirect_result, std::make_index_sequence<7>{});
			default: throw std::runtime_error("Too many arguments");
			}
		}
	};
}