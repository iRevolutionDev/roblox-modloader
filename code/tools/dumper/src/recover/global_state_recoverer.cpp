#include "recover/global_state_recoverer.hpp"

#include <algorithm>

namespace rml::dumper::recover
{
	disasm::Object GlobalStateRecoverer::loaded_by(const disasm::Trace& trace, const disasm::MemoryAccess& read)
	{
		for (const auto& access : trace.accesses)
			if (access.sequence > read.sequence && access.base == read.value_register)
				return access.object;

		return disasm::no_object;
	}

	std::expected<schema::StructLayout, Error> GlobalStateRecoverer::recover(const RecoveryContext& context) const
	{
		const auto trace = context.trace(target::Anchor::luaE_newthread);
		if (!trace)
			return std::unexpected(trace.error());

		schema::StructLayout layout{.name = "global_State"};

		const auto* state = context.layout("lua_State");
		const auto* header = context.layout("CommonHeader");
		const auto* global = state != nullptr ? state->find("global") : nullptr;
		const auto* marked = header != nullptr ? header->find("marked") : nullptr;

		const auto probe = "the byte the collector paints a freshly built object with";

		if (global == nullptr || marked == nullptr)
		{
			context.report().record_failure("global_State", "currentwhite", probe);
			return layout;
		}

		const auto reached = std::ranges::find_if(
		    (*trace)->accesses, [&](const disasm::MemoryAccess& access) {
			    return !access.is_write && access.object == disasm::entry_object(context.abi().argument(0)) &&
			           access.width == 8 && static_cast<std::size_t>(access.displacement) == global->offset;
		    });

		const auto owner = reached != (*trace)->accesses.end() ? loaded_by(**trace, *reached) : disasm::no_object;

		std::optional<std::int64_t> currentwhite;

		for (const auto& read : (*trace)->accesses)
		{
			if (read.is_write || read.object != owner || read.width != 1 || read.displacement < 0)
				continue;

			for (const auto& write : (*trace)->accesses)
			{
				if (!write.is_write || write.width != 1 || write.object == owner)
					continue;
				if (static_cast<std::size_t>(write.displacement) != marked->offset ||
				    write.value_register != read.value_register || write.sequence <= read.sequence)
					continue;

				currentwhite = read.displacement;
				break;
			}

			if (currentwhite)
				break;
		}

		if (!currentwhite)
		{
			context.report().record_failure("global_State", "currentwhite", probe);
			return layout;
		}

		context.report().record_recovered("global_State", "currentwhite", probe);
		layout.add({.name = "currentwhite",
		            .type = "uint8_t",
		            .size = 1,
		            .offset = static_cast<std::size_t>(*currentwhite),
		            .provenance = schema::Provenance::recovered("luaE_newthread", probe)});

		layout.size = layout.fields.back().end();

		return layout;
	}
}
