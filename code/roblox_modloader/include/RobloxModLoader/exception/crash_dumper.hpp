#pragma once

#include "RobloxModLoader/common.hpp"
#include <memory>
#include <bit>

namespace PLH {
    class IatHook;
}

namespace exception_filter {
    class CrashDumper {
        static constexpr int DEFAULT_DUMP_TYPE =
                MiniDumpNormal |
                MiniDumpWithThreadInfo |
                MiniDumpWithIndirectlyReferencedMemory |
                MiniDumpWithModuleHeaders |
                MiniDumpWithAvxXStateContext;

        bool m_enabled = false;
        bool m_full_memory_dump = false;
        void *m_previous_exception_filter = nullptr;
        void *m_veh_handle = nullptr;
        std::unique_ptr<PLH::IatHook> m_set_unhandled_exception_filter_hook;
        uint64_t m_hook_trampoline = 0;

    public:
        CrashDumper();

        ~CrashDumper();

        CrashDumper(const CrashDumper &) = delete;

        CrashDumper &operator=(const CrashDumper &) = delete;

        CrashDumper(CrashDumper &&) = delete;

        CrashDumper &operator=(CrashDumper &&) = delete;

        void enable();

        void disable();

        void set_full_memory_dump(bool enabled);

        [[nodiscard]] bool is_enabled() const { return m_enabled; }

    private:
        static LONG WINAPI exception_handler(PEXCEPTION_POINTERS exception_pointers);

        static LONG WINAPI vectored_exception_handler(PEXCEPTION_POINTERS exception_pointers);

        static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI hooked_set_unhandled_exception_filter(
            LPTOP_LEVEL_EXCEPTION_FILTER filter);

        bool create_minidump(PEXCEPTION_POINTERS exception_pointers, const std::wstring &dump_path) const;

        static std::wstring generate_dump_filename();

        static void log_exception_info(PEXCEPTION_POINTERS exception_pointers);

        static void log_register_state(PCONTEXT context);

        static void log_stack_trace();
    };
}

inline exception_filter::CrashDumper *g_crash_dumper{};
