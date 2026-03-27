#include "RobloxModLoader/exception/crash_dumper.hpp"
#include "RobloxModLoader/memory/module_utils.hpp"
#include <dbghelp.h>
#include <polyhook2/PE/IatHook.hpp>
#include <chrono>
#include <filesystem>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

namespace fs = std::filesystem;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::time_point_cast;

namespace exception_filter {
    CrashDumper::CrashDumper() = default;

    CrashDumper::~CrashDumper() {
        disable();
    }

    void CrashDumper::enable() {
        if (m_enabled) {
            LOG_WARN("Crash dumper already enabled");
            return;
        }

        try {
            SetErrorMode(SEM_FAILCRITICALERRORS);

            m_previous_exception_filter = SetUnhandledExceptionFilter(exception_handler);
            m_set_unhandled_exception_filter_hook = std::make_unique<PLH::IatHook>(
                "kernel32.dll",
                "SetUnhandledExceptionFilter",
                std::bit_cast<uint64_t>(&hooked_set_unhandled_exception_filter),
                &m_hook_trampoline,
                L""
            );

            if (!m_set_unhandled_exception_filter_hook->hook()) {
                LOG_ERROR("Failed to hook SetUnhandledExceptionFilter");
                SetUnhandledExceptionFilter(
                    reinterpret_cast<LPTOP_LEVEL_EXCEPTION_FILTER>(m_previous_exception_filter));
                return;
            }

            m_enabled = true;
            LOG_INFO("Crash dumper enabled");
        } catch (const std::exception &e) {
            LOG_ERROR("Failed to enable crash dumper: {}", e.what());
        } catch (...) {
            LOG_ERROR("Failed to enable crash dumper: unknown exception");
        }
    }

    void CrashDumper::disable() {
        if (!m_enabled) {
            return;
        }

        try {
            if (m_set_unhandled_exception_filter_hook) {
                m_set_unhandled_exception_filter_hook->unHook();
                m_set_unhandled_exception_filter_hook.reset();
            }

            SetUnhandledExceptionFilter(reinterpret_cast<LPTOP_LEVEL_EXCEPTION_FILTER>(m_previous_exception_filter));
            m_enabled = false;

            LOG_INFO("Crash dumper disabled successfully");
        } catch (...) {
            LOG_ERROR("Exception occurred while disabling crash dumper");
        }
    }

    void CrashDumper::set_full_memory_dump(bool enabled) {
        m_full_memory_dump = enabled;
        LOG_INFO("Full memory dump {}", enabled ? "enabled" : "disabled");
    }

    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI CrashDumper::hooked_set_unhandled_exception_filter(
        LPTOP_LEVEL_EXCEPTION_FILTER filter) {
        LOG_WARN("Attempt to override exception handler blocked");
        return nullptr;
    }

    std::wstring CrashDumper::generate_dump_filename() {
        try {
            std::wstring dump_dir;
            if (auto current_path = fs::current_path(); !current_path.empty()) {
                dump_dir = current_path.wstring();
            } else {
                wchar_t temp_path[MAX_PATH];
                if (GetTempPathW(MAX_PATH, temp_path) > 0) {
                    dump_dir = temp_path;
                } else {
                    dump_dir = L"C:\\";
                }
            }

            const auto crash_dir = fs::path(dump_dir) / "RobloxModLoader" / "crashes";
            std::error_code ec;
            fs::create_directories(crash_dir, ec);

            bool use_local_time = true;
#ifdef _WIN32
            if (const auto module = GetModuleHandleW(L"ntdll.dll");
                module && GetProcAddress(module, "wine_get_version")) {
                use_local_time = false;
            }
#endif

            std::wstring filename;
            if (use_local_time) {
                const auto now = time_point_cast<seconds>(system_clock::now());
                const auto time_t = system_clock::to_time_t(now);

                std::tm local_tm{};
                if (localtime_s(&local_tm, &time_t) == 0) {
                    filename = std::format(L"crash_{:04d}_{:02d}_{:02d}_{:02d}_{:02d}_{:02d}.dmp",
                                           local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
                                           local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
                } else {
                    filename = std::format(L"crash_{}.dmp",
                                           std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).
                                           count());
                }
            } else {
                const auto now = time_point_cast<seconds>(system_clock::now());
                const auto time_t = system_clock::to_time_t(now);

                std::tm utc_tm{};
                if (gmtime_s(&utc_tm, &time_t) == 0) {
                    filename = std::format(L"crash_{:04d}_{:02d}_{:02d}_{:02d}_{:02d}_{:02d}.dmp",
                                           utc_tm.tm_year + 1900, utc_tm.tm_mon + 1, utc_tm.tm_mday,
                                           utc_tm.tm_hour, utc_tm.tm_min, utc_tm.tm_sec);
                } else {
                    filename = std::format(L"crash_{}.dmp",
                                           std::chrono::duration_cast<seconds>(now.time_since_epoch()).
                                           count());
                }
            }

            return (crash_dir / filename).wstring();
        } catch (...) {
            const auto timestamp = std::chrono::duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
            return std::format(L"crash_{}.dmp", timestamp);
        }
    }

    bool CrashDumper::create_minidump(PEXCEPTION_POINTERS exception_pointers, const std::wstring &dump_path) {
        try {
            const HANDLE file = CreateFileW(
                dump_path.c_str(),
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );

            if (file == INVALID_HANDLE_VALUE) {
                const auto error = GetLastError();
                LOG_ERROR("Failed to create crash dump file {}: {}",
                          std::filesystem::path(dump_path).string(), error);
                return false;
            }

            MINIDUMP_EXCEPTION_INFORMATION exception_info{};
            exception_info.ThreadId = GetCurrentThreadId();
            exception_info.ExceptionPointers = exception_pointers;
            exception_info.ClientPointers = FALSE;

            constexpr bool full_memory = false;
            constexpr int additional_flags =
                    full_memory ? MiniDumpWithFullMemory | MiniDumpIgnoreInaccessibleMemory : 0;
            constexpr auto dump_type = static_cast<MINIDUMP_TYPE>(DEFAULT_DUMP_TYPE | additional_flags);

            const BOOL result = MiniDumpWriteDump(
                GetCurrentProcess(),
                GetCurrentProcessId(),
                file,
                dump_type,
                &exception_info,
                nullptr,
                nullptr
            );

            CloseHandle(file);

            if (!result) {
                const auto error = GetLastError();
                LOG_ERROR("Failed to write crash dump: {}", error);
                return false;
            }

            LOG_INFO("Crash dump successfully written to: {}",
                     std::filesystem::path(dump_path).string());
            return true;
        } catch (...) {
            LOG_ERROR("Exception occurred while creating crash dump");
            return false;
        }
    }

    LONG WINAPI CrashDumper::exception_handler(PEXCEPTION_POINTERS exception_pointers) {
        thread_local bool in_exception_handler = false;
        if (in_exception_handler) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        in_exception_handler = true;

        try {
            LOG_ERROR("=== CRITICAL EXCEPTION DETECTED ===");
            LOG_ERROR("Exception Code: 0x{:08X}", exception_pointers->ExceptionRecord->ExceptionCode);
            LOG_ERROR("Exception Address: 0x{:016X}",
                      reinterpret_cast<uintptr_t>(exception_pointers->ExceptionRecord->ExceptionAddress));

            const auto dump_path = generate_dump_filename();
            const bool dump_created = create_minidump(exception_pointers, dump_path);

            log_exception_info(exception_pointers);
            log_register_state(exception_pointers->ContextRecord);
            log_stack_trace();

            try {
                global_logger()->flush();
            } catch (...) {
            }

            std::wstring message;
            if (dump_created) {
                message = std::format(L"Roblox ModLoader has encountered a critical error!\n\n"
                                      L"A crash dump has been created at:\n{}\n\n"
                                      L"Please send this file along with the log files to the developer for analysis.\n\n"
                                      L"The application will now exit.",
                                      dump_path);
            } else {
                message = L"Roblox ModLoader has encountered a critical error!\n\n"
                        L"Failed to create crash dump, but error details have been logged.\n"
                        L"Please check the log files and send them to the developer.\n\n"
                        L"The application will now exit.";
            }

            MessageBoxW(nullptr, message.c_str(), L"RobloxModLoader - Critical Error", MB_OK | MB_ICONERROR);
        } catch (...) {
            try {
                MessageBoxW(nullptr,
                            L"RobloxModLoader encountered a critical error during exception handling.\n"
                            L"Please check log files for details.",
                            L"RobloxModLoader - Fatal Error",
                            MB_OK | MB_ICONERROR);
            } catch (...) {
            }
        }

        in_exception_handler = false;
        return EXCEPTION_EXECUTE_HANDLER;
    }

    void CrashDumper::log_exception_info(PEXCEPTION_POINTERS exception_pointers) {
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
            if (const auto module_name = memory::module_utils::get_module_name_from_address(exception_addr);
                !module_name.empty()) {
                const auto module_base = memory::module_utils::get_module_base_address(module_name);
                const auto rebased_addr = exception_addr - module_base;

                LOG_ERROR("Exception Module: {} @ 0x{:016X}", module_name, module_base);
                LOG_ERROR("Exception Rebased Address: 0x{:016X}", rebased_addr);
            }

            LOG_ERROR("Exception Flags: 0x{:08X}", exception_record->ExceptionFlags);
            LOG_ERROR("Number of Parameters: {}", exception_record->NumberParameters);
            for (DWORD i = 0; i < exception_record->NumberParameters && i < EXCEPTION_MAXIMUM_PARAMETERS; ++i) {
                LOG_ERROR("Parameter {}: 0x{:016X}", i, exception_record->ExceptionInformation[i]);
            }
        } catch (...) {
            LOG_ERROR("Failed to log exception info safely");
        }
    }

    void CrashDumper::log_register_state(PCONTEXT context) {
        try {
            LOG_ERROR("=== REGISTER STATE ===");

            LOG_ERROR("-- GENERAL PURPOSE REGISTERS --");
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

            LOG_ERROR("-- STACK POINTERS --");
            LOG_ERROR("RBP: 0x{:016X}", context->Rbp);
            LOG_ERROR("RSP: 0x{:016X}", context->Rsp);
            LOG_ERROR("RIP: 0x{:016X}", context->Rip);

            LOG_ERROR("-- FLAGS --");
            LOG_ERROR("RFLAGS: 0x{:016X}", context->EFlags);
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

    void CrashDumper::log_stack_trace() {
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
