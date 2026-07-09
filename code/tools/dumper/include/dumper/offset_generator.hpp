#pragma once

#include <filesystem>
#include <map>
#include <string>

namespace dumper
{

	struct StructInfo;

	enum class OutputFormat
	{
		CPP_Header,
		CPP_Struct,
		JSON,
		Genny,
		LuauShuffle
	};

	class OffsetGenerator
	{
	public:
		OffsetGenerator()  = default;
		~OffsetGenerator() = default;

		bool generate(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path, OutputFormat format = OutputFormat::CPP_Header);

	private:
		bool generate_cpp_header(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path);
		bool generate_cpp_structs(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path);

		static bool generate_json(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path);

		static bool generate_genny(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path);
		
		static bool generate_luau_shuffle(const std::map<std::string, StructInfo>& structures, const std::filesystem::path& output_path);

		std::string sanitize_name(const std::string& name);
	};

}
