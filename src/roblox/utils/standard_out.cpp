#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/util/standard_out.hpp"

#include "pointers.hpp"

void RBX::StandardOut::printf(const MessageType message_type, const char *format, ...) {
    if (!g_pointers) return;

    va_list args;
    va_start(args, format);
    g_pointers->m_roblox_pointers.print(message_type, format, args);
    va_end(args);
}

void RBX::StandardOut::printf(const MessageType message_type, std::string_view format, ...) {
    if (!g_pointers) return;

    va_list args;
    va_start(args, format);
    g_pointers->m_roblox_pointers.print(message_type, format.data(), args);
    va_end(args);
}
