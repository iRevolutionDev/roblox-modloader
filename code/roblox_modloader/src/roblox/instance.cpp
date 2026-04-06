#include "RobloxModLoader/roblox/instance.hpp"

#include "RobloxModLoader/common.hpp"

namespace RBX
{
	std::string Instance::get_full_name()
	{
		uintptr_t result = 0;
		return get_descriptor().find_function_descriptor("GetFullName")->invoke<std::string&>(this, &result);
	}
}
