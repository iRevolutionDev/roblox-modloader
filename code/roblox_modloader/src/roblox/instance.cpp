#include "RobloxModLoader/roblox/instance.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace RBX
{
	std::string Instance::get_full_name()
	{
		std::string full_name{name};
		for (Instance* ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent)
			full_name = std::string{ancestor->name} + "." + full_name;
		return full_name;
	}
}
