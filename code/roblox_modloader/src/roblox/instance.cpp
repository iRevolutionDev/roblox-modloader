#include "RobloxModLoader/roblox/instance.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace RBX
{
	std::string Instance::get_full_name()
	{
		std::string full_name{name.value()};
		for (Instance* ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent)
			full_name = ancestor->name.value() + "." + full_name;
		return full_name;
	}
}
