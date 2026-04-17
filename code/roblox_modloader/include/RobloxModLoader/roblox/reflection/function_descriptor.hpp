#pragma once

#include "RobloxModLoader/roblox/util/G3DCore.h"
#include "enum_descriptor.hpp"
#include "member.hpp"
#include "type.hpp"

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

			virtual bool get_varint(int index, Variant& value) const                        = 0;
			virtual bool get_bool(int index, bool& value) const                             = 0;
			virtual bool get_long(int index, long& value) const                             = 0;
			virtual bool get_double(int index, double& value) const                         = 0;
			virtual bool get_string(int index, std::string& value) const                    = 0;
			virtual bool get_vector3_int16(int index, Vector3int16& value) const            = 0;
			virtual bool get_region3_int16(int index, void* value) const                    = 0;
			virtual bool get_vector3(int index, Vector3& value) const                       = 0;
			virtual bool get_region3(int index, void* value) const                          = 0;
			virtual bool get_rect(int index, Rect2D& value) const                           = 0;
			virtual bool get_object(int index, std::shared_ptr<DescribedBase>& value) const = 0;
			virtual bool get_enum(int index, const EnumDescriptor& desc, int& value) const  = 0;

			// Generic type
			virtual void* get(int index) const = 0;
		};

		enum Kind : std::uint32_t
		{
			Default = 0,
			Custom  = 1,
		};

		virtual int invoke_lua(DescribedBase* instance, lua_State*) const
		{
			return 0;
		}

		// Roblox interns totally fucked up this function, so we have to call it directly instead of through the vtable. ;-(
		// everything would be easier if they had chosen agnostic
		virtual void invoke(DescribedBase* instance, Arguments& arguments, lua_State* L) const = 0;

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
			return static_cast<T*>(invoke_func_ptr);
		}

	protected:
		SignatureDescriptor signature;
		Kind kind;
		void* invoke_func_ptr;
		std::intptr_t bound_this_delta;
	};

	class Function
	{
	protected:
		const FunctionDescriptor* m_descriptor;
		DescribedBase* m_instance;

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

		uint64_t invoke(FunctionDescriptor::Arguments& arguments) const
		{
			using fn_t = uint64_t(DescribedBase*, uint64_t&, ...);
			switch (arguments.size())
			{
			case 0: return m_descriptor->native_func_ptr<fn_t>()(m_instance, arguments.return_value);
			case 1: return m_descriptor->native_func_ptr<fn_t>()(m_instance, arguments.return_value, arguments.get(1));
			case 2:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2));
			case 3:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2),
				    arguments.get(3));
			case 4:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2),
				    arguments.get(3),
				    arguments.get(4));
			case 5:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2),
				    arguments.get(3),
				    arguments.get(4),
				    arguments.get(5));
			case 6:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2),
				    arguments.get(3),
				    arguments.get(4),
				    arguments.get(5),
				    arguments.get(6));
			case 7:
				return m_descriptor->native_func_ptr<fn_t>()(m_instance,
				    arguments.return_value,
				    arguments.get(1),
				    arguments.get(2),
				    arguments.get(3),
				    arguments.get(4),
				    arguments.get(5),
				    arguments.get(6),
				    arguments.get(7));
			default: throw std::runtime_error("Too many arguments");
			}
		}
	};
}