#include "recover/closure_recoverer.hpp"

#include <algorithm>
#include <format>
#include <map>

namespace rml::dumper::recover
{
	disasm::Register ClosureRecoverer::allocated_object(const disasm::Trace& trace)
	{
		std::map<disasm::Register, std::size_t> writes;

		for (const auto& access : trace.accesses)
			if (access.is_write && access.base != disasm::Register::none &&
			    access.index == disasm::Register::none && access.displacement >= 0 &&
			    access.displacement < 0x40)
				++writes[access.base];

		disasm::Register best = disasm::Register::none;
		std::size_t most = 0;

		for (const auto& [base, count] : writes)
			if (count > most)
			{
				most = count;
				best = base;
			}

		return best;
	}

	const disasm::MemoryAccess* ClosureRecoverer::write_from(const disasm::Trace& trace,
	                                                         const disasm::Register object,
	                                                         const disasm::Register value, const std::uint8_t width)
	{
		const auto found = std::ranges::find_if(trace.accesses, [&](const disasm::MemoryAccess& access) {
			return access.is_write && access.base == object && access.value_register == value &&
			       access.width == width && access.displacement >= 0;
		});

		return found != trace.accesses.end() ? &*found : nullptr;
	}

	static std::map<std::int64_t, std::uint8_t> constant_bytes(const disasm::Trace& trace,
	                                                           const disasm::Register object)
	{
		std::map<std::int64_t, std::uint8_t> bytes;

		for (const auto& access : trace.accesses)
		{
			if (!access.is_write || access.base != object || !access.immediate || access.displacement < 0)
				continue;
			if (access.width == 0 || access.width > 8)
				continue;

			for (std::uint8_t i = 0; i < access.width; ++i)
				bytes[access.displacement + i] = static_cast<std::uint8_t>(*access.immediate >> i * 8 & 0xFF);
		}

		return bytes;
	}

	std::optional<std::int64_t> ClosureRecoverer::differing_constant(const disasm::Trace& first,
	                                                                 const disasm::Trace& second,
	                                                                 const disasm::Register first_object,
	                                                                 const disasm::Register second_object)
	{
		const auto left = constant_bytes(first, first_object);
		const auto right = constant_bytes(second, second_object);

		for (const auto& [offset, value] : left)
		{
			const auto twin = right.find(offset);
			if (twin != right.end() && twin->second != value)
				return offset;
		}

		return std::nullopt;
	}

	std::expected<schema::StructLayout, Error> ClosureRecoverer::recover(const RecoveryContext& context) const
	{
		const auto lua_trace = context.trace(target::Anchor::luaF_newLclosure);
		const auto c_trace = context.trace(target::Anchor::luaF_newCclosure);
		if (!lua_trace)
			return std::unexpected(lua_trace.error());
		if (!c_trace)
			return std::unexpected(c_trace.error());

		const auto lua_object = allocated_object(**lua_trace);
		const auto c_object = allocated_object(**c_trace);

		schema::StructLayout layout{.name = "Closure"};

		const auto record = [&](std::string name, std::string type, const std::size_t size,
		                        const std::optional<std::int64_t> offset, std::string description) {
			if (!offset || *offset < 0)
			{
				context.report().record_failure("Closure", name, std::move(description));
				return;
			}

			context.report().record_recovered("Closure", name, description);
			layout.add({.name = std::move(name),
			            .type = std::move(type),
			            .size = size,
			            .offset = static_cast<std::size_t>(*offset),
			            .provenance = schema::Provenance::recovered("luaF_newLclosure", std::move(description))});
		};

		const auto offset_of = [](const disasm::MemoryAccess* access) -> std::optional<std::int64_t> {
			return access != nullptr ? std::optional{access->displacement} : std::nullopt;
		};

		record("isC", "uint8_t", 1, differing_constant(**lua_trace, **c_trace, lua_object, c_object),
		       "byte constant that differs between the lua and the c closure constructor");

		record("nupvalues", "uint8_t", 1,
		       offset_of(write_from(**lua_trace, lua_object, context.abi().argument(1), 1)),
		       "byte taken from the element count argument");

		record("env", "LuaTable*", 8,
		       offset_of(write_from(**lua_trace, lua_object, context.abi().argument(2), 8)),
		       "qword taken from the environment argument");

		layout.size = layout.fields.empty() ? 0 : layout.fields.back().end();

		return layout;
	}
}
