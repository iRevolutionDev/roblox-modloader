#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "pe_parser.hpp"

namespace rml::memory::rtti
{
	class Scanner;
	class RTTIInfo;

	/**
     * @brief Type descriptor structure from MSVC RTTI
     */
	struct TypeDescriptor
	{
		void** type_info_vft;
		void** spare;
		char name[256];
	};

	/**
     * @brief Complete object locator structure
     */
	struct CompleteObjectLocator
	{
		std::uint32_t signature;
		std::uint32_t offset;
		std::uint32_t constructor_displacement;
		pe::IBO32 type_descriptor_offset;
		pe::IBO32 class_hierarchy_offset;
	};

	/**
     * @brief Class hierarchy descriptor
     */
	struct ClassHierarchyDescriptor
	{
		std::uint32_t signature;
		std::uint32_t attributes;
		std::uint32_t num_base_classes;
		pe::IBO32 base_class_array_offset;
	};

	/**
     * @brief Base class descriptor
     */
	struct BaseClassDescriptor
	{
		std::int32_t type_descriptor_offset;
		std::uint32_t num_contained_bases;
		std::int32_t member_displacement[3];
		std::uint32_t attributes;
		pe::IBO32 class_hierarchy_offset;
	};

	/**
     * @brief Section data container for RTTI scanning
     */
	struct SectionData
	{
		pe::sections* text_sections;
		pe::sections* data_sections;
		pe::sections* rdata_sections;

		SectionData(pe::sections* text, pe::sections* data, pe::sections* rdata) :
		    text_sections(text),
		    data_sections(data),
		    rdata_sections(rdata)
		{
		}
	};

	/**
     * @brief RTTI information container
     */
	class RTTIInfo
	{
	public:
		RTTIInfo(void** vft, CompleteObjectLocator* col, TypeDescriptor* td, ClassHierarchyDescriptor* chd, BaseClassDescriptor* bcd) noexcept :
		    m_virtual_function_table(vft),
		    m_complete_object_locator(col),
		    m_type_descriptor(td),
		    m_class_hierarchy_descriptor(chd),
		    m_base_class_descriptor(bcd)
		{
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
		[[nodiscard]] void** get_virtual_function_table() const noexcept
		{
			return m_virtual_function_table;
		}

		/**
         * @brief Get complete object locator
         * @return COL pointer
         */
		[[nodiscard]] CompleteObjectLocator* get_complete_object_locator() const noexcept
		{
			return m_complete_object_locator;
		}

		/**
         * @brief Get type descriptor
         * @return Type descriptor pointer
         */
		[[nodiscard]] TypeDescriptor* get_type_descriptor() const noexcept
		{
			return m_type_descriptor;
		}

		/**
         * @brief Demangle C++ symbol name using Windows API
         * @param mangled_name Mangled symbol name
         * @return Demangled name or empty string on failure
         */
		[[nodiscard]] static std::string demangle_name(const char* mangled_name);

		/**
         * @brief Get class hierarchy descriptor
         * @return Class hierarchy descriptor pointer
         */
		[[nodiscard]] ClassHierarchyDescriptor* get_class_hierarchy_descriptor() const noexcept
		{
			return m_class_hierarchy_descriptor;
		}

		/**
         * @brief Get base class descriptor
         * @return Base class descriptor pointer
         */
		[[nodiscard]] BaseClassDescriptor* get_base_class_descriptor() const noexcept
		{
			return m_base_class_descriptor;
		}

	private:
		void** m_virtual_function_table;
		CompleteObjectLocator* m_complete_object_locator;
		TypeDescriptor* m_type_descriptor;
		ClassHierarchyDescriptor* m_class_hierarchy_descriptor;
		BaseClassDescriptor* m_base_class_descriptor;
	};

	/**
     * @brief RTTI scanner using a pointer walk over the module's .rdata section
     */
	class Scanner
	{
	public:
		Scanner();

		~Scanner();

		Scanner(const Scanner&) = delete;

		Scanner& operator=(const Scanner&) = delete;

		Scanner(Scanner&&) = delete;

		Scanner& operator=(Scanner&&) = delete;

		/**
         * @brief Scan for RTTI information in the current process
         * @param process_info Optional process information override
         * @return true on successful scan
         */
		[[nodiscard]] bool scan(const std::shared_ptr<pe::ProcessInfo>& process_info = nullptr);

		/**
         * @brief Get RTTI information by class name
         * @param class_name Name of the class to find
         * @return Pointer to RTTI info or nullptr if not found
         */
		[[nodiscard]] static RTTIInfo* get_class_rtti(std::string_view class_name) noexcept;

		/**
         * @brief Get all discovered RTTI classes
         * @return Reference to RTTI map
         */
		[[nodiscard]] static const std::unordered_map<std::string, std::unique_ptr<RTTIInfo> >& get_all_classes() noexcept
		{
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
         * @brief Scan for RTTI patterns via pointer walk
         * @param base_address Process base address
         * @return Number of RTTI entries found
         */
		[[nodiscard]] std::size_t scan_rtti_patterns(std::uint8_t* base_address) const;

		/**
         * @brief Validate and process potential RTTI structure
         * @param pointer_col Pointer to complete object locator candidate
         * @param col Complete object locator candidate
         * @param base_address Process base address
         * @return true if valid RTTI structure
         */
		[[nodiscard]] bool validate_and_process_rtti(CompleteObjectLocator** pointer_col, CompleteObjectLocator* col, std::uint8_t* base_address) const;

		std::unique_ptr<pe::Parser> m_pe_parser;
		std::unique_ptr<SectionData> m_section_data;

		static inline std::unordered_map<std::string, std::unique_ptr<RTTIInfo> > s_class_rtti_map{};
	};

	class RTTIManager
	{
	public:
		RTTIManager();

		~RTTIManager();

		RTTIManager(const RTTIManager&) = delete;

		RTTIManager& operator=(const RTTIManager&) = delete;

		RTTIManager(RTTIManager&&) = delete;

		RTTIManager& operator=(RTTIManager&&) = delete;

		/**
         * @brief Get the internal scanner
         * @return Reference to scanner
         */
		[[nodiscard]] Scanner& get_scanner() const noexcept
		{
			return *m_scanner;
		}

		/**
         * @brief Get RTTI info by class name (convenience method)
         * @param class_name Name of class to find
         * @return Pointer to RTTI info or nullptr
         */
		[[nodiscard]] static RTTIInfo* get_class_rtti(const std::string_view class_name) noexcept
		{
			return Scanner::get_class_rtti(class_name);
		}

	private:
		std::unique_ptr<Scanner> m_scanner;
	};

	inline RTTIManager* g_rtti_manager{};
}
