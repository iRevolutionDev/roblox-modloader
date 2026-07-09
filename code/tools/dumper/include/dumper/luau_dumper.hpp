#pragma once

#include "assembly_analyzer.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace dumper
{
	class pointers;
	class AssemblyAnalyzer;

	struct FieldInfo
	{
		std::string name;
		std::size_t offset;
		std::size_t size;
		std::string type;
		int64_t adjustment = 0;
	};

	struct StructInfo
	{
		std::string name;
		std::size_t size;
		std::vector<FieldInfo> fields;
	};

	struct Proto
	{
		uint64_t linedefined;
		uint64_t execdata;
	};

	class LuauDumper
	{
	public:
		explicit LuauDumper(std::unique_ptr<pointers>& pointers, uintptr_t image_base);
		~LuauDumper() = default;

		bool analyze();

		const std::map<std::string, StructInfo>& get_structures() const;

		const StructInfo* find_structure(const std::string& name) const;

	private:
		std::map<std::string, StructInfo> m_structures;
		uintptr_t m_base_address;
		std::unique_ptr<pointers> m_pointers;

		void apply_common_header(StructInfo& structure) const;
		static bool has_field(const StructInfo& structure, std::string_view field_name, std::size_t offset);

		static std::size_t validate_offset(std::string_view field, uint64_t recovered, std::size_t expected, bool allow_zero = false);

		bool analyze_lua_state();
		bool analyze_common();
		bool analyze_table();
		bool analyze_closure();
		bool analyze_proto();
		void recover_proto_metadata(StructInfo& proto) const;
		bool analyze_upval();
		bool analyze_tvalue();
		bool analyze_global_state();

		bool generate_offsets();

	private:
		Proto m_proto;
	};

}
