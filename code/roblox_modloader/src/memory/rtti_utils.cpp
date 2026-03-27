#include "RobloxModLoader/memory/rtti_utils.hpp"

#include <dbghelp.h>
#include <algorithm>

namespace memory::rtti::utils {
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
        if (!mangled_name) {
            return false;
        }

        // Quick length calculation without expensive memory checks
        size_t length = 0;
        const char *ptr = mangled_name;
        
        // Use strnlen_s for safer length calculation with bounds
        length = strnlen_s(mangled_name, config::MAX_NAME_LENGTH);
        
        if (length == 0 || length >= config::MAX_NAME_LENGTH) {
            return false;
        }

        out_length = length;
        return true;
    }
}
