#include "recover/tvalue_recoverer.hpp"

#include <algorithm>
#include <format>
#include <map>

namespace rml::dumper::recover
{
	std::expected<schema::StructLayout, Error> TValueRecoverer::recover(const RecoveryContext& context) const
	{
		const auto trace = context.trace(target::Anchor::lua_pushnumber);
		if (!trace)
			return std::unexpected(trace.error());

		std::map<disasm::Object, std::vector<const disasm::MemoryAccess*>> by_object;

		for (const auto& access : (*trace)->accesses)
		{
			if (!access.is_write || access.object == disasm::no_object ||
			    access.index != disasm::Register::none || access.displacement < 0 || access.displacement >= 0x20)
				continue;

			by_object[access.object].push_back(&access);
		}

		schema::StructLayout layout{.name = "TValue"};

		const disasm::MemoryAccess* value = nullptr;
		const disasm::MemoryAccess* tag = nullptr;

		for (const auto& [object, writes] : by_object)
		{
			const auto stored = std::ranges::find_if(writes, [](const disasm::MemoryAccess* access) {
				return access->width == 8 && access->value_register == disasm::Register::none &&
				       !access->immediate;
			});

			const auto tagged = std::ranges::find_if(writes, [](const disasm::MemoryAccess* access) {
				return access->width == 4 && access->immediate;
			});

			if (stored == writes.end() || tagged == writes.end())
				continue;

			value = *stored;
			tag = *tagged;
			break;
		}

		if (value == nullptr || tag == nullptr)
		{
			context.report().record_failure(
			    "TValue", "value",
			    "lua_pushnumber writes no floating point value together with a constant tag through one base");
			return layout;
		}

		const auto value_probe = "the floating point store lua_pushnumber makes through the stack top";
		context.report().record_recovered("TValue", "value", value_probe);
		layout.add({.name = "value",
		            .type = "Value",
		            .size = 8,
		            .offset = static_cast<std::size_t>(value->displacement),
		            .provenance = schema::Provenance::recovered("lua_pushnumber", value_probe)});

		const auto tag_probe = std::format("the constant {} stored beside it", *tag->immediate);
		context.report().record_recovered("TValue", "tt", tag_probe);
		layout.add({.name = "tt",
		            .type = "int",
		            .size = 4,
		            .offset = static_cast<std::size_t>(tag->displacement),
		            .provenance = schema::Provenance::recovered("lua_pushnumber", tag_probe)});

		layout.size = layout.fields.back().end();

		return layout;
	}
}
