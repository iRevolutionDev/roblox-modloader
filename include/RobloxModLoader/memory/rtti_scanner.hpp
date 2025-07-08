#pragma once

#include "RobloxModLoader/common.hpp"
#include "pe_parser.hpp"

#include <immintrin.h>

namespace memory::rtti {
    class scanner;
    class rtti_info;

    /**
     * @brief Type descriptor structure from MSVC RTTI
     */
    struct type_descriptor {
        void **type_info_vft;
        void **spare;
        char name[256];
    };

    /**
     * @brief Complete object locator structure
     */
    struct complete_object_locator {
        std::uint32_t signature;
        std::uint32_t offset;
        std::uint32_t constructor_displacement;
        pe::ibo32 type_descriptor_offset;
        pe::ibo32 class_hierarchy_offset;
    };

    /**
     * @brief Class hierarchy descriptor
     */
    struct class_hierarchy_descriptor {
        std::uint32_t signature;
        std::uint32_t attributes;
        std::uint32_t num_base_classes;
        pe::ibo32 base_class_array_offset;
    };

    /**
     * @brief Base class descriptor
     */
    struct base_class_descriptor {
        std::int32_t type_descriptor_offset;
        std::uint32_t num_contained_bases;
        std::int32_t member_displacement[3];
        std::uint32_t attributes;
        pe::ibo32 class_hierarchy_offset;
    };

    /**
     * @brief Section data container for RTTI scanning
     */
    struct section_data {
        pe::sections *text_sections;
        pe::sections *data_sections;
        pe::sections *rdata_sections;

        section_data(pe::sections *text, pe::sections *data, pe::sections *rdata)
            : text_sections(text), data_sections(data), rdata_sections(rdata) {
        }
    };

    /**
     * @brief RTTI information container
     */
    class rtti_info {
    public:
        rtti_info(void **vft,
                  complete_object_locator *col,
                  type_descriptor *td,
                  class_hierarchy_descriptor *chd,
                  base_class_descriptor *bcd) noexcept
            : m_virtual_function_table(vft)
              , m_complete_object_locator(col)
              , m_type_descriptor(td)
              , m_class_hierarchy_descriptor(chd)
              , m_base_class_descriptor(bcd) {
        }

        /**
         * @brief Get demangled class name
         * @return Demangled class name or empty string on failure
         */
        [[nodiscard]] std::string get_name() const;

        /**
         * @brief Get virtual function table pointer
         * @return VFT pointer
         */
        [[nodiscard]] void **get_virtual_function_table() const noexcept {
            return m_virtual_function_table;
        }

        /**
         * @brief Get complete object locator
         * @return COL pointer
         */
        [[nodiscard]] complete_object_locator *get_complete_object_locator() const noexcept {
            return m_complete_object_locator;
        }

        /**
         * @brief Get type descriptor
         * @return Type descriptor pointer
         */
        [[nodiscard]] type_descriptor *get_type_descriptor() const noexcept {
            return m_type_descriptor;
        }

        /**
         * @brief Demangle C++ symbol name using Windows API
         * @param mangled_name Mangled symbol name
         * @return Demangled name or empty string on failure
         */
        [[nodiscard]] static std::string demangle_name(const char *mangled_name);

    private:
        void **m_virtual_function_table;
        complete_object_locator *m_complete_object_locator;
        type_descriptor *m_type_descriptor;
        class_hierarchy_descriptor *m_class_hierarchy_descriptor;
        base_class_descriptor *m_base_class_descriptor;
    };

    /**
     * @brief RTTI Scanner using SIMD pattern matching
     */
    class scanner {
    public:
        scanner();

        ~scanner();

        scanner(const scanner &) = delete;

        scanner &operator=(const scanner &) = delete;

        scanner(scanner &&) = delete;

        scanner &operator=(scanner &&) = delete;

        /**
         * @brief Scan for RTTI information in the current process
         * @param process_info Optional process information override
         * @return true on successful scan
         */
        [[nodiscard]] bool scan(const std::shared_ptr<pe::process_info> &process_info = nullptr);

        /**
         * @brief Get RTTI information by class name
         * @param class_name Name of the class to find
         * @return Pointer to RTTI info or nullptr if not found
         */
        [[nodiscard]] static rtti_info *get_class_rtti(std::string_view class_name) noexcept;

        /**
         * @brief Get all discovered RTTI classes
         * @return Reference to RTTI map
         */
        [[nodiscard]] static const std::unordered_map<std::string, std::unique_ptr<rtti_info> > &
        get_all_classes() noexcept {
            return s_class_rtti_map;
        }

        /**
         * @brief Clear all cached RTTI information
         */
        static void clear_cache() noexcept;

    private:
        /**
         * @brief Set up section data for scanning
         * @return true on success
         */
        [[nodiscard]] bool setup_section_data();

        /**
         * @brief Scan for RTTI patterns using SIMD
         * @param base_address Process base address
         * @return Number of RTTI entries found
         */
        [[nodiscard]] std::size_t scan_rtti_patterns(std::uint8_t *base_address) const;

        /**
         * @brief Validate and process potential RTTI structure
         * @param col Complete object locator candidate
         * @param base_address Process base address
         * @return true if valid RTTI structure
         */
        [[nodiscard]] bool validate_and_process_rtti(
            complete_object_locator *col,
            std::uint8_t *base_address
        ) const;

        std::unique_ptr<pe::parser> m_pe_parser;
        std::unique_ptr<section_data> m_section_data;

        static const __m128i RTTI_PATTERN;
        static const __m128i RTTI_MASK;

        static inline std::unique_ptr<pe::parser> s_pe_parser{};
        static inline std::unordered_map<std::string, std::unique_ptr<rtti_info> > s_class_rtti_map{};
        static inline std::unique_ptr<section_data> s_section_data{};
    };

    class rtti_manager {
    public:
        rtti_manager();

        ~rtti_manager();

        rtti_manager(const rtti_manager &) = delete;

        rtti_manager &operator=(const rtti_manager &) = delete;

        rtti_manager(rtti_manager &&) = delete;

        rtti_manager &operator=(rtti_manager &&) = delete;

        /**
         * @brief Get the internal scanner
         * @return Reference to scanner
         */
        [[nodiscard]] scanner &get_scanner() const noexcept {
            return *m_scanner;
        }

        /**
         * @brief Get RTTI info by class name (convenience method)
         * @param class_name Name of class to find
         * @return Pointer to RTTI info or nullptr
         */
        [[nodiscard]] static rtti_info *get_class_rtti(const std::string_view class_name) noexcept {
            return scanner::get_class_rtti(class_name);
        }

    private:
        std::unique_ptr<scanner> m_scanner;
    };

    inline rtti_manager *g_rtti_manager{};
}
