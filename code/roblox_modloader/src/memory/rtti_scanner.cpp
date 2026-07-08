#include "RobloxModLoader/internal/memory/rtti_scanner.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/symbol_resolver.hpp"

#include <algorithm>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace rml::memory::rtti
{
	std::string RTTIInfo::get_name() const
	{
		if (!m_type_descriptor || !m_type_descriptor->name)
		{
			return {};
		}
		return demangle_name(m_type_descriptor->name);
	}

	std::string RTTIInfo::demangle_name(const char* mangled_name)
	{
		if (!mangled_name)
		{
			LOG_TRACE("Null mangled name provided");
			return {};
		}

		MEMORY_BASIC_INFORMATION mbi{};
		if (VirtualQuery(mangled_name, &mbi, sizeof(mbi)) == 0)
		{
			LOG_DEBUG("VirtualQuery failed for mangled name pointer");
			return {};
		}
		const auto ptr_offset = reinterpret_cast<uintptr_t>(mangled_name) - reinterpret_cast<uintptr_t>(mbi.BaseAddress);
		const auto max_safe_length = mbi.RegionSize - ptr_offset;

		if (max_safe_length < 4)
		{
			LOG_DEBUG("Insufficient readable memory for symbol");
			return {};
		}

		const auto safe_max_len = std::min(max_safe_length, static_cast<size_t>(1024));
		const auto name_length = strnlen(mangled_name, safe_max_len);

		if (name_length == 0 || name_length >= safe_max_len)
		{
			LOG_DEBUG("Invalid symbol length: {}", name_length);
			return {};
		}

		if (name_length < 3 || (mangled_name[0] != '?' && mangled_name[0] != '.' && mangled_name[0] != '_'))
		{
			LOG_TRACE("Symbol doesn't match expected mangled name pattern");
			return {};
		}

		for (size_t i = 0; i < std::min(name_length, static_cast<size_t>(20)); ++i)
		{
			const char c = mangled_name[i];
			if (c == '\0')
				break;
			if (!std::isprint(static_cast<unsigned char>(c)) && c != '\0')
			{
				LOG_TRACE("Symbol contains non-printable characters");
				return {};
			}
		}

		const char* name_to_process = mangled_name;
		if (mangled_name[0] == '.')
		{
			++name_to_process;
		}

		std::string demangled = memory::demangle(name_to_process);
		if (demangled.empty())
		{
			LOG_TRACE("Failed to demangle symbol: {}", std::string(mangled_name, std::min(name_length, size_t(30))));
			return {};
		}

		if (demangled.size() > 500)
		{
			LOG_DEBUG("Demangled name suspiciously long: {} chars", demangled.size());
			return {};
		}

		return demangled;
	}

	Scanner::Scanner()
	{
		m_pe_parser = std::make_unique<pe::Parser>();

		LOG_DEBUG("RTTI scanner created");
	}

	Scanner::~Scanner()
	{
		LOG_DEBUG("RTTI scanner destroyed");
	}

	bool Scanner::scan(const std::shared_ptr<pe::ProcessInfo>& process_info)
	{
		LOG_INFO("Starting RTTI scan...");

		try
		{
			if (!m_pe_parser->parse(process_info))
			{
				LOG_ERROR("Failed to parse PE structure");
				return false;
			}
			if (!setup_section_data())
			{
				LOG_ERROR("Failed to setup section data");
				return false;
			}

			const auto proc_info = m_pe_parser->get_process_info();
			if (!proc_info || !proc_info->module_info)
			{
				LOG_ERROR("Invalid process information");
				return false;
			}

			auto* base_address = static_cast<std::uint8_t*>(proc_info->module_info->lpBaseOfDll);
			if (!base_address)
			{
				LOG_ERROR("Invalid base address");
				return false;
			}
			s_class_rtti_map.clear();

			const auto found_count = scan_rtti_patterns(base_address);

			LOG_INFO("RTTI scan completed. Found {} classes", found_count);
			return true;
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("RTTI scan failed with exception: {}", e.what());
			return false;
		}
	}

	RTTIInfo* Scanner::get_class_rtti(std::string_view class_name) noexcept
	{
		const auto it = s_class_rtti_map.find(std::string(class_name));
		return it != s_class_rtti_map.end() ? it->second.get() : nullptr;
	}

	void Scanner::clear_cache() noexcept
	{
		s_class_rtti_map.clear();
		LOG_DEBUG("RTTI cache cleared");
	}

	bool Scanner::setup_section_data()
	{
		auto* text_sections = m_pe_parser->get_sections_with_name(".text");
		auto* data_sections = m_pe_parser->get_sections_with_name(".data");
		auto* rdata_sections = m_pe_parser->get_sections_with_name(".rdata");

		if (!text_sections || !data_sections || !rdata_sections)
		{
			LOG_ERROR("Required sections not found (text: {}, data: {}, rdata: {})", text_sections != nullptr, data_sections != nullptr, rdata_sections != nullptr);
			return false;
		}

		m_section_data = std::make_unique<SectionData>(text_sections, data_sections, rdata_sections);

		LOG_DEBUG("Section data setup complete");
		return true;
	}

	std::size_t Scanner::scan_rtti_patterns(std::uint8_t* base_address) const
	{
		std::size_t found_count = 0;

		if (!m_section_data || !m_section_data->rdata_sections || !base_address)
		{
			LOG_ERROR("Invalid section data");
			return 0;
		}

		for (const auto& section : *m_section_data->rdata_sections)
		{
			if (!section)
				continue;

			LOG_DEBUG("Scanning section: {} (size: 0x{:X})", section->name, section->size);

			auto* start = section->start.as<CompleteObjectLocator**>(base_address);
			auto* end = section->end.as<CompleteObjectLocator**>(base_address);

			if (!start || !end || start >= end)
			{
				LOG_DEBUG("Invalid section bounds for: {}", section->name);
				continue;
			}

			const auto section_size = reinterpret_cast<std::uintptr_t>(end) - reinterpret_cast<std::uintptr_t>(start);
			if (section_size > 100 * 1024 * 1024)
			{ // 100MB limit
				LOG_WARN("Section {} too large ({} bytes), skipping", section->name, section_size);
				continue;
			}

			for (auto* current = start; current < end; ++current)
			{
				auto* col = *current;
				if (!col)
					continue;

				if (validate_and_process_rtti(current, col, base_address))
				{
					++found_count;
				}
			}
		}

		return found_count;
	}

	bool Scanner::validate_and_process_rtti(CompleteObjectLocator** pointer_col, CompleteObjectLocator* col, std::uint8_t* base_address) const
	{
		if (!col || !m_section_data || !base_address || !pointer_col)
		{
			return false;
		}

		if (!pe::Parser::is_address_in_section(col, m_section_data->rdata_sections))
		{
			return false;
		}

		if (col->signature != 0 && col->signature != 1)
		{
			return false;
		}

		if (!pe::Parser::is_ibo_in_section(col->type_descriptor_offset, m_section_data->data_sections))
		{
			return false;
		}

		auto* type_desc = col->type_descriptor_offset.as<TypeDescriptor*>(base_address);
		if (!type_desc)
		{
			return false;
		}

		if (!type_desc->name)
		{
			return false;
		}

		if (!pe::Parser::is_ibo_in_section(col->class_hierarchy_offset, m_section_data->rdata_sections))
		{
			return false;
		}

		auto* class_hierarchy = col->class_hierarchy_offset.as<ClassHierarchyDescriptor*>(base_address);
		if (!class_hierarchy)
		{
			return false;
		}

		if (class_hierarchy->signature != 0 && class_hierarchy->signature != 1)
		{
			return false;
		}

		if (class_hierarchy->num_base_classes > 100)
		{
			return false;
		}

		if (!pe::Parser::is_ibo_in_section(class_hierarchy->base_class_array_offset, m_section_data->rdata_sections))
		{
			return false;
		}

		auto* base_class = class_hierarchy->base_class_array_offset.as<BaseClassDescriptor*>(base_address);
		if (!base_class)
		{
			return false;
		}

		const std::string class_name = RTTIInfo::demangle_name(type_desc->name);
		if (class_name.empty())
		{
			return false;
		}

		if (!*pointer_col)
		{
			LOG_DEBUG("Invalid dereferenced complete object locator for class: {}", class_name);
			return false;
		}

		auto* vft_ptr = reinterpret_cast<void**>(pointer_col) + 1;
		if (!vft_ptr)
		{
			LOG_DEBUG("Invalid VFT pointer for class: {}", class_name);
			return false;
		}

		auto rtti = std::make_unique<RTTIInfo>(vft_ptr, col, type_desc, class_hierarchy, base_class);

		s_class_rtti_map.emplace(class_name, std::move(rtti));

		LOG_TRACE("Found RTTI for class: {}", class_name);
		return true;
	}

	RTTIManager::RTTIManager()
	{
		g_rtti_manager = this;

		LOG_INFO("Initializing RTTI manager...");

		m_scanner = std::make_unique<Scanner>();

		if (!m_scanner->scan())
		{
			LOG_ERROR("Initial RTTI scan failed");
			throw std::runtime_error("Failed to initialize RTTI scanner");
		}

		LOG_INFO("RTTI manager initialized successfully");
	}

	RTTIManager::~RTTIManager()
	{
		LOG_INFO("Shutting down RTTI manager...");

		g_rtti_manager = nullptr;
		m_scanner.reset();
		Scanner::clear_cache();

		LOG_INFO("RTTI manager shutdown complete");
	}
}
