#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/pe_parser.hpp"

namespace memory::pe {
    bool parser::parse(std::shared_ptr<process_info> info) {
        try {
            m_process_info = info ? info : set_process_info();
            if (!m_process_info) {
                LOG_ERROR("Failed to get process information");
                return false;
            }

            m_section_map.clear();
            return parse_sections();
        } catch (const std::exception &e) {
            LOG_ERROR("PE parsing failed: {}", e.what());
            return false;
        }
    }

    sections *parser::get_sections_with_name(std::string_view name) noexcept {
        const auto it = m_section_map.find(std::string(name));
        return it != m_section_map.end() ? &it->second : nullptr;
    }

    bool parser::is_ibo_in_section(ibo32 offset, const sections *target_sections) noexcept {
        if (!target_sections) return false;

        return std::ranges::any_of(*target_sections, [&offset](const auto &section) {
            return offset >= section->start && offset < section->end;
        });
    }

    std::shared_ptr<process_info> parser::set_process_info(std::shared_ptr<process_info> info) {
        if (info) {
            s_process_info = std::move(info);
            return s_process_info;
        }

        const auto new_info = std::make_shared<process_info>();
        new_info->process_handle = ::GetCurrentProcess();
        new_info->process_module = ::GetModuleHandleA(nullptr);
        new_info->module_info = std::make_unique<MODULEINFO>();

        if (!::GetModuleInformation(
            new_info->process_handle,
            new_info->process_module,
            new_info->module_info.get(),
            sizeof(MODULEINFO)
        )) {
            LOG_ERROR("Failed to get module information: {}", ::GetLastError());
            return nullptr;
        }

        s_process_info = new_info;
        return s_process_info;
    }

    bool parser::parse_sections() {
        if (!m_process_info || !m_process_info->module_info) {
            LOG_ERROR("Invalid process information");
            return false;
        }

        const auto base = static_cast<std::uint8_t *>(m_process_info->module_info->lpBaseOfDll);
        if (!base) {
            LOG_ERROR("Invalid module base address");
            return false;
        }

        const auto dos_header = reinterpret_cast<const IMAGE_DOS_HEADER *>(base);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
            LOG_ERROR("Invalid DOS signature");
            return false;
        }

        const auto nt_headers = reinterpret_cast<const IMAGE_NT_HEADERS *>(
            base + dos_header->e_lfanew
        );
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
            LOG_ERROR("Invalid NT signature");
            return false;
        }

        const auto section_headers = reinterpret_cast<const IMAGE_SECTION_HEADER *>(
            reinterpret_cast<const std::uint8_t *>(&nt_headers->OptionalHeader) +
            nt_headers->FileHeader.SizeOfOptionalHeader
        );

        for (std::uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
            const auto &section_header = section_headers[i];

            std::string section_name;
            section_name.reserve(IMAGE_SIZEOF_SHORT_NAME + 1);
            for (std::size_t j = 0; j < IMAGE_SIZEOF_SHORT_NAME && section_header.Name[j]; ++j) {
                section_name += static_cast<char>(section_header.Name[j]);
            }

            const auto virtual_address = section_header.VirtualAddress;
            const auto virtual_size = section_header.Misc.VirtualSize;

            const ibo32 start_offset(static_cast<std::int32_t>(virtual_address));
            const ibo32 end_offset(static_cast<std::int32_t>(virtual_address + virtual_size));

            auto section_info = std::make_unique<section>(
                section_name, virtual_size, start_offset, end_offset
            );

            m_section_map[section_name].emplace_back(std::move(section_info));

            LOG_DEBUG("Parsed section: {} (VA: 0x{:X}, Size: 0x{:X})",
                      section_name, virtual_address, virtual_size);
        }

        LOG_INFO("Successfully parsed {} sections", m_section_map.size());
        return true;
    }
}
