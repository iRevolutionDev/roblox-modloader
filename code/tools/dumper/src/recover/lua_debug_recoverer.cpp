#include "recover/lua_debug_recoverer.hpp"

#include <algorithm>
#include <array>
#include <format>

namespace rml::dumper::recover
{
	std::expected<schema::StructLayout, Error> LuaDebugRecoverer::recover(const RecoveryContext& context) const
	{
		const auto trace = context.trace(target::Anchor::lua_getinfo);
		if (!trace)
			return std::unexpected(trace.error());

		schema::StructLayout layout{.name = "lua_Debug"};

		const disasm::TraceQuery query(**trace);
		const auto record = query.dominant_object();

		struct Slot
		{
			std::string_view name;
			std::string_view type;
			std::uint8_t width;
			std::size_t position;
			std::string_view probe;
		};

		static constexpr std::array<Slot, 9> filled_in_order{{
		    {"source", "const char*", 8, 0, "first pointer the source case stores"},
		    {"what", "const char*", 8, 1, "second pointer the source case stores"},
		    {"short_src", "const char*", 8, 2, "last pointer the source case stores"},
		    {"name", "const char*", 8, 3, "the pointer left for the name case"},
		    {"linedefined", "int", 4, 0, "first dword, stored beside the source strings"},
		    {"currentline", "int", 4, 1, "the dword the line case stores on its own"},
		    {"nupvals", "uint8_t", 1, 0, "first byte, stored by the upvalue case"},
		    {"isvararg", "uint8_t", 1, 1, "second byte, the one the c branch sets to one"},
		    {"nparams", "uint8_t", 1, 2, "the byte left for the parameter count"},
		}};

		for (const auto& slot : filled_in_order)
		{
			const auto* access = query.nth_distinct_write(record, slot.position, slot.width);
			if (access == nullptr || access->displacement < 0)
			{
				context.report().record_failure("lua_Debug", std::string(slot.name), std::string(slot.probe));
				continue;
			}

			context.report().record_recovered("lua_Debug", std::string(slot.name), std::string(slot.probe));
			layout.add({.name = std::string(slot.name),
			            .type = std::string(slot.type),
			            .size = slot.width,
			            .offset = static_cast<std::size_t>(access->displacement),
			            .provenance = schema::Provenance::recovered("lua_getinfo", std::string(slot.probe))});
		}

		const auto* always_vararg = query.first_immediate_write(record, 1);
		const auto* isvararg = layout.find("isvararg");

		if (always_vararg == nullptr || isvararg == nullptr ||
		    static_cast<std::size_t>(always_vararg->displacement) != isvararg->offset)
		{
			context.report().record_failure(
			    "lua_Debug", "isvararg",
			    "the byte a c function always gets set to one is not the one taken for the vararg flag");
		}

		layout.size = layout.fields.empty() ? 0 : layout.fields.back().end();

		if (!layout.fields.empty())
			context.report().record_note(
			    "lua_Debug", std::format("the record runs past 0x{:X} into the buffer luau keeps the short source "
			                             "in, which lua_getinfo never has to write",
			                             layout.size));

		return layout;
	}
}
