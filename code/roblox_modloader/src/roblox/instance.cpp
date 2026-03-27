#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/instance.hpp"

std::string Instance::get_full_name() {
    uintptr_t result = 0;
    return class_descriptor->find_function_descriptor("GetFullName")->invoke<std::string &>(this, &result);
}
