#pragma once

#include "RobloxModLoader/common.hpp"
#include "handle.hpp"

namespace memory::pe {
    /**
     * @brief Integer Base Offset - represents offset from process base address
     */
    class ibo32 {
    public:
        constexpr ibo32() noexcept = default;
        constexpr explicit ibo32(std::int32_t offset) noexcept : m_value(offset) {}
        
        template<typename T>
        explicit ibo32(T* address) noexcept {
            const auto base = get_process_base();
            m_value = static_cast<std::int32_t>(
                reinterpret_cast<std::uintptr_t>(address) - 
                reinterpret_cast<std::uintptr_t>(base)
            );
        }
        
        template<typename T, typename Base>
        ibo32(T* address, Base* base) noexcept {
            m_value = static_cast<std::int32_t>(
                reinterpret_cast<std::uintptr_t>(address) - 
                reinterpret_cast<std::uintptr_t>(base)
            );
        }
        
        template<typename T = std::uint8_t*>
        [[nodiscard]] T as() const noexcept {
            const auto base = get_process_base();
            return reinterpret_cast<T>(
                static_cast<std::uint8_t*>(base) + m_value
            );
        }
        
        template<typename T, typename Base>
        [[nodiscard]] T as(Base* base) const noexcept {
            return reinterpret_cast<T>(
                reinterpret_cast<std::uint8_t*>(base) + m_value
            );
        }
        
        [[nodiscard]] constexpr std::int32_t value() const noexcept {
            return m_value;
        }
        
        constexpr auto operator<=>(const ibo32&) const noexcept = default;
        
    private:
        [[nodiscard]] static void* get_process_base() noexcept {
            static void* base = [] {
                return GetModuleHandleA(nullptr);
            }();
            return base;
        }
        
        std::int32_t m_value = 0;
    };
    
    /**
     * @brief PE Section information
     */
    struct section {
        std::string name;
        std::size_t size;
        ibo32 start;
        ibo32 end;
        
        section(std::string_view section_name, std::size_t section_size,
                ibo32 section_start, ibo32 section_end)
            : name(section_name), size(section_size), 
              start(section_start), end(section_end) {}
    };
    
    using sections = std::vector<std::unique_ptr<section>>;
    
    /**
     * @brief Process information container
     */
    struct process_info {
        HANDLE process_handle;
        HMODULE process_module;
        std::unique_ptr<MODULEINFO> module_info;
        
        process_info() : process_handle(nullptr), process_module(nullptr) {}
    };
    
    /**
     * @brief PE Parser for analyzing executable structure
     */
    class parser {
    public:
        parser() = default;
        ~parser() = default;
        
        parser(const parser&) = delete;
        parser& operator=(const parser&) = delete;
        parser(parser&&) = default;
        parser& operator=(parser&&) = default;
        
        /**
         * @brief Parse PE headers and sections
         * @param info Optional process info override
         * @return true on success
         */
        [[nodiscard]] bool parse(std::shared_ptr<process_info> info = nullptr);
        
        /**
         * @brief Get sections with specific name
         * @param name Section name to search for
         * @return Pointer to sections vector or nullptr
         */
        [[nodiscard]] sections* get_sections_with_name(std::string_view name) noexcept;
        
        /**
         * @brief Check if address is within given sections
         * @param address Address to check
         * @param target_sections Sections to check against
         * @return true if address is in any section
         */
        template<typename T>
        [[nodiscard]] static bool is_address_in_section(T* address, const sections* target_sections) noexcept {
            if (!target_sections) return false;
            
            const auto addr_value = reinterpret_cast<std::uintptr_t>(address);
            return std::ranges::any_of(*target_sections, [addr_value](const auto& section) {
                const auto start_addr = section->start.template as<std::uintptr_t>();
                const auto end_addr = section->end.template as<std::uintptr_t>();
                return addr_value >= start_addr && addr_value < end_addr;
            });
        }
        
        /**
         * @brief Check if IBO is within given sections
         * @param offset IBO to check
         * @param target_sections Sections to check against  
         * @return true if IBO is in any section
         */
        [[nodiscard]] static bool is_ibo_in_section(ibo32 offset, const sections* target_sections) noexcept;
        
        /**
         * @brief Get current process information
         * @return Shared pointer to process info
         */
        [[nodiscard]] std::shared_ptr<process_info> get_process_info() const noexcept {
            return m_process_info;
        }
        
        /**
         * @brief Set process information
         * @param info Process info to set
         * @return Shared pointer to process info
         */
        [[nodiscard]] static std::shared_ptr<process_info> set_process_info(
            std::shared_ptr<process_info> info = nullptr
        );
        
    private:
        [[nodiscard]] bool parse_sections();
        
        static inline std::shared_ptr<process_info> s_process_info{};
        std::shared_ptr<process_info> m_process_info{};
        std::unordered_map<std::string, sections> m_section_map{};
    };
}
