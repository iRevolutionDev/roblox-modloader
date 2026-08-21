#pragma once
#include "flyweight.hpp"
#include "reflection/object.hpp"
#include "security/script_permissions.hpp"
#include "slots_holder.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace RBX
{
	class Actor;
	class ComponentMap;
	class EngineContext;
	class RemoteInvocationMetadata;
	class Time;

	namespace DataModelChangeTracking
	{
		class ChangeTracker;
		enum class DeltaTypeTag : std::int32_t;
	}

	namespace Security
	{
		class Context;
	}

	namespace details
	{
		class AttributesAndTags;
	}

	class ObjectProp
	{
	};

	template<typename T>
	class GuidItem
	{
	public:
		void* scope;
		std::uint64_t index;
	};

	class Object : public Reflection::DescribedBase, public ObjectProp
	{
	public:
		virtual void write_all_properties_for_change_tracking(
		    DataModelChangeTracking::ChangeTracker* tracker, DataModelChangeTracking::DeltaTypeTag tag) = 0;

		virtual void deprecated_predelete() = 0;

		virtual void on_property_changed(const Reflection::PropertyDescriptor& descriptor) = 0;

		virtual void on_guid_changed() = 0;

		virtual bool security_check(const Security::Context& context) const = 0;

		virtual bool security_check_permission_level_only(const Security::Context& context) const = 0;

		virtual bool security_check_capabilities(Security::Capabilities capabilities) const = 0;

		virtual bool security_check_capabilities_permission_level_only(
		    Security::Capabilities capabilities) const = 0;

		virtual Actor* get_actor() = 0;

		virtual void set_modified_flag(const Reflection::PropertyDescriptor* descriptor, bool modified) = 0;

		virtual bool filter_event_invocation(const Reflection::EventDescriptor& descriptor,
		    const RemoteInvocationMetadata& metadata, Reflection::SystemAddress source) = 0;

		virtual void* to_content_ownership() = 0;

		virtual std::string get_object_name() const = 0;

		virtual std::string get_full_name() const = 0;

		virtual void record_property_update_if_eligible(Time when, const std::string& property) = 0;

		virtual void raise_property_changed_post_virtual(
		    const Reflection::PropertyDescriptor& descriptor) = 0;

		GuidItem<Object> guid;
		std::unique_ptr<ComponentMap> components;
		std::unique_ptr<details::AttributesAndTags> attributes_and_tags;
		boost::intrusive_ptr<rbx::signals::slots_holder> ancestry_changed_slots;
		boost::intrusive_ptr<rbx::signals::slots_holder> property_changed_slots;
		EngineContext* engine_context;
	};

	RML_LAYOUT_DIAGNOSTIC_PUSH()
	RML_ASSERT_OFFSET(Object, guid, 0x28);
	RML_ASSERT_OFFSET(Object, components, 0x38);
	RML_ASSERT_OFFSET(Object, attributes_and_tags, 0x40);
	RML_ASSERT_OFFSET(Object, ancestry_changed_slots, 0x48);
	RML_ASSERT_OFFSET(Object, property_changed_slots, 0x50);
	RML_ASSERT_OFFSET(Object, engine_context, 0x58);
	RML_ASSERT_SIZE(Object, 0x60);
	RML_LAYOUT_DIAGNOSTIC_POP()
}
