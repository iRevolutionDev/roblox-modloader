#pragma once

#include "RobloxModLoader/roblox/util/G3DCore.h"
#include "enum_descriptor.hpp"
#include "member.hpp"
#include "type.hpp"

#include <cstdint>

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
			Variant return_value;

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

	protected:
		SignatureDescriptor signature;
		Kind kind;
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

		void invoke(FunctionDescriptor::Arguments& arguments, lua_State* L) const
		{
			return m_descriptor->invoke(m_instance, arguments, L);
		}
	};
}