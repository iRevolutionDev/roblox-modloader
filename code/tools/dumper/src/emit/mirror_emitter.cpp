#include "emit/mirror_emitter.hpp"

#include <format>
#include <set>

namespace rml::dumper::emit
{
	std::string MirrorEmitter::mirror_name(const std::string_view struct_name)
	{
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
		if (type == "Instruction*")
			return "std::uint32_t*";
		if (type == "const Instruction*")
			return "const std::uint32_t*";
		if (type == "unsigned")
			return "std::uint32_t";
		if (type == "unsigned short")
			return "std::uint16_t";
		if (type == "uint8_t")
			return "std::uint8_t";
		if (type == "int")
			return "std::int32_t";

		return type;
	}

	std::vector<MirrorEmitter::Slot> MirrorEmitter::pack(const schema::StructLayout& layout)
	{
		std::vector<Slot> slots;
		std::size_t cursor = 0;

		for (const auto& field : layout.fields)
		{
			if (field.offset > cursor)
				slots.push_back({std::format("reserved_{:x}", cursor), "std::byte", cursor,
				                 field.offset - cursor, true});

			slots.push_back({field.name, self_contained(field.type), field.offset, field.size, false});
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
		out << "namespace rml::luau\n{\n";

		std::set<std::string> referenced;
		for (const auto& [name, layout] : layouts.structs)
			for (const auto& field : layout.fields)
			{
				auto pointee = self_contained(field.type);
				while (!pointee.empty() && pointee.back() == '*')
					pointee.pop_back();

				if (pointee == field.type || pointee.empty())
					continue;
				if (pointee.starts_with("std::") || pointee == "void" || pointee == "char" || pointee == "bool")
					continue;

				if (pointee.starts_with("const "))
					pointee.erase(0, 6);
				if (pointee.starts_with("std::"))
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
			const auto slots = pack(layout);

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

			out << std::format("\tstatic_assert(sizeof({}) >= 0x{:X});\n\n", mirror, layout.size);
		}

		out << "}\n";

		return {};
	}
}
