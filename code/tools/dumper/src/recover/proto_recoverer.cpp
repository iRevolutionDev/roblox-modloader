#include "recover/proto_recoverer.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <map>

namespace rml::dumper::recover
{
	std::vector<ProtoRecoverer::FreedArray> ProtoRecoverer::freed_arrays(const disasm::Trace& trace,
	                                                                     const Rva free_function,
	                                                                     const disasm::Object proto)
	{
		std::vector<FreedArray> arrays;
		std::size_t previous = 0;

		for (const auto& call : trace.calls)
		{
			if (call.target != free_function)
				continue;

			FreedArray array{.pointer = -1, .size = -1, .element = 1};

			for (const auto& access : trace.accesses)
			{
				if (access.is_write || access.object != proto || access.sequence >= call.sequence ||
				    access.sequence < previous || access.displacement < 0)
					continue;

				if (access.width == 8)
					array.pointer = access.displacement;
				else if (access.width == 4)
					array.size = access.displacement;
			}

			for (const auto& constant : trace.constants)
				if (constant.kind == disasm::ConstantKind::scale && constant.sequence < call.sequence &&
				    constant.sequence >= previous)
					array.element *= constant.value;

			previous = call.sequence;

			if (array.pointer >= 0 && array.size >= 0)
				arrays.push_back(array);
		}

		return arrays;
	}

	std::expected<schema::StructLayout, Error> ProtoRecoverer::recover(const RecoveryContext& context) const
	{
		const auto trace = context.trace(target::Anchor::luaF_freeproto);
		if (!trace)
			return std::unexpected(trace.error());

		const auto free_function = context.anchors().at(target::Anchor::luaM_free);

		disasm::Object proto = disasm::no_object;
		std::size_t most = 0;
		std::map<disasm::Object, std::size_t> reads;

		for (const auto& access : (*trace)->accesses)
			if (!access.is_write && access.object != disasm::no_object && access.displacement >= 0)
				if (const auto count = ++reads[access.object]; count > most)
				{
					most = count;
					proto = access.object;
				}

		const auto arrays = freed_arrays(**trace, free_function, proto);

		struct FreedField
		{
			std::string_view pointer;
			std::string_view type;
			std::string_view size;
		};

		static constexpr std::array<FreedField, 8> freed_in_order{{
		    {"code", "Instruction*", "sizecode"},
		    {"p", "Proto**", "sizep"},
		    {"k", "TValue*", "sizek"},
		    {"lineinfo", "uint8_t*", "sizelineinfo"},
		    {"locvars", "LocVar*", "sizelocvars"},
		    {"upvalues", "TString**", "sizeupvalues"},
		    {"debuginsn", "uint8_t*", "sizecode"},
		    {"typeinfo", "uint8_t*", "sizetypeinfo"},
		}};

		schema::StructLayout layout{.name = "Proto"};

		if (arrays.size() < freed_in_order.size())
		{
			context.report().record_failure(
			    "Proto", "code",
			    std::format("luaF_freeproto releases {} arrays through luaM_free, expected at least {}",
			                arrays.size(), freed_in_order.size()));
			return layout;
		}

		if (arrays.size() > freed_in_order.size())
			context.report().record_note(
			    "Proto", std::format("luaF_freeproto releases {} arrays, the {} beyond the ones luau declares stay "
			                         "unnamed",
			                         arrays.size(), arrays.size() - freed_in_order.size()));

		for (std::size_t i = 0; i < freed_in_order.size(); ++i)
		{
			const auto& freed = freed_in_order[i];
			const auto probe = std::format("array {} released by luaF_freeproto", i);

			context.report().record_recovered("Proto", std::string(freed.pointer), probe);
			layout.add({.name = std::string(freed.pointer),
			            .type = std::string(freed.type),
			            .size = 8,
			            .offset = static_cast<std::size_t>(arrays[i].pointer),
			            .provenance = schema::Provenance::recovered("luaF_freeproto", probe)});

			auto size_name = std::string(freed.size);
			auto size_probe = probe;

			if (const auto* already = layout.find(size_name); already != nullptr)
			{
				if (already->offset == static_cast<std::size_t>(arrays[i].size))
					continue;

				size_name = std::format("size{}", freed.pointer);
				size_probe = std::format(
				    "array {} counts through its own field, not the {} at 0x{:X} that was expected", i, freed.size,
				    already->offset);
			}

			context.report().record_recovered("Proto", size_name, size_probe);
			layout.add({.name = size_name,
			            .type = "int",
			            .size = 4,
			            .offset = static_cast<std::size_t>(arrays[i].size),
			            .provenance = schema::Provenance::recovered("luaF_freeproto", std::move(size_probe))});
		}

		static constexpr std::array<std::pair<std::string_view, std::int64_t>, 3> element_sizes{
		    {{"code", 4}, {"p", 8}, {"k", 16}}};

		for (const auto& [name, expected] : element_sizes)
		{
			const auto* field = layout.find(name);
			if (field == nullptr)
				continue;

			const auto freed = std::ranges::find(arrays, static_cast<std::int64_t>(field->offset),
			                                     &FreedArray::pointer);
			if (freed == arrays.end() || freed->element == expected)
				continue;

			context.report().record_failure(
			    "Proto", std::string(name),
			    std::format("luaF_freeproto walks it in steps of {} bytes, not the {} an element takes",
			                freed->element, expected));
		}

		layout.size = layout.fields.empty() ? 0 : layout.fields.back().end();

		return layout;
	}
}
