#include "emit/mirror_emitter.hpp"

#include <format>
#include <set>

namespace rml::dumper::emit
{
	static std::string_view free_of_luau_macros(const std::string_view struct_name)
	{
		return struct_name == "CommonHeader" ? "GcHeader" : struct_name;
	}

	std::string MirrorEmitter::mirror_name(const std::string_view name_in_luau)
	{
		const auto struct_name = free_of_luau_macros(name_in_luau);

		std::string name;
		bool capitalise = true;

		for (const auto character : struct_name)
		{
			if (character == '_')
			{
				capitalise = true;
				continue;
			}

			name.push_back(capitalise ? static_cast<char>(std::toupper(character)) : character);
			capitalise = false;
		}

		return name;
	}

	static std::string self_contained(const std::string& type)
	{
		if (type == "StkId")
			return "TValue*";

		std::string core = type;
		std::string suffix;

		while (!core.empty() && core.back() == '*')
		{
			suffix.push_back('*');
			core.pop_back();
		}

		std::string prefix;
		if (core.starts_with("const "))
		{
			prefix = "const ";
			core.erase(0, 6);
		}

		if (core == "Value")
			core = "std::uint64_t";
		else if (core == "Instruction" || core == "unsigned")
			core = "std::uint32_t";
		else if (core == "unsigned short")
			core = "std::uint16_t";
		else if (core == "uint8_t")
			core = "std::uint8_t";
		else if (core == "int")
			core = "std::int32_t";

		return prefix + core + suffix;
	}

	static std::string under_its_mirror_name(const std::string& type, const schema::LayoutSet& layouts)
	{
		std::string core = type;
		std::string suffix;

		while (!core.empty() && core.back() == '*')
		{
			suffix.push_back('*');
			core.pop_back();
		}

		std::string prefix;
		if (core.starts_with("const "))
		{
			prefix = "const ";
			core.erase(0, 6);
		}

		if (layouts.find(core) == nullptr)
			return type;

		return prefix + MirrorEmitter::mirror_name(core) + suffix;
	}

	std::vector<MirrorEmitter::Slot> MirrorEmitter::pack(const schema::StructLayout& layout,
	                                                     const schema::LayoutSet& layouts)
	{
		std::vector<Slot> slots;
		std::size_t cursor = 0;

		for (const auto& field : layout.fields)
		{
			if (field.offset > cursor)
				slots.push_back({std::format("reserved_{:x}", cursor), "std::byte", cursor,
				                 field.offset - cursor, true});

			slots.push_back({field.name, self_contained(under_its_mirror_name(field.type, layouts)), field.offset,
			                 field.size, false});
			cursor = field.end();
		}

		if (layout.size > cursor)
			slots.push_back({std::format("reserved_{:x}", cursor), "std::byte", cursor, layout.size - cursor,
			                 true});

		return slots;
	}

	std::expected<void, Error> MirrorEmitter::emit(const schema::LayoutSet& layouts, std::ostream& out) const
	{
		write_banner(out, layouts, "//");

		out << "#pragma once\n\n";
		out << "#include <cstddef>\n#include <cstdint>\n\n";
		out << "namespace rml::luau::mirror\n{\n";

		std::set<std::string> referenced;
		for (const auto& [name, layout] : layouts.structs)
			for (const auto& field : layout.fields)
			{
				auto pointee = self_contained(under_its_mirror_name(field.type, layouts));
				while (!pointee.empty() && pointee.back() == '*')
					pointee.pop_back();

				if (pointee == field.type || pointee.empty())
					continue;

				if (pointee.starts_with("const "))
					pointee.erase(0, 6);

				if (pointee.starts_with("std::") || pointee == "void" || pointee == "char" || pointee == "bool")
					continue;

				referenced.insert(pointee);
			}

		for (const auto& type : referenced)
			out << std::format("\tstruct {};\n", type);

		if (!referenced.empty())
			out << '\n';

		for (const auto& [name, layout] : layouts.structs)
		{
			if (layout.fields.empty())
				continue;

			const auto mirror = mirror_name(name);
			const auto slots = pack(layout, layouts);

			out << std::format("\tstruct {}\n\t{{\n", mirror);

			for (const auto& slot : slots)
			{
				if (slot.reserved)
					out << std::format("\t\tstd::byte {}[0x{:X}];\n", slot.name, slot.size);
				else
					out << std::format("\t\t{} {};\n", slot.type, slot.name);
			}

			out << "\t};\n\n";

			for (const auto& slot : slots)
			{
				if (slot.reserved)
					continue;

				out << std::format("\tstatic_assert(offsetof({}, {}) == 0x{:X});\n", mirror, slot.name,
				                   slot.offset);
			}

			out << std::format("\tstatic_assert(sizeof({}) >= 0x{:X});\n", mirror, layout.size);

			std::string constant;
			for (const auto character : mirror)
				constant.push_back(static_cast<char>(std::tolower(character)));

			out << std::format("\tinline constexpr std::size_t {}_size = 0x{:X};\n\n", constant, layout.size);
		}

		out << "}\n";

		return {};
	}
}
