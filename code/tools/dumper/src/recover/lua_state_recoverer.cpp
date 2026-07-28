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
		const auto state = stack.dominant_base();
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
		const auto call_state = calls.dominant_base();

		probe.take("base_ci", "CallInfo*", 8, calls.nth_write(call_state, 0, 8), target::Anchor::luaD_reallocCI,
		           "first qword write");
		probe.take("ci", "CallInfo*", 8, calls.nth_write(call_state, 1, 8), target::Anchor::luaD_reallocCI,
		           "second qword write");
		probe.take("end_ci", "CallInfo*", 8, calls.nth_write(call_state, 2, 8), target::Anchor::luaD_reallocCI,
		           "third qword write");
		probe.take("size_ci", "int", 4, calls.nth_write(call_state, 0, 4), target::Anchor::luaD_reallocCI,
		           "first dword write");

		const disasm::TraceQuery frees(**free_trace);
		probe.take("global", "global_State*", 8, frees.nth_read(context.abi().argument(0), 0, 8),
		           target::Anchor::luaM_free, "first qword read through the state argument");

		const disasm::TraceQuery resume(**resume_trace);
		const auto resume_state = resume.dominant_base();

		probe.take("status", "uint8_t", 1, resume.nth_read(context.abi().argument(0), 0, 1),
		           target::Anchor::lua_resume, "first byte read through the state argument");
		probe.take("isactive", "bool", 1, resume.first_immediate_write(resume_state, 1),
		           target::Anchor::lua_resume, "byte set to one");
		probe.take("nCcalls", "unsigned short", 2, resume.nth_write(resume_state, 0, 2),
		           target::Anchor::lua_resume, "first word write");
		probe.take("baseCcalls", "unsigned short", 2, resume.nth_write(resume_state, 1, 2),
		           target::Anchor::lua_resume, "second word write");

		const disasm::TraceQuery settop(**settop_trace);
		const auto settop_state = context.abi().argument(0);

		probe.confirm("top", settop.nth_read(settop_state, 0, 8), "lua_settop reads top at");
		probe.confirm("base", settop.nth_read(settop_state, 1, 8), "lua_settop reads base at");

		layout.size = layout.fields.empty() ? 0 : layout.fields.back().end();

		return layout;
	}
}
