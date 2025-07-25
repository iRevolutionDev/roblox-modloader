#pragma once

#include "RobloxModLoader/common.hpp"

#include <string_view>
#include <array>

namespace memory::rtti::utils {
    namespace config {
        constexpr size_t MAX_SYMBOL_LENGTH = 500;
        constexpr size_t MAX_TEMPLATE_DEPTH = 10;
        constexpr size_t MAX_AT_SYMBOLS = 20;
        constexpr size_t MAX_NAME_LENGTH = 512;
        constexpr size_t OUTPUT_BUFFER_SIZE = 1024;

        constexpr std::array PROBLEMATIC_PATTERNS = {
            std::string_view{"<lambda_"},
            std::string_view{"_Func_impl_no_alloc"},
            std::string_view{"clone_impl"},
            std::string_view{"sp_counted_impl"},
            std::string_view{"shared_state"}
        };
    }

    [[nodiscard]] bool is_memory_readable(const void *ptr, size_t size) noexcept;

    [[nodiscard]] bool contains_problematic_patterns(std::string_view symbol) noexcept;

    [[nodiscard]] size_t calculate_template_depth(std::string_view symbol) noexcept;

    [[nodiscard]] bool is_symbol_too_complex(std::string_view symbol) noexcept;

    [[nodiscard]] bool is_symbol_safe_to_demangle(const char *mangled_name) noexcept;

    [[nodiscard]] DWORD safe_undecorate_symbol(const char *name, char *output, DWORD size, DWORD flags) noexcept;

    [[nodiscard]] bool validate_symbol_memory_access(const char *mangled_name, size_t &out_length) noexcept;
}
