#include "rml/dumper/disasm/trace_query.hpp"

#include <array>

namespace rml::dumper::disasm
{
	const MemoryAccess* TraceQuery::nth(const bool is_write, const Register base, const std::size_t n,
	                                    const std::uint8_t width, const std::optional<std::size_t> after,
	                                    const Register value) const
	{
		std::size_t seen = 0;

		for (const auto& access : m_trace.accesses)
		{
			if (access.is_write != is_write || access.base != base)
				continue;
			if (width != 0 && access.width != width)
				continue;
			if (value != Register::none && access.value_register != value)
				continue;
			if (after && access.sequence <= *after)
				continue;

			if (seen++ == n)
				return &access;
		}

		return nullptr;
	}

	const MemoryAccess* TraceQuery::nth_write(const Register base, const std::size_t n, const std::uint8_t width,
	                                          const std::optional<std::size_t> after, const Register value) const
	{
		return nth(true, base, n, width, after, value);
	}

	const MemoryAccess* TraceQuery::nth_read(const Register base, const std::size_t n, const std::uint8_t width,
	                                         const std::optional<std::size_t> after, const Register value) const
	{
		return nth(false, base, n, width, after, value);
	}

	const MemoryAccess* TraceQuery::first_immediate_write(const Register base, const std::uint64_t value,
	                                                      const std::optional<std::size_t> after) const
	{
		for (const auto& access : m_trace.accesses)
		{
			if (!access.is_write || access.base != base || access.immediate != value)
				continue;
			if (after && access.sequence <= *after)
				continue;

			return &access;
		}

		return nullptr;
	}

	const MemoryAccess* TraceQuery::first_indexed(const Register base, const bool is_write,
	                                              const std::optional<std::size_t> after) const
	{
		for (const auto& access : m_trace.accesses)
		{
			if (access.is_write != is_write || access.base != base || access.index == Register::none)
				continue;
			if (after && access.sequence <= *after)
				continue;

			return &access;
		}

		return nullptr;
	}

	Register TraceQuery::dominant_base() const
	{
		std::array<std::size_t, 64> counts{};

		for (const auto& access : m_trace.accesses)
			if (access.is_write && access.base != Register::none)
				++counts[static_cast<std::size_t>(access.base)];

		std::size_t best = 0;
		Register winner = Register::none;

		for (std::size_t i = 0; i < counts.size(); ++i)
			if (counts[i] > best)
			{
				best = counts[i];
				winner = static_cast<Register>(i);
			}

		return winner;
	}

	std::optional<std::size_t> TraceQuery::call_sequence(const Rva target, const std::size_t skip) const
	{
		std::size_t seen = 0;

		for (const auto& call : m_trace.calls)
		{
			if (call.target != target)
				continue;

			if (seen++ == skip)
				return call.sequence;
		}

		return std::nullopt;
	}

	std::optional<std::size_t> TraceQuery::first_call_sequence() const
	{
		if (m_trace.calls.empty())
			return std::nullopt;

		return m_trace.calls.front().sequence;
	}
}
