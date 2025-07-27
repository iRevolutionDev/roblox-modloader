#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/rtti_scanner.hpp"
#include "RobloxModLoader/memory/rtti_utils.hpp"

#include <immintrin.h>
#include <dbghelp.h>
#include <algorithm>

#pragma comment(lib, "dbghelp.lib")

namespace memory::rtti {
    const __m128i scanner::RTTI_PATTERN = _mm_setr_epi8(
        0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, // lea rax,[rip+offset]
        0x48, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    );

    const __m128i scanner::RTTI_MASK = _mm_setr_epi8(
        0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // Mask for LEA
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    );

    std::string rtti_info::get_name() const {
        if (!m_type_descriptor || !m_type_descriptor->name) {
            return {};
        }
        return demangle_name(m_type_descriptor->name);
    }

    std::string rtti_info::demangle_name(const char *mangled_name) {
        if (!mangled_name) {
            return {};
        }

        if (!utils::is_symbol_safe_to_demangle(mangled_name)) {
            LOG_TRACE("Skipping unsafe symbol: {}", mangled_name);
            return {};
        }

        size_t name_length;
        if (!utils::validate_symbol_memory_access(mangled_name, name_length)) {
            LOG_DEBUG("Invalid symbol memory access for: {}", mangled_name);
            return {};
        }

        std::array<char, utils::config::OUTPUT_BUFFER_SIZE> output{};

        const char *name_to_process = mangled_name;
        if (mangled_name[0] == '.') {
            ++name_to_process;
        }

        constexpr DWORD flags = UNDNAME_NO_ARGUMENTS | UNDNAME_NAME_ONLY |
                                UNDNAME_32_BIT_DECODE | UNDNAME_NO_MS_KEYWORDS |
                                UNDNAME_NO_LEADING_UNDERSCORES;

        const DWORD result = utils::safe_undecorate_symbol(name_to_process, output.data(), output.size(), flags);
        if (result != ERROR_SUCCESS) {
            if (result != ERROR_INVALID_PARAMETER) {
                LOG_ERROR("Failed to demangle symbol: {} (error: {})", mangled_name, result);
            }
            return {};
        }

        output[output.size() - 1] = '\0';
        return output.data();
    }

    scanner::scanner() {
        m_pe_parser = std::make_unique<pe::parser>();
        if (!s_pe_parser) {
            s_pe_parser = std::make_unique<pe::parser>();
        }

        LOG_DEBUG("RTTI scanner created");
    }

    scanner::~scanner() {
        LOG_DEBUG("RTTI scanner destroyed");
    }

    bool scanner::scan(const std::shared_ptr<pe::process_info> &process_info) {
        LOG_INFO("Starting RTTI scan...");

        try {
            if (!m_pe_parser->parse(process_info)) {
                LOG_ERROR("Failed to parse PE structure");
                return false;
            }
            if (!setup_section_data()) {
                LOG_ERROR("Failed to setup section data");
                return false;
            }

            const auto proc_info = m_pe_parser->get_process_info();
            if (!proc_info || !proc_info->module_info) {
                LOG_ERROR("Invalid process information");
                return false;
            }

            auto *base_address = static_cast<std::uint8_t *>(proc_info->module_info->lpBaseOfDll);
            if (!base_address) {
                LOG_ERROR("Invalid base address");
                return false;
            }
            s_class_rtti_map.clear();

            const auto found_count = scan_rtti_patterns(base_address);

            LOG_INFO("RTTI scan completed. Found {} classes", found_count);
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("RTTI scan failed with exception: {}", e.what());
            return false;
        }
    }

    rtti_info *scanner::get_class_rtti(std::string_view class_name) noexcept {
        const auto it = s_class_rtti_map.find(std::string(class_name));
        return it != s_class_rtti_map.end() ? it->second.get() : nullptr;
    }

    void scanner::clear_cache() noexcept {
        s_class_rtti_map.clear();
        s_section_data.reset();
        s_pe_parser.reset();
        LOG_DEBUG("RTTI cache cleared");
    }

    bool scanner::setup_section_data() {
        auto *text_sections = m_pe_parser->get_sections_with_name(".text");
        auto *data_sections = m_pe_parser->get_sections_with_name(".data");
        auto *rdata_sections = m_pe_parser->get_sections_with_name(".rdata");

        if (!text_sections || !data_sections || !rdata_sections) {
            LOG_ERROR("Required sections not found (text: {}, data: {}, rdata: {})",
                      text_sections != nullptr, data_sections != nullptr, rdata_sections != nullptr);
            return false;
        }

        m_section_data = std::make_unique<section_data>(text_sections, data_sections, rdata_sections);
        s_section_data = std::make_unique<section_data>(text_sections, data_sections, rdata_sections);

        LOG_DEBUG("Section data setup complete");
        return true;
    }

    std::size_t scanner::scan_rtti_patterns(std::uint8_t *base_address) const {
        std::size_t found_count = 0;

        if (!m_section_data || !m_section_data->rdata_sections || !base_address) {
            LOG_ERROR("Invalid section data");
            return 0;
        }

        for (const auto &section: *m_section_data->rdata_sections) {
            if (!section) continue;

            LOG_DEBUG("Scanning section: {} (size: 0x{:X})", section->name, section->size);

            auto *start = section->start.as<complete_object_locator **>(base_address);
            auto *end = section->end.as<complete_object_locator **>(base_address);

            if (!start || !end || start >= end) {
                LOG_DEBUG("Invalid section bounds for: {}", section->name);
                continue;
            }

            const auto section_size = reinterpret_cast<std::uintptr_t>(end) - reinterpret_cast<std::uintptr_t>(start);
            if (section_size > 100 * 1024 * 1024) { // 100MB limit
                LOG_WARN("Section {} too large ({} bytes), skipping", section->name, section_size);
                continue;
            }

            for (auto *current = start; current < end; ++current) {
                auto *col = *current;
                if (!col) continue;

                if (validate_and_process_rtti(current, col, base_address)) {
                    ++found_count;
                }
            }
        }

        return found_count;
    }

    bool scanner::validate_and_process_rtti(complete_object_locator **pointer_col, complete_object_locator *col,
                                            std::uint8_t *base_address) const {
        if (!col || !m_section_data || !base_address || !pointer_col) {
            return false;
        }

        if (!pe::parser::is_address_in_section(col, m_section_data->rdata_sections)) {
            return false;
        }

        if (col->signature != 0 && col->signature != 1) {
            return false;
        }

        if (!pe::parser::is_ibo_in_section(col->type_descriptor_offset, m_section_data->data_sections)) {
            return false;
        }

        auto *type_desc = col->type_descriptor_offset.as<type_descriptor *>(base_address);
        if (!type_desc) {
            return false;
        }

        if (!type_desc->name) {
            return false;
        }

        if (!pe::parser::is_ibo_in_section(col->class_hierarchy_offset, m_section_data->rdata_sections)) {
            return false;
        }

        auto *class_hierarchy = col->class_hierarchy_offset.as<class_hierarchy_descriptor *>(base_address);
        if (!class_hierarchy) {
            return false;
        }

        if (class_hierarchy->signature != 0 && class_hierarchy->signature != 1) {
            return false;
        }

        if (class_hierarchy->num_base_classes > 100) {
            return false;
        }

        if (!pe::parser::is_ibo_in_section(class_hierarchy->base_class_array_offset,
                                           m_section_data->rdata_sections)) {
            return false;
        }

        auto *base_class = class_hierarchy->base_class_array_offset.as<base_class_descriptor *>(base_address);
        if (!base_class) {
            return false;
        }

        const std::string class_name = rtti_info::demangle_name(type_desc->name);
        if (class_name.empty()) {
            return false;
        }

        if (!*pointer_col) {
            LOG_DEBUG("Invalid dereferenced complete object locator for class: {}", class_name);
            return false;
        }

        auto *vft_ptr = reinterpret_cast<void **>(pointer_col) + 1;
        if (!vft_ptr) {
            LOG_DEBUG("Invalid VFT pointer for class: {}", class_name);
            return false;
        }

        auto rtti = std::make_unique<rtti_info>(
            vft_ptr,
            col,
            type_desc,
            class_hierarchy,
            base_class
        );

        s_class_rtti_map.emplace(class_name, std::move(rtti));

        LOG_TRACE("Found RTTI for class: {}", class_name);
        return true;
    }

    rtti_manager::rtti_manager() {
        g_rtti_manager = this;
        
        LOG_INFO("Initializing RTTI manager...");

        m_scanner = std::make_unique<scanner>();

        if (!m_scanner->scan()) {
            LOG_ERROR("Initial RTTI scan failed");
            throw std::runtime_error("Failed to initialize RTTI scanner");
        }

        LOG_INFO("RTTI manager initialized successfully");
    }

    rtti_manager::~rtti_manager() {
        LOG_INFO("Shutting down RTTI manager...");

        g_rtti_manager = nullptr;
        m_scanner.reset();
        scanner::clear_cache();

        LOG_INFO("RTTI manager shutdown complete");
    }
}
