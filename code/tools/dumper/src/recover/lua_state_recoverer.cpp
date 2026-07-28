#include "recover/lua_state_recoverer.hpp"

#include <format>

namespace rml::dumper::recover
{
	void LuaStateRecoverer::Probe::take(std::string name, std::string type, const std::size_t size,
	                                    const disasm::MemoryAccess* access, const target::Anchor anchor,
	                                    std::string description)
	{
		if (access == nullptr || access->displacement < 0)
		{
			m_context.report().record_failure("lua_State", name, std::move(description));
			return;
		}

		m_context.report().record_recovered("lua_State", name, description);
		m_layout.add({.name = std::move(name),
		              .type = std::move(type),
		              .size = size,
		              .offset = static_cast<std::size_t>(access->displacement),
		              .provenance = schema::Provenance::recovered(std::string(to_string(anchor)),
		                                                          std::move(description))});
	}

	void LuaStateRecoverer::Probe::confirm(const std::string_view name, const disasm::MemoryAccess* access,
	                                       std::string description) const
	{
		const auto* field = m_layout.find(name);
		if (field == nullptr || access == nullptr)
			return;

		if (static_cast<std::size_t>(access->displacement) != field->offset)
			m_context.report().record_failure(
			    "lua_State", std::string(name),
			    std::format("{} disagrees: 0x{:X} against 0x{:X}", description, access->displacement,
			                field->offset));
	}

	void LuaStateRecoverer::Probe::take_offset(std::string name, std::string type, const std::size_t size,
	                                           const std::optional<std::int64_t> offset,
	                                           const target::Anchor anchor, std::string description)
	{
		if (!offset || *offset < 0)
		{
			m_context.report().record_failure("lua_State", name, std::move(description));
			return;
		}

		m_context.report().record_recovered("lua_State", name, description);
		m_layout.add({.name = std::move(name),
		              .type = std::move(type),
		              .size = size,
		              .offset = static_cast<std::size_t>(*offset),
		              .provenance = schema::Provenance::recovered(std::string(to_string(anchor)),
		                                                          std::move(description))});
	}

	std::optional<std::int64_t> LuaStateRecoverer::copied_from_parent(const disasm::Trace& trace,
	                                                                  const disasm::Object parent,
	                                                                  const std::uint8_t width,
	                                                                  const schema::StructLayout& claimed)
	{
		for (const auto& read : trace.accesses)
		{
			if (read.is_write || read.object != parent || read.width != width || read.displacement < 0)
				continue;
			if (claimed.covers(static_cast<std::size_t>(read.displacement)))
				continue;

			for (const auto& write : trace.accesses)
			{
				if (!write.is_write || write.object == parent || write.width != width)
					continue;
				if (write.displacement != read.displacement || write.sequence <= read.sequence)
					continue;

				return read.displacement;
			}
		}

		return std::nullopt;
	}

	std::expected<schema::StructLayout, Error> LuaStateRecoverer::recover(const RecoveryContext& context) const
	{
		const auto stack_trace = context.trace(target::Anchor::luaD_reallocstack);
		const auto call_trace = context.trace(target::Anchor::luaD_reallocCI);
		const auto free_trace = context.trace(target::Anchor::luaM_free);
		const auto resume_trace = context.trace_callee(target::Anchor::lua_resume, 0);
		const auto settop_trace = context.trace(target::Anchor::lua_settop);

		for (const auto* traced : {&stack_trace, &call_trace, &free_trace, &resume_trace, &settop_trace})
			if (!traced->has_value())
				return std::unexpected(traced->error());

		schema::StructLayout layout{.name = "lua_State"};
		Probe probe(context, layout);

		const disasm::TraceQuery stack(**stack_trace);
		const auto state = stack.dominant_object();
		const auto after_alloc = stack.first_call_sequence();

		probe.take("stack", "StkId", 8, stack.nth_write(state, 0, 8, after_alloc),
		           target::Anchor::luaD_reallocstack, "first qword write after the reallocation");
		probe.take("stack_last", "StkId", 8, stack.nth_write(state, 1, 8, after_alloc),
		           target::Anchor::luaD_reallocstack, "second qword write after the reallocation");
		probe.take("top", "StkId", 8, stack.nth_write(state, 2, 8, after_alloc), target::Anchor::luaD_reallocstack,
		           "third qword write after the reallocation");
		probe.take("base", "StkId", 8, stack.nth_write(state, 3, 8, after_alloc), target::Anchor::luaD_reallocstack,
		           "fourth qword write after the reallocation");
		probe.take("stacksize", "int", 4, stack.nth_write(state, 0, 4, after_alloc),
		           target::Anchor::luaD_reallocstack, "first dword write after the reallocation");

		const disasm::TraceQuery calls(**call_trace);
		const auto call_state = calls.dominant_object();

		probe.take("base_ci", "CallInfo*", 8, calls.nth_write(call_state, 0, 8), target::Anchor::luaD_reallocCI,
		           "first qword write");
		probe.take("ci", "CallInfo*", 8, calls.nth_write(call_state, 1, 8), target::Anchor::luaD_reallocCI,
		           "second qword write");
		probe.take("end_ci", "CallInfo*", 8, calls.nth_write(call_state, 2, 8), target::Anchor::luaD_reallocCI,
		           "third qword write");
		probe.take("size_ci", "int", 4, calls.nth_write(call_state, 0, 4), target::Anchor::luaD_reallocCI,
		           "first dword write");

		const auto argument = disasm::entry_object(context.abi().argument(0));

		const disasm::TraceQuery frees(**free_trace);
		probe.take("global", "global_State*", 8, frees.nth_read(argument, 0, 8), target::Anchor::luaM_free,
		           "first qword read through the state argument");

		const disasm::TraceQuery resume(**resume_trace);
		const auto resume_state = resume.dominant_object();

		probe.take("status", "uint8_t", 1, resume.nth_read(argument, 0, 1), target::Anchor::lua_resume,
		           "first byte read through the state argument");
		probe.take("isactive", "bool", 1, resume.first_immediate_write(resume_state, 1),
		           target::Anchor::lua_resume, "byte set to one");
		probe.take("nCcalls", "unsigned short", 2, resume.nth_distinct_write(resume_state, 0, 2),
		           target::Anchor::lua_resume, "first word the resume counter touches");
		probe.take("baseCcalls", "unsigned short", 2, resume.nth_distinct_write(resume_state, 1, 2),
		           target::Anchor::lua_resume, "the word it copies the counter into");

		const auto thread_trace = context.trace(target::Anchor::luaE_newthread);
		const auto upval_trace = context.trace(target::Anchor::luaF_findupval);
		if (!thread_trace)
			return std::unexpected(thread_trace.error());
		if (!upval_trace)
			return std::unexpected(upval_trace.error());

		const auto parent = argument;

		probe.take_offset("activememcat", "uint8_t", 1, copied_from_parent(**thread_trace, parent, 1, layout),
		                  target::Anchor::luaE_newthread, "first byte copied from the parent state");
		probe.take_offset("gt", "LuaTable*", 8, copied_from_parent(**thread_trace, parent, 8, layout),
		                  target::Anchor::luaE_newthread,
		                  "the qword copied from the parent state that is not the global one");

		const auto* known_global = layout.find("global");

		for (const auto& access : (*upval_trace)->accesses)
		{
			if (access.object != parent || access.width != 8 || access.is_write)
				continue;
			if (known_global != nullptr && static_cast<std::size_t>(access.displacement) == known_global->offset)
				continue;

			probe.take("openupval", "UpVal*", 8, &access, target::Anchor::luaF_findupval,
			           "the other qword reached through the state argument, the list head taken by address");
			break;
		}

		if (layout.find("openupval") == nullptr)
			context.report().record_failure("lua_State", "openupval",
			                                "luaF_findupval reaches no qword through the state besides global");

		probe.take_offset("singlestep", "bool", 1, copied_from_parent(**thread_trace, parent, 1, layout),
		                  target::Anchor::luaE_newthread,
		                  "the byte copied from the parent state that is not the memory category");

		const disasm::TraceQuery settop(**settop_trace);

		probe.confirm("top", settop.nth_read(argument, 0, 8), "lua_settop reads top at");
		probe.confirm("base", settop.nth_read(argument, 1, 8), "lua_settop reads base at");

		const auto allocation = context.abi().argument(1);
		const auto smallest = layout.fields.empty() ? 0 : layout.fields.back().end();

		std::optional<std::int64_t> size;
		for (const auto& constant : (*thread_trace)->constants)
		{
			if (constant.kind != disasm::ConstantKind::literal || constant.destination != allocation)
				continue;
			if (constant.value < static_cast<std::int64_t>(smallest) || constant.value % 8 != 0)
				continue;
			if (!size || constant.value < *size)
				size = constant.value;
		}

		if (!size)
		{
			layout.size = smallest;
			return layout;
		}

		context.report().record_recovered(
		    "lua_State", "sizeof", std::format("0x{:X}, the size luaE_newthread asks the collector for", *size));
		layout.size = static_cast<std::size_t>(*size);

		if (const auto tail = layout.size - smallest; tail == 4 && smallest % 4 == 0)
		{
			const auto probe = std::format("the only four byte slot left between 0x{:X} and the end", smallest);

			context.report().record_recovered("lua_State", "cachedslot", probe);
			layout.add({.name = "cachedslot",
			            .type = "int",
			            .size = 4,
			            .offset = smallest,
			            .provenance = schema::Provenance::recovered("luaE_newthread", probe)});
		}

		return layout;
	}
}
