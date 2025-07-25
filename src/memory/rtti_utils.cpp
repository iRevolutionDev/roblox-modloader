#include "RobloxModLoader/memory/rtti_utils.hpp"

#include <dbghelp.h>
#include <algorithm>

namespace memory::rtti::utils {
    bool is_memory_readable(const void *ptr, size_t size) noexcept {
        __try {
            const auto *test_ptr = static_cast<volatile char *>(const_cast<void *>(ptr));
            for (size_t i = 0; i < size; ++i) {
                [[maybe_unused]] const volatile char test = test_ptr[i];
            }
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    bool contains_problematic_patterns(std::string_view symbol) noexcept {
        return std::ranges::any_of(config::PROBLEMATIC_PATTERNS,
                                   [symbol](const std::string_view pattern) {
                                       return symbol.find(pattern) != std::string_view::npos;
                                   });
    }

    size_t calculate_template_depth(const std::string_view symbol) noexcept {
        size_t current_depth = 0;
        size_t max_depth = 0;
        for (const char c: symbol) {
            if (c == '<') {
                ++current_depth;
                max_depth = std::max(max_depth, current_depth);
            } else if (c == '>' && current_depth > 0) {
                --current_depth;
            }
        }
        return max_depth;
    }

    bool is_symbol_too_complex(std::string_view symbol) noexcept {
        return symbol.length() > config::MAX_SYMBOL_LENGTH ||
               calculate_template_depth(symbol) > config::MAX_TEMPLATE_DEPTH ||
               std::ranges::count(symbol, '@') > config::MAX_AT_SYMBOLS;
    }

    bool is_symbol_safe_to_demangle(const char *mangled_name) noexcept {
        if (!mangled_name) {
            return false;
        }

        const std::string_view symbol{mangled_name};
        return !contains_problematic_patterns(symbol) && !is_symbol_too_complex(symbol);
    }

    DWORD safe_undecorate_symbol(const char *name, char *output, DWORD size, DWORD flags) noexcept {
        __try {
            if (!is_symbol_safe_to_demangle(name)) {
                return ERROR_INVALID_PARAMETER;
            }
            return UnDecorateSymbolName(name, output, size, flags) ? ERROR_SUCCESS : GetLastError();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return ERROR_INVALID_PARAMETER;
        }
    }

    bool validate_symbol_memory_access(const char *mangled_name, size_t &out_length) noexcept {
        if (!is_memory_readable(mangled_name, 1)) {
            return false;
        }

        size_t length = 0;
        const char *ptr = mangled_name;
        while (length < config::MAX_NAME_LENGTH && is_memory_readable(ptr, 1) && *ptr != '\0') {
            ++ptr;
            ++length;
        }

        if (length == 0 || length >= config::MAX_NAME_LENGTH) {
            return false;
        }

        if (!is_memory_readable(mangled_name, length + 1)) {
            return false;
        }

        out_length = length;
        return true;
    }
}
