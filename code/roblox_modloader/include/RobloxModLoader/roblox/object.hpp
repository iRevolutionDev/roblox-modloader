#pragma once
#include "flyweight.hpp"
#include "reflection/object.hpp"
#include "slots_holder.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstdint>
#include <memory>

namespace RBX
{
	class ComponentMap;
	class EngineContext;

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
