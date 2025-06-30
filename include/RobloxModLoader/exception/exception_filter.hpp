#pragma once

#include "RobloxModLoader/common.hpp"

namespace exception_filter {
    class exception_handler {
    public:
        static void initialize();

        static void shutdown();

    private:
        static long unhandled_exception_filter(PEXCEPTION_POINTERS exception_pointers);

        static void log_exception_info(PEXCEPTION_POINTERS exception_pointers);

        static void log_register_state(PCONTEXT context);

        static void log_stack_trace();

        static LPTOP_LEVEL_EXCEPTION_FILTER m_previous_filter;
        static bool m_initialized;
    };
}
