#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/handle.hpp"

namespace rml::memory::pe
{
	/**
     * @brief Integer Base Offset - represents offset from process base address
     */
	class RML_EXPORT IBO32
	{
	public:
		constexpr IBO32() noexcept = default;

		constexpr explicit IBO32(std::int32_t offset) noexcept :
		    m_value(offset)
		{
		}

		template<typename T>
		explicit IBO32(T* address) noexcept
		{
			const auto base = get_process_base();
			m_value = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(address) - reinterpret_cast<std::uintptr_t>(base));
		}

		template<typename T, typename Base>
		IBO32(T* address, Base* base) noexcept
		{
			m_value = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(address) - reinterpret_cast<std::uintptr_t>(base));
		}

		template<typename T = std::uint8_t*>
		[[nodiscard]] T as() const noexcept
		{
			const auto base = get_process_base();
			return reinterpret_cast<T>(static_cast<std::uint8_t*>(base) + m_value);
		}

		template<typename T, typename Base>
		[[nodiscard]] T as(Base* base) const noexcept
		{
			return reinterpret_cast<T>(reinterpret_cast<std::uint8_t*>(base) + m_value);
		}

		[[nodiscard]] constexpr std::int32_t value() const noexcept
		{
			return m_value;
		}

		constexpr auto operator<=>(const IBO32&) const noexcept = default;

	private:
		[[nodiscard]] static void* get_process_base() noexcept
		{
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
	struct Section
	{
		std::string name;
		std::size_t size;
		IBO32 start;
		IBO32 end;

		Section(std::string_view section_name, std::size_t section_size, IBO32 section_start, IBO32 section_end) :
		    name(section_name),
		    size(section_size),
		    start(section_start),
		    end(section_end)
		{
		}
	};

	using sections = std::vector<std::unique_ptr<Section> >;

	/**
     * @brief Process information container
     */
	struct ProcessInfo
	{
		HANDLE process_handle;
		HMODULE process_module;
		std::unique_ptr<MODULEINFO> module_info;

		ProcessInfo() :
		    process_handle(nullptr),
		    process_module(nullptr)
		{
		}
	};

	/**
     * @brief PE Parser for analyzing executable structure
     */
	class Parser
	{
	public:
		Parser() = default;

		~Parser() = default;

		Parser(const Parser&) = delete;

		Parser& operator=(const Parser&) = delete;

		Parser(Parser&&) = default;

		Parser& operator=(Parser&&) = default;

		/**
         * @brief Parse PE headers and sections
         * @param info Optional process info override
         * @return true on success
         */
		[[nodiscard]] bool parse(std::shared_ptr<ProcessInfo> info = nullptr);

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
		[[nodiscard]] static bool is_address_in_section(T* address, const sections* target_sections) noexcept
		{
			if (!target_sections || !address)
				return false;

			const auto addr_value = reinterpret_cast<std::uintptr_t>(address);
			return std::ranges::any_of(*target_sections, [addr_value](const auto& section) {
				if (!section)
					return false;

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
		[[nodiscard]] static bool is_ibo_in_section(IBO32 offset, const sections* target_sections) noexcept;

		/**
         * @brief Get current process information
         * @return Shared pointer to process info
         */
		[[nodiscard]] std::shared_ptr<ProcessInfo> get_process_info() const noexcept
		{
			return m_process_info;
		}

		/**
         * @brief Set process information
         * @param info Process info to set
         * @return Shared pointer to process info
         */
		[[nodiscard]] static std::shared_ptr<ProcessInfo> set_process_info(std::shared_ptr<ProcessInfo> info = nullptr);

	private:
		[[nodiscard]] bool parse_sections();

		static inline std::shared_ptr<ProcessInfo> s_process_info{};
		std::shared_ptr<ProcessInfo> m_process_info{};
		std::unordered_map<std::string, sections> m_section_map{};
	};
}
