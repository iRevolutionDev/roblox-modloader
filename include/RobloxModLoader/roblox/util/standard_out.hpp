#pragma once

namespace RBX {
    typedef enum {
        MESSAGE_OUTPUT,
        MESSAGE_INFO,
        MESSAGE_WARNING,
        MESSAGE_ERROR,
        MESSAGE_SENSITIVE,
        MESSAGE_TYPE_MAX
    } MessageType;

    class StandardOut {
    public:
        static void printf(MessageType message_type, const char *format, ...);

        static void printf(MessageType message_type, std::string_view format, ...);
    };
}
