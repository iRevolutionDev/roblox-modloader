#include "recover/common_header_recoverer.hpp"

#include <algorithm>
#include <format>
#include <map>

namespace rml::dumper::recover
{
	std::vector<disasm::MemoryAccess> CommonHeaderRecoverer::header_writes(const disasm::Trace& trace)
	{
		std::map<disasm::Object, std::vector<disasm::MemoryAccess>> header_by_object;

		for (const auto& access : trace.accesses)
		{
			if (!access.is_write || access.object == disasm::no_object ||
			    access.index != disasm::Register::none)
				continue;

			if (access.width != 1 || access.displacement < 0 || access.displacement >= plausible_header_span)
				continue;

			auto& writes = header_by_object[access.object];
			const auto seen = std::ranges::find(writes, access.displacement,
			                                    &disasm::MemoryAccess::displacement) != writes.end();
			if (!seen)
				writes.push_back(access);
		}

		std::vector<disasm::MemoryAccess> best;

		for (auto& [object, writes] : header_by_object)
		{
			if (writes.size() < header_field_count)
				continue;

			if (writes.size() > best.size())
				best = writes;
		}

		std::ranges::sort(best, {}, &disasm::MemoryAccess::displacement);

		return best;
	}

	std::expected<schema::StructLayout, Error> CommonHeaderRecoverer::recover(const RecoveryContext& context) const
	{
		schema::StructLayout layout{.name = "CommonHeader"};

		const auto table = context.trace(target::Anchor::luaH_new);
		const auto closure = context.trace(target::Anchor::luaF_newLclosure);
		if (!table)
			return std::unexpected(table.error());
		if (!closure)
			return std::unexpected(closure.error());

		const auto closure_writes = header_writes(**closure);

		std::vector<disasm::MemoryAccess> table_writes;
		for (const auto& write : header_writes(**table))
			if (std::ranges::find(closure_writes, write.displacement, &disasm::MemoryAccess::displacement) !=
			    closure_writes.end())
				table_writes.push_back(write);

		if (table_writes.size() != header_field_count)
		{
			context.report().record_failure(
			    "CommonHeader", "tt",
			    std::format("luaH_new and luaF_newLclosure share {} small byte writes, expected {}",
			                table_writes.size(), header_field_count));
			layout.size = header_field_count;
			return layout;
		}

		const auto tag = std::ranges::find_if(table_writes, [&closure_writes](const disasm::MemoryAccess& write) {
			const auto twin = std::ranges::find(closure_writes, write.displacement,
			                                    &disasm::MemoryAccess::displacement);
			return twin != closure_writes.end() && write.immediate && twin->immediate &&
			       *write.immediate != *twin->immediate;
		});

		if (tag == table_writes.end())
		{
			context.report().record_failure("CommonHeader", "tt",
			                                "no byte field carries a different constant in luaH_new and "
			                                "luaF_newLclosure");
			layout.size = header_field_count;
			return layout;
		}

		const auto probe = std::format("byte write at 0x{:X} differing between luaH_new and luaF_newLclosure",
		                               tag->displacement);
		layout.add({.name = "tt",
		            .type = "uint8_t",
		            .size = 1,
		            .offset = static_cast<std::size_t>(tag->displacement),
		            .provenance = schema::Provenance::recovered("luaH_new", probe)});
		context.report().record_recovered("CommonHeader", "tt", probe);

		std::vector<disasm::MemoryAccess> others;
		for (const auto& write : table_writes)
			if (write.displacement != tag->displacement)
				others.push_back(write);

		std::ranges::sort(others, {}, &disasm::MemoryAccess::sequence);

		const std::array names{"marked", "memcat"};
		for (std::size_t i = 0; i < others.size() && i < names.size(); ++i)
		{
			const auto reason = std::format("byte write {} of the collectable header, {} the type tag", i,
			                                others[i].sequence < tag->sequence ? "before" : "after");

			layout.add({.name = names[i],
			            .type = "uint8_t",
			            .size = 1,
			            .offset = static_cast<std::size_t>(others[i].displacement),
			            .provenance = schema::Provenance::recovered("luaH_new", reason)});
			context.report().record_recovered("CommonHeader", names[i], reason);
		}

		layout.size = layout.fields.empty() ? header_field_count : layout.fields.back().end();

		return layout;
	}
}
