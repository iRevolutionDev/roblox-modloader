#include "RobloxModLoader/roblox/reflection/member.hpp"

namespace RBX::Reflection
{
	size_t StringHashPredicate::operator()(const char* s) const
	{
		if (!s)
		{
			return 0;
		}

		return std::hash<std::string_view>()(std::string_view(s));
	}
}