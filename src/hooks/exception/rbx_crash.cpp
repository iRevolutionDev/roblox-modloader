#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/memory/module_utils.hpp"
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

void log_crash_stack_trace() {
    LOG_ERROR("=== ROBLOX CRASH STACK TRACE ===");

    const auto process = GetCurrentProcess();
    if (!SymInitialize(process, nullptr, TRUE)) {
        LOG_ERROR("Failed to initialize symbol handler for crash trace: {}", GetLastError());
        return;
    }

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    void *stack[256];
    const auto frame_count = RtlCaptureStackBackTrace(0, 100, stack, nullptr);

    LOG_ERROR("Captured {} stack frames from Roblox crash:", frame_count);

    for (WORD i = 0; i < frame_count; ++i) {
        const auto address = reinterpret_cast<DWORD64>(stack[i]);

        char symbol_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbol_buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD value{};
        DWORD *pValue = &value;

        if (SymFromAddr(process, address, nullptr, symbol) &&
            ((*pValue = symbol->Address - address)) &&
            SymFromAddr(process, address, reinterpret_cast<PDWORD64>(pValue), symbol)) {
            const auto module_name = memory::module_utils::get_module_name_from_address(address);
            const auto roblox_base = memory::module_utils::get_roblox_studio_base();
            const auto rebased_addr = memory::module_utils::get_roblox_studio_rebased_address(address, roblox_base);

            LOG_ERROR("[Crash Frame {}] Inside {} @ 0x{:016X} ({}) | Studio Rebase: 0x{:016X}",
                      i, symbol->Name, address, module_name, rebased_addr);
        } else {
            const auto module_name = memory::module_utils::get_module_name_from_address(address);
            const auto roblox_base = memory::module_utils::get_roblox_studio_base();
            const auto rebased_addr = memory::module_utils::get_roblox_studio_rebased_address(address, roblox_base);

            LOG_ERROR("[Crash Frame {}] Unknown Subroutine @ 0x{:016X} ({}) | Studio Rebase: 0x{:016X}",
                      i, address, module_name, rebased_addr);
        }
    }

    std::stringstream stack_chain;
    for (WORD i = 0; i < frame_count; ++i) {
        stack_chain << std::hex << "0x" << reinterpret_cast<uintptr_t>(stack[i]);
        if (i < frame_count - 1) {
            stack_chain << " -> ";
        }
    }
    LOG_ERROR("Crash Stack Chain: {}", stack_chain.str());

    SymCleanup(process);
}

void try_capture_cpp_exception() {
    try {
        std::rethrow_exception(std::current_exception());
    } catch (const std::exception &ex) {
        LOG_ERROR("Roblox Studio invoked RBXCRASH with C++ exception!");
        LOG_ERROR("C++ Exception details: '{}'", ex.what());
    } catch (...) {
        LOG_ERROR("Roblox Studio invoked RBXCRASH with unknown C++ exception type");
    }
}

void hooks::rbx_crash(const char *type, const char *message) {
    const auto function_name = "hooks::rbx_crash";

    const char *crash_type = (type != nullptr) ? type : "Unknown";
    const char *crash_description = (message != nullptr) ? message : "Not described";

    LOG_ERROR("=== ROBLOX CRASH TRIGGERED ===");
    LOG_ERROR("CRASH TYPE: {}", crash_type);
    LOG_ERROR("CRASH DESCRIPTION: {}", crash_description);

    const auto roblox_base = memory::module_utils::get_roblox_studio_base();
    const auto our_base = reinterpret_cast<uintptr_t>(g_hinstance);

    LOG_ERROR("RobloxStudioBeta.exe Base: 0x{:016X}", roblox_base);
    LOG_ERROR("roblox_modloader.dll Base: 0x{:016X}", our_base);

    try_capture_cpp_exception();

    log_crash_stack_trace();

    LOG_ERROR("=== END ROBLOX CRASH INFORMATION ===");
    LOG_ERROR("Crash handling completed. Please send the log file to the developer.");

    global_logger()->flush();

    const std::string message_text = std::format(
        "Roblox Studio has invoked RBXCRASH!\n\n"
        "Crash Type: {}\n"
        "Description: {}\n\n"
        "Detailed crash information has been logged.\n"
        "The application will continue after a brief pause.",
        crash_type, crash_description
    );

    MessageBoxA(nullptr,
                message_text.c_str(),
                "RobloxModLoader - Roblox Crash Detected",
                MB_OK | MB_ICONERROR);

    LOG_ERROR("Waiting 60 seconds before allowing Roblox to continue...");
    std::this_thread::sleep_for(std::chrono::seconds(60));

    LOG_INFO("Roblox crash handler completed, allowing execution to continue");
}
