#include "recover/call_info_recoverer.hpp"

#include <algorithm>
#include <format>
#include <map>

namespace rml::dumper::recover
{
	std::vector<std::int64_t> CallInfoRecoverer::relocated_slots(const disasm::Trace& trace,
	                                                             const disasm::Register state)
	{
		std::map<disasm::Register, std::vector<std::int64_t>> by_base;

		for (const auto& access : trace.accesses)
		{
			if (!access.is_write || access.width != 8 || access.base == state ||
			    access.base == disasm::Register::none || access.index != disasm::Register::none ||
			    access.displacement < 0 || access.displacement >= 0x40)
				continue;

			auto& slots = by_base[access.base];
			if (std::ranges::find(slots, access.displacement) == slots.end())
				slots.push_back(access.displacement);
		}

		for (const auto& [base, slots] : by_base)
			if (slots.size() == 3)
				return slots;

		return {};
	}

	std::expected<schema::StructLayout, Error> CallInfoRecoverer::recover(const RecoveryContext& context) const
	{
		const auto trace = context.trace(target::Anchor::luaD_reallocstack);
		if (!trace)
			return std::unexpected(trace.error());

		const disasm::TraceQuery query(**trace);
		const auto slots = relocated_slots(**trace, query.dominant_base());

		schema::StructLayout layout{.name = "CallInfo"};

		static constexpr std::array<std::string_view, 3> relocated_in_order{"top", "base", "func"};

		if (slots.size() != relocated_in_order.size())
		{
			context.report().record_failure(
			    "CallInfo", "top",
			    std::format("correctstack relocates {} slots through a call info, expected {}", slots.size(),
			                relocated_in_order.size()));
			return layout;
		}

		for (std::size_t i = 0; i < slots.size(); ++i)
		{
			const auto probe = std::format("slot {} relocated by correctstack inside luaD_reallocstack", i);

			context.report().record_recovered("CallInfo", std::string(relocated_in_order[i]), probe);
			layout.add({.name = std::string(relocated_in_order[i]),
			            .type = "StkId",
			            .size = 8,
			            .offset = static_cast<std::size_t>(slots[i]),
			            .provenance = schema::Provenance::recovered("luaD_reallocstack", probe)});
		}

		const auto stride = context.trace(target::Anchor::luaD_reallocCI);
		if (!stride)
			return std::unexpected(stride.error());

		std::map<disasm::Register, std::int64_t> scaled;
		for (const auto& constant : (*stride)->constants)
			if (constant.kind == disasm::ConstantKind::scale && constant.destination != disasm::Register::none)
			{
				auto& product = scaled[constant.destination];
				product = product == 0 ? constant.value : product * constant.value;
			}

		const auto smallest = static_cast<std::int64_t>(layout.fields.back().end());

		std::optional<std::int64_t> size;
		for (const auto& [destination, product] : scaled)
			if (product >= smallest && product % 8 == 0 && (!size || product < *size))
				size = product;

		if (!size)
		{
			context.report().record_failure(
			    "CallInfo", "sizeof",
			    std::format("luaD_reallocCI scales its allocation by nothing that reaches 0x{:X}", smallest));
			layout.size = static_cast<std::size_t>(smallest);
			return layout;
		}

		context.report().record_recovered(
		    "CallInfo", "sizeof", std::format("0x{:X}, the factor luaD_reallocCI scales its allocation by", *size));
		layout.size = static_cast<std::size_t>(*size);

		return layout;
	}
}
