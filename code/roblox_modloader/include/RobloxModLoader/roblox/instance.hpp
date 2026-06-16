#pragma once
#include "object.hpp"
#include "reflection/object.hpp"

#include <memory>
#include <vector>

namespace RBX
{
	using namespace Reflection;

	class Instance;

	using Instances = std::vector<std::shared_ptr<Instance>>;

	class Instance : public Object
	{
		std::byte pad_0048[0x40];

	public:
		Instance* parent;
		// shared_ptr causes some crashes because has invalid reference count TODO: fix it later
		std::shared_ptr<std::vector<std::shared_ptr<Instance>>> children;

	private:
		std::byte pad_0058[0x28];

	public:
		std::string_view name;

		template<typename T = Instance>
		T* as()
		{
			return static_cast<T*>(this);
		}

		std::string get_full_name();
	};
}