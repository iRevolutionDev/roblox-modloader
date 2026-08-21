#pragma once
#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"
#include "callback_descriptor.hpp"
#include "descriptor.hpp"
#include "event.hpp"
#include "event_descriptor.hpp"
#include "function_descriptor.hpp"
#include "member.hpp"
#include "pointers.hpp"
#include "property_descriptor.hpp"
#include "yield_function_descriptor.hpp"

#include <mutex>
#include <string_view>
#include <unordered_map>

namespace RBX::Reflection
{
	enum class InterfaceId : std::int32_t;

	class ClassDescriptor : public Descriptor, public MemberDescriptorContainer<PropertyDescriptor>, public MemberDescriptorContainer<EventDescriptor>, public MemberDescriptorContainer<FunctionDescriptor>, public MemberDescriptorContainer<YieldFunctionDescriptor>, public MemberDescriptorContainer<CallbackDescriptor>
	{
	public:
		enum Functionality
		{
			PERSISTENT = 0x1 + 0x2 + 0x8 + 0x10,
			PERSISTENT_PLAYER = 0x1 + 0x4 + 0x8 + 0x10,
			PERSISTENT_LOCAL = 0x1 + 0x0 + 0x8 + 0x10,
			RUNTIME = 0x1 + 0x2 + 0x0 + 0x10,
			RUNTIME_PLAYER = 0x1 + 0x4 + 0x0 + 0x10,
			RUNTIME_LOCAL = 0x1 + 0x0 + 0x0 + 0x10,
			INTERNAL = 0x1 + 0x2 + 0x0 + 0x0,
			INTERNAL_PLAYER = 0x1 + 0x4 + 0x0 + 0x0,
			INTERNAL_LOCAL = 0x1 + 0x0 + 0x0 + 0x0,
			PERSISTENT_HIDDEN = 0x1 + 0x2 + 0x8 + 0x0,
			PERSISTENT_LOCAL_INTERNAL = 0x1 + 0x0 + 0x8 + 0x0,
		};

		struct Attributes : Descriptor::Attributes
		{
			Functionality flags;

			explicit Attributes(const Functionality flags) :
			    flags(flags)
			{
			}

			static Attributes deprecated(const Functionality flags)
			{
				Attributes result(flags);
				result.is_deprecated = true;
				return result;
			}
		};

		enum ReplicationLevel
		{
			NEVER_REPLICATE = 0,
			STANDARD_REPLICATE = 1,
			PLAYER_REPLICATE = 2,
		};

		typedef Vector<ClassDescriptor*> ClassDescriptors;

		using PropertyDescriptors = MemberDescriptorContainer<PropertyDescriptor>::DescriptorView;
		using FunctionDescriptors = MemberDescriptorContainer<FunctionDescriptor>::DescriptorView;
		using YieldFunctionDescriptors = MemberDescriptorContainer<YieldFunctionDescriptor>::DescriptorView;
		using EventDescriptors = MemberDescriptorContainer<EventDescriptor>::DescriptorView;
		using CallbackDescriptors = MemberDescriptorContainer<CallbackDescriptor>::DescriptorView;

	private:
		char padding[24]; // some boolean?
	public:
		const Security::Permissions security;

		ClassDescriptor* const base;
		ClassDescriptors derived_classes;
		const unsigned replicate_type : 2;
		const unsigned can_xml_write : 1;
		const unsigned is_scriptable : 1;

		const ClassDescriptor* get_base() const
		{
			return base;
		}

		ReplicationLevel get_replication_level() const
		{
			return static_cast<ReplicationLevel>(replicate_type);
		}

		bool is_script_creatable() const
		{
			return is_scriptable != 0;
		}

		bool is_serializable() const
		{
			return can_xml_write != 0;
		}

		ClassDescriptors::const_iterator derived_classes_begin() const
		{
			return derived_classes.begin();
		}

		ClassDescriptors::const_iterator derived_classes_end() const
		{
			return derived_classes.end();
		}

		bool is_base_of(const ClassDescriptor& child) const
		{
			return child.base && child.base->is_a(*this);
		}

		bool is_base_of(const char* child_name) const
		{
			return child_name && base ? base->is_a(child_name) : false;
		}

		bool is_a(const ClassDescriptor& test) const
		{
			if (name == test.name)
			{
				return true;
			}

			return base ? base->is_a(test) : false;
		}

		bool is_a(const char* test_name) const
		{
			if (name == test_name)
			{
				return true;
			}

			return base ? base->is_a(test_name) : false;
		}

		const ClassDescriptor& get_descriptor() const
		{
			return *this;
		}

		PropertyDescriptors property_descriptors() const
		{
			return MemberDescriptorContainer<PropertyDescriptor>::get_descriptor_view();
		}

		FunctionDescriptors function_descriptors() const
		{
			return MemberDescriptorContainer<FunctionDescriptor>::get_descriptor_view();
		}

		YieldFunctionDescriptors yield_function_descriptors() const
		{
			return MemberDescriptorContainer<YieldFunctionDescriptor>::get_descriptor_view();
		}

		EventDescriptors event_descriptors() const
		{
			return MemberDescriptorContainer<EventDescriptor>::get_descriptor_view();
		}

		CallbackDescriptors callback_descriptors() const
		{
			return MemberDescriptorContainer<CallbackDescriptor>::get_descriptor_view();
		}

		PropertyDescriptor* find_property_descriptor(const char* name) const
		{
			return MemberDescriptorContainer<PropertyDescriptor>::find_descriptor(name);
		}

		FunctionDescriptor* find_function_descriptor(const char* name) const
		{
			return MemberDescriptorContainer<FunctionDescriptor>::find_descriptor(name);
		}

		YieldFunctionDescriptor* find_yield_function_descriptor(const char* name) const
		{
			return MemberDescriptorContainer<YieldFunctionDescriptor>::find_descriptor(name);
		}

		EventDescriptor* find_event_descriptor(const char* name) const
		{
			return MemberDescriptorContainer<EventDescriptor>::find_descriptor(name);
		}

		CallbackDescriptor* find_callback_descriptor(const char* name) const
		{
			return MemberDescriptorContainer<CallbackDescriptor>::find_descriptor(name);
		}

		PropertyDescriptor* find_property_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<PropertyDescriptor>::find_descriptor(name);
		}

		FunctionDescriptor* find_function_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<FunctionDescriptor>::find_descriptor(name);
		}

		YieldFunctionDescriptor* find_yield_function_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<YieldFunctionDescriptor>::find_descriptor(name);
		}

		EventDescriptor* find_event_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<EventDescriptor>::find_descriptor(name);
		}

		CallbackDescriptor* find_callback_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<CallbackDescriptor>::find_descriptor(name);
		}

		template<typename T>
		T* find_descriptor(const char* name) const
		{
			if (!g_pointers || !g_pointers->m_roblox_pointers.get_string_atom
			    || !g_pointers->m_roblox_pointers.descriptor_lookup
			    || !g_pointers->m_roblox_pointers.member_table_offset)
			{
				return nullptr;
			}

			auto atom = g_pointers->m_roblox_pointers.get_string_atom(name);
			if (!atom)
			{
				return nullptr;
			}

			const std::uint64_t member_table_offset = g_pointers->m_roblox_pointers.member_table_offset;
			const auto desc = g_pointers->m_roblox_pointers.descriptor_lookup(reinterpret_cast<uint64_t>(this) + member_table_offset, &atom);
			if (desc && *desc)
			{
				return reinterpret_cast<T*>(*desc);
			}

			LOG_WARN("[ClassDescriptor::find_descriptor] Failed to find descriptor '{}' in class '{}'",
			    name,
			    this->name.c_str());
			return nullptr;
		}

		PropertyDescriptor* find_property(const char* name) const
		{
			return find_descriptor<PropertyDescriptor>(name);
		}

		FunctionDescriptor* find_function(const char* name) const
		{
			return find_descriptor<FunctionDescriptor>(name);
		}

		YieldFunctionDescriptor* find_yield_function(const char* name) const
		{
			return find_descriptor<YieldFunctionDescriptor>(name);
		}

		EventDescriptor* find_event(const char* name) const
		{
			return find_descriptor<EventDescriptor>(name);
		}

		CallbackDescriptor* find_callback(const char* name) const
		{
			return find_descriptor<CallbackDescriptor>(name);
		}

		bool operator==(const ClassDescriptor& other) const
		{
			return this == &other;
		}

		bool operator!=(const ClassDescriptor& other) const
		{
			return !(*this == other);
		}

		template<class T>
		MemberDescriptorContainer<T>::Collection::const_iterator begin() const
		{
			return MemberDescriptorContainer<T>::descriptors_begin();
		}

		template<class T>
		MemberDescriptorContainer<T>::Collection::const_iterator end() const
		{
			return MemberDescriptorContainer<T>::descriptors_end();
		}

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(ClassDescriptor, 0x250);
			RML_ASSERT_LAYOUT_OFFSET(ClassDescriptor, padding, 0x208);
			RML_ASSERT_LAYOUT_OFFSET(ClassDescriptor, security, 0x220);
			RML_ASSERT_LAYOUT_OFFSET(ClassDescriptor, base, 0x228);
			RML_ASSERT_LAYOUT_OFFSET(ClassDescriptor, derived_classes, 0x230);
		RML_LAYOUT_GUARD_END()
	};

	class DescribedBase : public EventSource, public std::enable_shared_from_this<DescribedBase>
	{
	protected:
		const ClassDescriptor* descriptor;
		std::unique_ptr<std::string> xml_id;

	public:
		DescribedBase()
		{
		}

		virtual ~DescribedBase()
		{
		}

		inline const ClassDescriptor& get_descriptor() const
		{
			return *descriptor;
		};

		template<class T>
		inline bool is_a() const
		{
			return get_descriptor().is_a(T::get_descriptor());
		}

		template<class T>
		static inline bool is_a(const DescribedBase* instance)
		{
			return instance ? instance->get_descriptor().is_a(T::get_descriptor()) : false;
		}

		bool is_a(std::string className)
		{
			return get_descriptor().is_a(className.c_str());
		}

		template<class T>
		inline T* fast_dynamic_cast()
		{
			return (get_descriptor().is_a(T::get_descriptor())) ? static_cast<T*>(this) : NULL;
		}

		template<class T>
		inline const T* fast_dynamic_cast() const
		{
			return (get_descriptor().is_a(T::get_descriptor())) ? static_cast<const T*>(this) : NULL;
		}

		template<class T>
		static inline T* fast_dynamic_cast(DescribedBase* instance)
		{
			return (instance && instance->get_descriptor().is_a(T::get_descriptor())) ? static_cast<T*>(instance) : NULL;
		}

		template<class T>
		static inline const T* fast_dynamic_cast(const DescribedBase* instance)
		{
			return (instance && instance->get_descriptor().is_a(T::get_descriptor())) ? static_cast<const T*>(instance) : NULL;
		}

		// This function replaces shared_dynamic_cast for classes that derives from DescribedCreatable or DescribedNonCreatable.
		template<class T, class U>
		static inline std::shared_ptr<T> fast_shared_dynamic_cast(const std::shared_ptr<U>& instance)
		{
			return is_a<T>(instance.get()) ? shared_static_cast<T>(instance) : std::shared_ptr<T>();
		}

		PropertyDescriptor* find_property_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<PropertyDescriptor>::find_descriptor(name);
		}

		FunctionDescriptor* find_function_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<FunctionDescriptor>::find_descriptor(name);
		}

		YieldFunctionDescriptor* find_yield_function_descriptor(const char* name) const
		{
			return get_descriptor().MemberDescriptorContainer<YieldFunctionDescriptor>::find_descriptor(name);
		}

		CallbackDescriptor* find_callback_descriptor(const char* name)
		{
			return get_descriptor().MemberDescriptorContainer<CallbackDescriptor>::find_descriptor(name);
		}

		EventDescriptor* find_signal_descriptor(const char* name) const
		{
			return get_descriptor().MemberDescriptorContainer<EventDescriptor>::find_descriptor(name);
		}

		const std::string* get_xml_id() const
		{
			return xml_id.get();
		}

		void set_xml_id(const std::string& newId)
		{
			if (!xml_id)
			{
				xml_id.reset(new std::string(newId));
			}
			else
			{
				*xml_id = newId;
			}
		}

		virtual void* get_as_internal(InterfaceId interface_id) const = 0;

		virtual const RBX::Name& get_class_name() const = 0;

		virtual void on_created() = 0;
	};
}