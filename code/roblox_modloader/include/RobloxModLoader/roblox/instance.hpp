#pragma once
#include "object.hpp"
#include "reflection/object.hpp"

#include <memory>
#include <vector>

namespace RBX
{
	using namespace Reflection;

	class Instance : public Object
	{
		std::byte pad_0048[0x30];

	public:
		Instance* parent;

	private:
		std::byte pad_0058[0x20];

	public:
		std::string_view name;

		// shared_ptr causes some crashes because has invalid reference count TODO: fix it later
		std::shared_ptr<std::vector<std::shared_ptr<Instance> > > children;

		template<typename T = Instance>
		T* as()
		{
			return static_cast<T*>(this);
		}

		std::string get_full_name();
	};
}