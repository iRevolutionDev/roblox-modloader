#include "RobloxModLoader/roblox/reflection/member.hpp"

size_t StringHashPredicate::operator ()(const char *s) const {
    return std::hash<std::string>()(s);
}
