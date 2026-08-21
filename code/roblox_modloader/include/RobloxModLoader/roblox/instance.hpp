#pragma once
#include "object.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace RBX
{
	using namespace Reflection;

	class Instance;

	using Instances = std::vector<std::shared_ptr<Instance>>;

	class InstanceProp
	{
	public:
		void* reserved;
	};

	class Instance : public Object
	{
	public:
		InstanceProp prop;
		Instance* parent;
		Flyweight<std::string> name;
		std::shared_ptr<std::vector<std::shared_ptr<Instance>>> children;

	private:
		std::byte reserved_88[0x28];

	public:
		template<typename T = Instance>
		T* as()
		{
			return static_cast<T*>(this);
		}

		std::string get_full_name();
	};

	RML_LAYOUT_DIAGNOSTIC_PUSH()
	RML_ASSERT_OFFSET(Instance, prop, 0x60);
	RML_ASSERT_OFFSET(Instance, parent, 0x68);
	RML_ASSERT_OFFSET(Instance, name, 0x70);
	RML_ASSERT_OFFSET(Instance, children, 0x78);
	RML_ASSERT_SIZE(Instance, 0xB0);
	RML_LAYOUT_DIAGNOSTIC_POP()
}
