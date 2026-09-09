#pragma once

#include "instance.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <map>
#include <memory>
#include <vector>

namespace RBX
{
	class Name;

	class ServiceProviderProp
	{
	};

	class ServiceProvider : public Instance, public ServiceProviderProp
	{
	public:
		virtual bool can_find_service() const = 0;

		virtual bool can_create_service() const = 0;

		std::vector<std::shared_ptr<Instance>> services;
		std::map<const Name*, std::shared_ptr<Instance>> service_by_name;
		boost::intrusive_ptr<rbx::signals::slots_holder> service_added_slots;
		boost::intrusive_ptr<rbx::signals::slots_holder> service_removing_slots;
		boost::intrusive_ptr<rbx::signals::slots_holder> close_slots;
		boost::intrusive_ptr<rbx::signals::slots_holder> closing_slots;
	};

	template<typename Derived, typename Base>
	class Described : public Base
	{
	};

	RML_LAYOUT_DIAGNOSTIC_PUSH()
	RML_ASSERT_OFFSET(ServiceProvider, services, 0xB0);
	RML_ASSERT_OFFSET(ServiceProvider, service_by_name, 0xC8);
#if defined(RML_WINDOWS)
	RML_ASSERT_OFFSET(ServiceProvider, service_added_slots, 0xD8);
	RML_ASSERT_SIZE(ServiceProvider, 0xF8);
#else
	RML_ASSERT_OFFSET(ServiceProvider, service_added_slots, 0xE0);
	RML_ASSERT_SIZE(ServiceProvider, 0x100);
#endif
	RML_LAYOUT_DIAGNOSTIC_POP()
}
