#include "emit/shuffle_layout_emitter.hpp"

#include <algorithm>
#include <array>
#include <format>

namespace rml::dumper::emit
{
	static constexpr std::array<std::string_view, 22> lua_state_canonical{
	    "status", "activememcat", "isactive", "singlestep", "top", "base", "global", "ci", "stack_last", "stack",
	    "end_ci", "base_ci", "stacksize", "size_ci", "nCcalls", "baseCcalls", "cachedslot", "gt", "openupval",
	    "gclist", "namecall", "userdata"};

	static constexpr std::array<std::string_view, 6> call_info_canonical{"base",    "func",     "top",
	                                                                     "savedpc", "nresults", "flags"};

	static constexpr std::array<std::string_view, 6> closure_canonical{"isC",     "nupvalues", "stacksize",
	                                                                   "preload", "gclist",    "env"};

	static constexpr std::array<std::string_view, 29> proto_canonical{
	    "k",           "code",        "p",          "codeentry",    "execdata",      "exectarget",
	    "lineinfo",    "abslineinfo", "locvars",    "upvalues",     "source",        "debugname",
	    "debuginsn",   "typeinfo",    "userdata",   "gclist",       "sizecode",      "sizep",
	    "sizelocvars", "sizeupvalues", "sizek",     "sizelineinfo", "linegaplog2",   "linedefined",
	    "bytecodeid",  "sizetypeinfo", "feedbackvec", "feedbackvecsize", "funid"};

	std::span<const std::string_view> ShuffleLayoutEmitter::canonical_fields(const std::string_view struct_name)
	{
		if (struct_name == "lua_State")
			return lua_state_canonical;
		if (struct_name == "CallInfo")
			return call_info_canonical;
		if (struct_name == "Closure")
			return closure_canonical;
		if (struct_name == "Proto")
			return proto_canonical;

		return {};
	}

	std::expected<std::vector<std::size_t>, Error> ShuffleLayoutEmitter::permutation(
	    const schema::StructLayout& layout, const std::span<const std::string_view> canonical)
	{
		std::vector<std::pair<std::size_t, std::size_t>> ordered;
		ordered.reserve(canonical.size());

		for (std::size_t i = 0; i < canonical.size(); ++i)
		{
			const auto* field = layout.find(canonical[i]);
			if (field == nullptr)
				return std::unexpected(Error::make(ErrorCode::recovery, "{}.{} was never recovered", layout.name,
				                                   canonical[i]));

			ordered.emplace_back(field->offset, i);
		}

		std::ranges::sort(ordered);

		std::vector<std::size_t> result;
		result.reserve(ordered.size());
		for (const auto& [offset, index] : ordered)
			result.push_back(index);

		return result;
	}

	std::expected<void, Error> ShuffleLayoutEmitter::emit(const schema::LayoutSet& layouts, std::ostream& out) const
	{
		write_banner(out, layouts, "//");

		out << "#pragma once\n\n";

		for (const auto& [name, layout] : layouts.structs)
		{
			const auto canonical = canonical_fields(name);
			if (canonical.empty())
				continue;

			const auto order = permutation(layout, canonical);
			if (!order)
				return std::unexpected(order.error());

			out << std::format("// {} sizeof 0x{:X}\n", name, layout.size);
			out << "// canonical:";
			for (const auto& field : canonical)
				out << ' ' << field;
			out << '\n';

			out << std::format("#define RBX_SHUFFLE_{}(", name);
			for (std::size_t i = 0; i < canonical.size(); ++i)
				out << (i == 0 ? "" : ", ") << 'a' << i;
			out << ") \\\n    ";

			for (std::size_t i = 0; i < order->size(); ++i)
				out << (i == 0 ? "" : "; ") << 'a' << (*order)[i];

			out << "\n\n";
		}

		return {};
	}
}
