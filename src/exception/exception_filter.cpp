#include "RobloxModLoader/exception/exception_filter.hpp"
#include "RobloxModLoader/memory/module_utils.hpp"
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace exception_filter {
    LPTOP_LEVEL_EXCEPTION_FILTER exception_handler::m_previous_filter = nullptr;
    bool exception_handler::m_initialized = false;

    void exception_handler::initialize() {
        if (m_initialized) {
            LOG_WARN("Exception handler already initialized");
            return;
        }
        m_previous_filter = SetUnhandledExceptionFilter(unhandled_exception_filter);
        m_initialized = true;

        LOG_INFO("Exception handler initialized successfully");
    }

    void exception_handler::shutdown() {
        if (!m_initialized) {
            LOG_WARN("Exception handler not initialized");
            return;
        }

        SetUnhandledExceptionFilter(m_previous_filter);
        m_initialized = false;

        LOG_INFO("Exception handler shutdown successfully");
    }

    long exception_handler::unhandled_exception_filter(PEXCEPTION_POINTERS exception_pointers) {
        thread_local bool in_exception_handler = false;
        if (in_exception_handler) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        in_exception_handler = true;

        try {
            LOG_ERROR("WARNING: Exception handler caught an exception!");
            LOG_ERROR("Exception Code: 0x{:08X}", exception_pointers->ExceptionRecord->ExceptionCode);
            LOG_ERROR("Exception Address: 0x{:016X}",
                      reinterpret_cast<uintptr_t>(exception_pointers->ExceptionRecord->ExceptionAddress));

            log_exception_info(exception_pointers);
            log_register_state(exception_pointers->ContextRecord);
            log_stack_trace();

            LOG_ERROR("Exception handling completed.");
            LOG_ERROR("Please send the log file to the developer for analysis.");
            try {
                global_logger()->flush();
            } catch (...) {
            }
            try {
                MessageBoxA(nullptr,
                            "Roblox ModLoader exception filter has been triggered!\nStack trace has been logged.\nCheck the console and log files for details.",
                            "RobloxModLoader - Exception Caught",
                            MB_OK | MB_ICONWARNING);
            } catch (...) {
            }
        } catch (...) {
            in_exception_handler = false;
            return EXCEPTION_CONTINUE_SEARCH;
        }
        in_exception_handler = false;

        return EXCEPTION_CONTINUE_SEARCH;
    }

    void exception_handler::log_exception_info(PEXCEPTION_POINTERS exception_pointers) {
        try {
            const auto *context = exception_pointers->ContextRecord;
            const auto *exception_record = exception_pointers->ExceptionRecord;

            LOG_ERROR("=== EXCEPTION INFORMATION ===");
            LOG_ERROR("Thread RIP: 0x{:016X}", context->Rip);
            const auto our_module_base = reinterpret_cast<uintptr_t>(g_hinstance);
            const auto rebased_our_rip = context->Rip - our_module_base;
            LOG_ERROR("roblox_modloader.dll Base: 0x{:016X}", our_module_base);
            LOG_ERROR("Rebased ModLoader: 0x{:016X}", rebased_our_rip);

            if (const auto roblox_base = memory::module_utils::get_roblox_studio_base(); roblox_base != 0) {
                const auto rebased_rip = context->Rip - roblox_base;
                LOG_ERROR("RobloxStudioBeta.exe Base: 0x{:016X}", roblox_base);
                LOG_ERROR("Rebased Studio: 0x{:016X}", rebased_rip);
            }
            const auto exception_addr = reinterpret_cast<uintptr_t>(exception_record->ExceptionAddress);
            if (const auto module_name = memory::module_utils::get_module_name_from_address(exception_addr); !
                module_name.
                empty()) {
                const auto module_base = memory::module_utils::get_module_base_address(module_name);
                const auto rebased_addr = exception_addr - module_base;

                LOG_ERROR("Exception Module: {} @ 0x{:016X}", module_name, module_base);
                LOG_ERROR("Exception Rebased Address: 0x{:016X}", rebased_addr);
            }
        } catch (...) {
            LOG_ERROR("Failed to log exception info safely");
        }
    }

    void exception_handler::log_register_state(PCONTEXT context) {
        try {
            LOG_ERROR("=== REGISTER STATE ===");

            LOG_ERROR("-- START GENERAL PURPOSE REGISTERS --");
            LOG_ERROR("RAX: 0x{:016X}", context->Rax);
            LOG_ERROR("RBX: 0x{:016X}", context->Rbx);
            LOG_ERROR("RCX: 0x{:016X}", context->Rcx);
            LOG_ERROR("RDX: 0x{:016X}", context->Rdx);
            LOG_ERROR("RDI: 0x{:016X}", context->Rdi);
            LOG_ERROR("RSI: 0x{:016X}", context->Rsi);
            LOG_ERROR("R08: 0x{:016X}", context->R8);
            LOG_ERROR("R09: 0x{:016X}", context->R9);
            LOG_ERROR("R10: 0x{:016X}", context->R10);
            LOG_ERROR("R11: 0x{:016X}", context->R11);
            LOG_ERROR("R12: 0x{:016X}", context->R12);
            LOG_ERROR("R13: 0x{:016X}", context->R13);
            LOG_ERROR("R14: 0x{:016X}", context->R14);
            LOG_ERROR("R15: 0x{:016X}", context->R15);
            LOG_ERROR("-- END GP REGISTERS --");

            LOG_ERROR("-- START STACK POINTERS --");
            LOG_ERROR("RBP: 0x{:016X}", context->Rbp);
            LOG_ERROR("RSP: 0x{:016X}", context->Rsp);
            LOG_ERROR("-- END STACK POINTERS --");

            LOG_ERROR("-- END REGISTERS STATE --");
        } catch (...) {
            LOG_ERROR("Failed to log register state safely");
        }
    }

    static bool safe_resolve_symbol(const HANDLE process, const DWORD64 address, char *symbol_name,
                                    const size_t symbol_name_size, DWORD64 *displacement) {
        __try {
            char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(char)];
            auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            if (SymFromAddr(process, address, displacement, symbol)) {
                strncpy_s(symbol_name, symbol_name_size, symbol->Name, _TRUNCATE);
                return true;
            }
            return false;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    void exception_handler::log_stack_trace() {
        try {
            LOG_ERROR("=== STACK TRACE ===");

            HANDLE process = nullptr;
            if (!DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(),
                                 &process, PROCESS_ALL_ACCESS, FALSE, 0)) {
                LOG_ERROR("Failed to duplicate process handle: {}", GetLastError());
                process = GetCurrentProcess();
            }

            if (!SymInitialize(process, nullptr, TRUE)) {
                LOG_ERROR("Failed to initialize symbol handler: {}", GetLastError());
                if (process != GetCurrentProcess()) {
                    CloseHandle(process);
                }
                return;
            }

            SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

            void *stack[256];
            const auto frame_count = RtlCaptureStackBackTrace(0, 255, stack, nullptr);

            LOG_ERROR("Captured {} stack frames:", frame_count);

            for (WORD i = 0; i < frame_count; ++i) {
                try {
                    const auto address = reinterpret_cast<DWORD64>(stack[i]);
                    char symbol_name[MAX_SYM_NAME] = {0};
                    DWORD64 displacement = 0;

                    if (safe_resolve_symbol(process, address, symbol_name, sizeof(symbol_name), &displacement)) {
                        const auto module_name = memory::module_utils::get_module_name_from_address(address);
                        const auto roblox_base = memory::module_utils::get_roblox_studio_base();
                        const auto rebased_addr = memory::module_utils::get_roblox_studio_rebased_address(
                            address, roblox_base);

                        LOG_ERROR(
                            "[Stack Frame {}] Inside {} @ 0x{:016X} ({}) | Studio Rebase: 0x{:016X} | Displacement: +0x{:X}",
                            i, symbol_name, address, module_name, rebased_addr, displacement);
                    } else {
                        const auto module_name = memory::module_utils::get_module_name_from_address(address);
                        const auto roblox_base = memory::module_utils::get_roblox_studio_base();
                        const auto rebased_addr = memory::module_utils::get_roblox_studio_rebased_address(
                            address, roblox_base);

                        LOG_ERROR("[Stack Frame {}] Unknown Subroutine @ 0x{:016X} ({}) | Studio Rebase: 0x{:016X}",
                                  i, address, module_name, rebased_addr);
                    }
                } catch (...) {
                    LOG_ERROR("[Stack Frame {}] Failed to resolve frame", i);
                }
            }

            try {
                std::stringstream stack_chain;
                for (WORD i = 0; i < frame_count; ++i) {
                    stack_chain << std::hex << "0x" << reinterpret_cast<uintptr_t>(stack[i]);
                    if (i < frame_count - 1) {
                        stack_chain << " -> ";
                    }
                }
                LOG_ERROR("Stack Chain: {}", stack_chain.str());
            } catch (...) {
                LOG_ERROR("Failed to generate stack chain");
            }

            SymCleanup(process);

            if (process != GetCurrentProcess()) {
                CloseHandle(process);
            }
        } catch (...) {
            LOG_ERROR("Failed to log stack trace safely");
        }
    }
}
